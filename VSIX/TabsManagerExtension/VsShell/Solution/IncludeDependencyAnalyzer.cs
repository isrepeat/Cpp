using System;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.VCProjectEngine;


namespace TabsManagerExtension.VsShell.Solution {
    public class IncludeDependencyAnalyzer {
        private readonly EnvDTE80.DTE2 _dte;
        private MsBuildSolutionWatcher _msBuildSolutionWatcher;
        private SolutionSourceFileGraph _solutionSourceFileGraph;

        private FileSystemWatcher? _vcSourceFilesWatcher;
        private readonly HashSet<string> _pendingChangedFiles = new(StringComparer.OrdinalIgnoreCase);
        private readonly DispatcherTimer _debounceTimer;

        public IncludeDependencyAnalyzer() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));

            _debounceTimer = new DispatcherTimer {
                Interval = TimeSpan.FromMilliseconds(300)
            };
            _debounceTimer.Tick += (_, _) => this.OnDebounceTimerTick();
        }

        public void Build() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("Build()");
            ThreadHelper.ThrowIfNotOnUIThread();

            var cppSolutionProjects = this.GetAllCppProjects(_dte);
            _msBuildSolutionWatcher = new MsBuildSolutionWatcher(cppSolutionProjects);
            _solutionSourceFileGraph = new SolutionSourceFileGraph(_msBuildSolutionWatcher);

            foreach (var project in cppSolutionProjects) {
                var tabItemProject = new State.Document.TabItemProject(project);

                var stack = new Stack<EnvDTE.ProjectItem>();
                foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                    stack.Push(item);
                }

                while (stack.Count > 0) {
                    var current = stack.Pop();

                    if (current.FileCount > 0 && current.FileCodeModel is VCFileCodeModel) {
                        string filePath = current.FileNames[1];
                        string ext = Path.GetExtension(filePath);

                        bool isCppProjectFile =
                            string.Equals(ext, ".h", StringComparison.OrdinalIgnoreCase) ||
                            string.Equals(ext, ".hpp", StringComparison.OrdinalIgnoreCase) ||
                            string.Equals(ext, ".cpp", StringComparison.OrdinalIgnoreCase);

                        if (isCppProjectFile) {
                            var source = new SourceFile(filePath, tabItemProject);
                            var includeEntries = this.ExtractRawIncludes(filePath);
                            _solutionSourceFileGraph.AddSourceFileWithIncludes(source, includeEntries);
                        }
                        else {
                            Helpers.Diagnostic.Logger.LogDebug($"[skip non cpp file] {filePath}");
                        }
                    }

                    if (current.ProjectItems != null) {
                        foreach (EnvDTE.ProjectItem child in current.ProjectItems) {
                            stack.Push(child);
                        }
                    }
                }

                // ⛔️ BuildReverseMap больше не нужен
            }

            string? rootDir = Path.GetDirectoryName(_dte.Solution.FullName);
            if (rootDir != null && Directory.Exists(rootDir)) {
                _vcSourceFilesWatcher = new FileSystemWatcher(rootDir) {
                    Filter = "*.*",
                    IncludeSubdirectories = true,
                    NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName
                };

                _vcSourceFilesWatcher.Changed += (_, e) => this.OnRawFileChanged(e.FullPath);
                _vcSourceFilesWatcher.Created += (_, e) => this.OnRawFileChanged(e.FullPath);
                _vcSourceFilesWatcher.Renamed += (_, e) => this.OnRawFileChanged(e.FullPath);
                _vcSourceFilesWatcher.EnableRaisingEvents = true;
            }
        }



        public IReadOnlyCollection<State.Document.TabItemProject> GetTransitiveProjectsIncludersByIncludeString(string includeString) {
            var transitiveIncluders = this.GetTransitiveFilesIncludersByIncludeString(includeString);
            return transitiveIncluders
                .Select(f => f.Project)
                .Distinct()
                .ToList();
        }


        public IReadOnlyCollection<State.Document.TabItemProject> GetTransitiveProjectsIncludersByIncludePath(string includePath) {
            var transitive = this.GetTransitiveFilesIncludersByIncludePath(includePath);
            return transitive
                .Select(f => f.Project)
                .Distinct()
                .ToList();
        }


        /// <summary>
        /// Находит все <see cref="SourceFile"/>'ы, которые транзитивно включают include с заданным именем.
        /// </summary>
        /// <remarks>
        /// Используется двухфазный алгоритм: сначала ищутся прямые попадания по <c>RawInclude</c>,
        /// затем выполняется транзитивный обход вверх по цепочке включений.
        ///
        /// Если включён строгий режим, то кроме совпадения по имени, также сравнивается имя файла в <c>ResolvedPath</c>,
        /// чтобы исключить ложные срабатывания при совпадении <c>RawInclude</c>, но разном физическом файле.
        /// </remarks>
        /// <param name="includeString">Имя include-файла (например, <c>"Logger.h"</c>).</param>
        /// <param name="strictResolvedMatch">
        /// Включает строгую фильтрацию: true — требует совпадения не только по <c>RawInclude</c>, но и по <c>ResolvedPath</c>.
        /// </param>
        /// <returns>Список всех файлов, которые напрямую или транзитивно включают данный include.</returns>
        public IReadOnlyList<SourceFile> GetTransitiveFilesIncludersByIncludeString(string includeString, bool strictResolvedMatch = false) {
            var result = new HashSet<SourceFile>();
            var queue = new Queue<SourceFile>();

            // ① Перебираем все ResolvedIncludeEntry во всех исходных файлах
            foreach (var kvp in _solutionSourceFileGraph.GetAllResolvedIncludeEntries()) {
                var sourceFile = kvp.Key;
                var resolvedList = kvp.Value;

                foreach (var resolved in resolvedList) {
                    var raw = resolved.IncludeEntry.RawInclude;

                    // ② Быстрая фильтрация по имени: проверяем RawInclude (например, "Logger.h")
                    if (!Path.GetFileName(raw).Equals(includeString, StringComparison.OrdinalIgnoreCase)) {
                        continue;
                    }

                    // ③ Строгий режим: фильтруем также по ResolvedPath → EndsWith("Logger.h")
                    if (strictResolvedMatch && resolved.ResolvedPath is not null) {
                        if (!Path.GetFileName(resolved.ResolvedPath).Equals(includeString, StringComparison.OrdinalIgnoreCase)) {
                            continue;
                        }
                    }

                    // ④ Получаем все SourceFile'ы, в которые реально резолвится include
                    if (resolved.ResolvedPath is not null) {
                        foreach (var includedFile in _solutionSourceFileGraph.GetSourceFilesByResolvedPath(resolved.ResolvedPath)) {
                            if (result.Add(includedFile)) {
                                queue.Enqueue(includedFile); // положим в очередь для обратного обхода
                            }
                        }
                    }
                }
            }

            // ⑤ Обратный обход: кто включает те файлы, что мы уже нашли (транзитивно вверх)
            while (queue.Count > 0) {
                var current = queue.Dequeue();

                foreach (var includer in _solutionSourceFileGraph.GetSourceFilesByResolvedPath(current.FilePath)) {
                    if (result.Add(includer)) {
                        queue.Enqueue(includer); // продолжаем подниматься вверх по графу
                    }
                }
            }

            return result.ToList();
        }





        /// <summary>
        /// Находит все <see cref="SourceFile"/>'ы, которые транзитивно включают файл с заданным <c>ResolvedPath</c>.
        /// </summary>
        /// <remarks>
        /// Метод начинает с прямых включений указанного файла (по точному <c>ResolvedPath</c>),
        /// после чего выполняет транзитивный обход вверх — находит все исходные файлы,
        /// которые включают его опосредованно.
        ///
        /// Это наиболее точный и надёжный способ анализа, исключающий неоднозначности,
        /// возникающие при использовании только имени include-файла.
        /// </remarks>
        /// <param name="includePath">
        /// Абсолютный путь до включаемого файла, например:
        /// <c>"d:\PROJECT\Helpers.Shared\Logger.h"</c>.
        /// </param>
        /// <returns>
        /// Список <see cref="SourceFile"/>-файлов, которые напрямую или транзитивно включают указанный путь.
        /// </returns>
        public IEnumerable<SourceFile> GetTransitiveFilesIncludersByIncludePath(string includePath) {
            var directFiles = _solutionSourceFileGraph.GetSourceFilesByResolvedPath(includePath);
            if (directFiles.Count() == 0) {
                return Enumerable.Empty<SourceFile>();
            }

            var result = new HashSet<SourceFile>(directFiles);
            var queue = new Queue<SourceFile>(directFiles);

            while (queue.Count > 0) {
                var current = queue.Dequeue();

                foreach (var kvp in this._solutionSourceFileGraph.GetAllResolvedIncludeEntries()) {
                    var source = kvp.Key;
                    var includes = kvp.Value;

                    foreach (var entry in includes) {
                        if (entry.ResolvedPath != null &&
                            StringComparer.OrdinalIgnoreCase.Equals(entry.ResolvedPath, current.FilePath)) {

                            if (result.Add(source)) {
                                queue.Enqueue(source);
                            }
                        }
                    }
                }
            }

            return result;
        }



        public void LogIncludeTree() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var sourceFile in _solutionSourceFileGraph.AllSourceFiles.OrderBy(f => f.FilePath)) {
                Helpers.Diagnostic.Logger.LogDebug($"[File] {sourceFile.FilePath}");

                var resolvedIncludes = _solutionSourceFileGraph.GetResolvedIncludes(sourceFile);
                if (resolvedIncludes.Any()) {
                    foreach (var resolvedInclude in resolvedIncludes) {
                        string normalized = resolvedInclude.IncludeEntry.RawInclude.Replace('\\', '/');
                        Helpers.Diagnostic.Logger.LogDebug($"  └─ #include \"{normalized}\"");
                    }
                }
                else {
                    Helpers.Diagnostic.Logger.LogDebug("  └─ (no includes)");
                }
            }
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        private void OnRawFileChanged(string path) {
            lock (_pendingChangedFiles) {
                _pendingChangedFiles.Add(path);
            }

            _debounceTimer.Stop();
            _debounceTimer.Start();
        }

        private void OnDebounceTimerTick() {
            _debounceTimer.Stop();

            List<string> changed;
            lock (_pendingChangedFiles) {
                changed = _pendingChangedFiles.ToList();
                _pendingChangedFiles.Clear();
            }

            foreach (var filePath in changed) {
                this.OnFileChanged(filePath);
            }
        }

        private void OnFileChanged(string changedFilePath) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var ext = Path.GetExtension(changedFilePath);
            switch (ext) {
                case ".h":
                case ".hpp":
                case ".cpp":
                    break;

                default:
                    return;
            }

            if (!File.Exists(changedFilePath)) {
                return;
            }

            if (!_solutionSourceFileGraph.TryGetSourceFileRepresentations(changedFilePath, out var candidates)) {
                return;
            }

            if (candidates.Count > 1) {
                Helpers.Diagnostic.Logger.LogDebug($"[ambiguous path] {changedFilePath} → found in {candidates.Count} projects");
            }

            foreach (var oldFile in candidates) {
                var newIncludeEntries = this.ExtractRawIncludes(changedFilePath);
                var oldIncludeEntries = _solutionSourceFileGraph.GetResolvedIncludes(oldFile)
                    .Select(resolvedInclude => resolvedInclude.IncludeEntry)
                    .ToList();

                bool changed = newIncludeEntries.Count != oldIncludeEntries.Count ||
                               !newIncludeEntries.SequenceEqual(oldIncludeEntries);

                if (!changed) {
                    continue;
                }

                var updatedFile = new SourceFile(changedFilePath, oldFile.Project);
                _solutionSourceFileGraph.UpdateSourceFileWithIncludes(updatedFile, newIncludeEntries);

                Helpers.Diagnostic.Logger.LogDebug($"[include changed] {changedFilePath} [{oldFile.Project.ShellProject.Project.UniqueName}] → includes updated");
            }
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        /// <summary>
        /// Возвращает все C++ проекты решения, включая C++/CLI, C++/CX, C++/WinRT.
        /// <br/> Все C++ проекты реализуют интерфейс VCProject.
        /// <br/> Проверка `project.Object is VCProject` — надёжный способ отфильтровать C++.
        /// <br/> Некоторые не-C++ проекты могут выбрасывать исключение при доступе к .Object — это игнорируется.
        /// </summary>
        private List<EnvDTE.Project> GetAllCppProjects(EnvDTE80.DTE2 dte) {
            ThreadHelper.ThrowIfNotOnUIThread();

            const string VsProjectKindMisc = "{66A2671D-8FB5-11D2-AA7E-00C04F688DDE}";
            var result = new List<EnvDTE.Project>();
            var queue = new Queue<EnvDTE.Project>();

            foreach (EnvDTE.Project p in dte.Solution.Projects) {
                queue.Enqueue(p);
            }

            while (queue.Count > 0) {
                var project = queue.Dequeue();

                if (string.Equals(project.Kind, VsProjectKindMisc, StringComparison.OrdinalIgnoreCase)) {
                    continue;
                }

                if (string.Equals(project.Kind, EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder, StringComparison.OrdinalIgnoreCase)) {
                    foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                        if (item.SubProject != null) {
                            queue.Enqueue(item.SubProject);
                        }
                    }
                }
                else {
                    try {
                        // все C++ проекты реализуют VCProject
                        if (project.Object is Microsoft.VisualStudio.VCProjectEngine.VCProject) {
                            result.Add(project);
                        }
                    }
                    catch {
                        // безопасно игнорируем исключения от .NET/SDK-проектов
                    }
                }
            }

            return result;
        }


        private List<IncludeEntry> ExtractRawIncludes(string filePath, int maxLinesToRead = 10) {
            var resultIncludeEntries = new List<IncludeEntry>();

            using var reader = new StreamReader(filePath);
            int lineCount = 0;

            while (!reader.EndOfStream && lineCount < maxLinesToRead) {
                string? line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                lineCount++;
                string trimmed = line.TrimStart();
                if (!trimmed.StartsWith("#include")) {
                    continue;
                }

                int start = trimmed.IndexOfAny(new[] { '"', '<' });
                int end = trimmed.LastIndexOfAny(new[] { '"', '>' });

                if (start >= 0 && end > start) {
                    string rawInclude = trimmed.Substring(start + 1, end - start - 1).Trim();
                    if (!string.IsNullOrWhiteSpace(rawInclude)) {
                        resultIncludeEntries.Add(new IncludeEntry(rawInclude));
                    }
                }
            }

            return resultIncludeEntries;
        }
    }
}