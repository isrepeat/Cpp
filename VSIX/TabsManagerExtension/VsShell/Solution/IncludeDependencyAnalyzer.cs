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
using Microsoft.VisualStudio.Package;


namespace TabsManagerExtension.VsShell.Solution {

    public class IncludeFile {
        public string RawInclude { get; }
        public string NormalizedName { get; }
        public IncludeFile(string rawInclude) {
            this.RawInclude = rawInclude;
            this.NormalizedName = Path.GetFileName(rawInclude);
        }

        public override string ToString() => this.NormalizedName;

        public override int GetHashCode() => StringComparer.OrdinalIgnoreCase.GetHashCode(this.NormalizedName);

        public override bool Equals(object? obj) {
            return obj is IncludeFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.NormalizedName, other.NormalizedName);
        }
    }


    public class SourceFile {
        public string FilePath { get; }
        public State.Document.TabItemProject Project { get; }
        public List<IncludeFile> Includes { get; } = new();

        public SourceFile(string filePath, State.Document.TabItemProject project) {
            this.FilePath = filePath;
            this.Project = project;
        }

        public override int GetHashCode() {
            int h1 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath);
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.Project.ShellProject.Project.UniqueName);
            return (h1 * 397) ^ h2; // HashCode.Combine(h1, h2);
        }

        public override bool Equals(object? obj) {
            return obj is SourceFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(
                       this.Project.ShellProject.Project.UniqueName,
                       other.Project.ShellProject.Project.UniqueName
                       );
        }

        public override string ToString() => $"{this.FilePath} [{this.Project.ShellProject.Project.UniqueName}]";
    }





    public class IncludeDependencyAnalyzer {
        private readonly EnvDTE80.DTE2 _dte;
        private readonly HashSet<SourceFile> _sourceFiles = new();
        private MsBuildSolutionWatcher _msBuildSolutionWatcher;
        public IncludeDependencyAnalyzer() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this._dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }

        public void Build() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("Build()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this._sourceFiles.Clear();

            var solutionProjects = GetAllProjects(this._dte);
            _msBuildSolutionWatcher = new MsBuildSolutionWatcher(solutionProjects);

            foreach (var project in solutionProjects) {
                var tabItemProject = new State.Document.TabItemProject(project);

                var stack = new Stack<EnvDTE.ProjectItem>();
                foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                    stack.Push(item);
                }

                while (stack.Count > 0) {
                    var current = stack.Pop();

                    if (current.FileCount > 0 && current.FileCodeModel is VCFileCodeModel) {
                        string filePath = current.FileNames[1];

                        var source = new SourceFile(filePath, tabItemProject);
                        var includes = ExtractRawIncludes(filePath);
                        source.Includes.AddRange(includes);

                        this._sourceFiles.Add(source);
                    }

                    if (current.ProjectItems != null) {
                        foreach (EnvDTE.ProjectItem child in current.ProjectItems) {
                            stack.Push(child);
                        }
                    }
                }
            }
        }

        public void LogIncludeTree() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var source in this._sourceFiles.OrderBy(f => f.FilePath)) {
                Helpers.Diagnostic.Logger.LogDebug($"[File] {source.FilePath}");

                foreach (var include in source.Includes) {
                    string normalized = include.RawInclude.Replace('\\', '/');
                    Helpers.Diagnostic.Logger.LogDebug($"  └─ #include \"{normalized}\"");
                }

                if (source.Includes.Count == 0) {
                    Helpers.Diagnostic.Logger.LogDebug("  └─ (no includes)");
                }
            }
        }

        public IReadOnlyCollection<State.Document.TabItemProject> GetProjectsIncludingTransitive(string includeName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var transitiveFilePaths = this.GetFilesIncludingTransitive(includeName);

            var projects_ = transitiveFilePaths
                .Select(sf => sf.Project)
                .Distinct()
                .ToList();

            return projects_;
        }


        public IReadOnlyList<SourceFile> GetFilesIncludingTransitive(string includeName) {
            var result = new HashSet<SourceFile>();
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var stack = new Stack<SourceFile>();

            //// ① Найти все файлы, у которых в Includes есть нужное имя
            //foreach (var source in this._sourceFiles) {
            //    foreach (var include in source.Includes) {
            //        if (Path.GetFileName(include.RawInclude).Equals(includeName, StringComparison.OrdinalIgnoreCase)) {
            //            // Проверка: действительно ли include разрешается в нужный файл?
            //            // Для этого нужно, чтобы includeName было путём, иначе разрешать нечего
            //            // Но если мы хотим поддерживать просто имя файла — разрешим всё
            //            // Альтернатива — всегда доверять Path.GetFileName без FilesAreSame

            //            if (result.Add(source)) {
            //                stack.Push(source);
            //            }
            //            break;
            //        }
            //    }
            //}




            //// ① Найти все файлы, у которых в Includes есть нужное имя
            //foreach (var source in this._sourceFiles) {
            //    foreach (var include in source.Includes) {
            //        // Быстрая фильтрация по имени: если имя include не совпадает — пропускаем
            //        if (!Path.GetFileName(include.RawInclude)
            //                 .Equals(includeName, StringComparison.OrdinalIgnoreCase)) {
            //            continue;
            //        }

            //        // Второй проход: пытаемся сопоставить include с реальным файлом проекта
            //        foreach (var candidate in this._sourceFiles) {
            //            // Имена должны совпадать, чтобы была хотя бы теоретическая возможность разрешения
            //            if (!Path.GetFileName(candidate.FilePath)
            //                     .Equals(includeName, StringComparison.OrdinalIgnoreCase)) {
            //                continue;
            //            }

            //            // Проверяем: действительно ли includeRaw из `source` разрешается в файл `candidate`
            //            if (this.FilesAreSame(include.RawInclude, source.FilePath, candidate.FilePath, source.Project)) {
            //                if (result.Add(candidate)) {
            //                    stack.Push(candidate);
            //                }

            //                // Дальше проверять нет смысла — include уже разрешился в один из файлов
            //                break;
            //            }
            //        }

            //        // Даже если не удалось разрешить — мы не пробуем другие include'ы в этом source-файле
            //        // (такая логика была и раньше: считаем, что достаточно одного совпадения)
            //        break;
            //    }
            //}



            // Построение индекса FileName → List<SourceFile>
            var filesByName = this._sourceFiles
                .GroupBy(sf => Path.GetFileName(sf.FilePath), StringComparer.OrdinalIgnoreCase)
                .ToDictionary(g => g.Key, g => g.ToList(), StringComparer.OrdinalIgnoreCase);

            // Основной проход
            foreach (var source in this._sourceFiles) {
                foreach (var include in source.Includes) {
                    // Быстрая фильтрация по имени: если имя include не совпадает — пропускаем
                    if (!Path.GetFileName(include.RawInclude).Equals(includeName, StringComparison.OrdinalIgnoreCase)) {
                        continue;
                    }

                    // Быстро получаем кандидатов по имени
                    if (!filesByName.TryGetValue(includeName, out var candidates)) {
                        continue;
                    }

                    foreach (var candidate in candidates) {
                        // Проверяем: действительно ли includeRaw из `source` разрешается в файл `candidate`
                        if (this.FilesAreSame(include.RawInclude, source.FilePath, candidate.FilePath, source.Project)) {
                            if (result.Add(candidate)) {
                                stack.Push(candidate);
                            }
                            break; // include разрешён — идём к следующему файлу
                        }
                    }

                    break; // не проверяем другие include'ы этого source-файла
                }
            }



            // ② Обратный обход: кто включает найденные файлы
            while (stack.Count > 0) {
                var current = stack.Pop();

                if (!visited.Add(current.FilePath)) {
                    continue;
                }

                foreach (var source in this._sourceFiles) {
                    foreach (var include in source.Includes) {
                        //if (source.FilePath == "d:\\WORK\\C++\\Cpp\\DirectX\\Desktop\\DxDesktop\\[02] Dx SwapChain simple render\\SimpleSceneRenderer.h") {
                        //    int xx = 8;
                        //}
                        //if (source.FilePath == "d:\\WORK\\TEST\\Extensions\\SimpleSolution\\DxDesktop\\SimpleSceneRenderer.h") {
                        //    int xx = 8;
                        //}

                        if (this.FilesAreSame(include.RawInclude, source.FilePath, current.FilePath, source.Project)) {
                            if (result.Add(source)) {
                                stack.Push(source);
                                Helpers.Diagnostic.Logger.LogDebug($"[transitive] {source.FilePath} includes → {current.FilePath}");
                                break;
                            }
                        }
                    }
                }
            }

            return result.ToList();
        }



        private bool FilesAreSame(string includeRaw, string includingFilePath, string candidateFilePath, State.Document.TabItemProject ownerProject) {
            try {
                string candidateFull = Path.GetFullPath(candidateFilePath);

                // ① Пробуем как относительный путь от файла
                string baseDir = Path.GetDirectoryName(includingFilePath)!;
                string resolvedFromLocal = Path.GetFullPath(Path.Combine(baseDir, includeRaw.Replace('/', '\\')));
                if (string.Equals(resolvedFromLocal, candidateFull, StringComparison.OrdinalIgnoreCase)) {
                    return true;
                }

                // ② Пробуем директории проекта (ограниченно)
                var projectIncludeDirs = _msBuildSolutionWatcher.GetIncludeDirectoriesFor(ownerProject.FullName);
                foreach (var includeDir in projectIncludeDirs) {
                    string resolved = Path.GetFullPath(Path.Combine(includeDir, includeRaw.Replace('/', '\\')));
                    if (string.Equals(resolved, candidateFull, StringComparison.OrdinalIgnoreCase)) {
                        return true;
                    }
                }
            }
            catch {
                // ignore
            }

            return false;
        }

        private static List<IncludeFile> ExtractRawIncludes(string filePath, int maxLinesToRead = 10) {
            var result = new List<IncludeFile>();
            string fileDir = Path.GetDirectoryName(filePath)!;

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
                    string raw = trimmed.Substring(start + 1, end - start - 1).Trim();
                    if (!string.IsNullOrWhiteSpace(raw)) {
                        result.Add(new IncludeFile(raw));
                    }
                }
            }

            return result;
        }



        private static List<EnvDTE.Project> GetAllProjects(EnvDTE80.DTE2 dte) {
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
                    result.Add(project);
                }
            }

            return result;
        }
    }
}