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


namespace TabsManagerExtension.VsShell.Solution.Services {
    public class IncludeDependencyAnalyzerService :
        TabsManagerExtension.Services.SingletonServiceBase<IncludeDependencyAnalyzerService>,
        TabsManagerExtension.Services.IExtensionService {

        private MsBuildSolutionWatcher _msBuildSolutionWatcher;
        private SolutionSourceFileGraph _solutionSourceFileGraph;
        private Helpers.DirectoryWatcher? _solutionDirWatcher;
        private DispatcherTimer _delayedFileChangeTimer;

        private readonly HashSet<Helpers.DirectoryChangedEventArgs> _pendingChangedFiles = new();

        private string _lastLoadedSolutionName;
        private bool _buildingSolutionGraphInProcess = false;
        private bool _buildingProjectGraphInProcess = false;

        public IncludeDependencyAnalyzerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return new[] {
                typeof(VsShell.Services.VsIDEStateFlagsTrackerService),
                typeof(VsShell.Solution.Services.VsSolutionEventsTrackerService),
                typeof(VsShell.Solution.Services.SolutionHierarchyAnalyzerService),
            };
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded += this.OnSolutionLoaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded += this.OnProjectLoaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded += this.OnProjectUnloaded;

            // Используем таймер для отложенной обработки изменённых файлов:
            // обработка произойдёт только через заданный интервал после последнего события.
            _delayedFileChangeTimer = new DispatcherTimer {
                Interval = TimeSpan.FromMilliseconds(300)
            };
            _delayedFileChangeTimer.Tick += (_, _) => this.OnDelayedFileChangeTimerTick();

            Helpers.Diagnostic.Logger.LogDebug("[IncludeDependencyAnalyzerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _delayedFileChangeTimer.Stop();
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded -= this.OnProjectUnloaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded -= this.OnProjectLoaded;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded -= this.OnSolutionLoaded;

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[IncludeDependencyAnalyzerService] Disposed.");
        }


        //
        // ░ Api
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        public bool IsReady() {
            return _solutionSourceFileGraph != null && _buildingSolutionGraphInProcess == false && _buildingProjectGraphInProcess == false;
        }


        public IReadOnlyCollection<VsShell.Project.LoadedProjectNode> GetTransitiveProjectsIncludersByIncludeString(string includeString) {
            var transitiveIncluders = this.GetTransitiveFilesIncludersByIncludeString(includeString);
            if (transitiveIncluders == null) {
                return null;
            }

            return transitiveIncluders
                .Select(sf => sf.ProjectNode.CurrentProjectNodeStateObj)
                .OfType<VsShell.Project.LoadedProjectNode>()
                .Distinct()
                .ToList();
        }


        public IReadOnlyCollection<VsShell.Project.LoadedProjectNode> GetTransitiveProjectsIncludersByIncludePath(string includePath) {
            var transitiveIncluders = this.GetTransitiveFilesIncludersByIncludePath(includePath);
            if (transitiveIncluders == null) {
                return null;
            }

            return transitiveIncluders
                .Select(sf => sf.ProjectNode.CurrentProjectNodeStateObj)
                .OfType<VsShell.Project.LoadedProjectNode>()
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
        public IReadOnlyList<Document.SourceFile> GetTransitiveFilesIncludersByIncludeString(string includeString, bool strictResolvedMatch = false) {
            if (!this.IsReady()) {
                return null;
            }

            var result = new HashSet<Document.SourceFile>();
            var queue = new Queue<Document.SourceFile>();

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
        public IEnumerable<Document.SourceFile> GetTransitiveFilesIncludersByIncludePath(string includePath) {
            if (!this.IsReady()) {
                return null;
            }

            var directFiles = _solutionSourceFileGraph.GetSourceFilesByResolvedPath(includePath);
            if (directFiles.Count() == 0) {
                return Enumerable.Empty<Document.SourceFile>();
            }

            var result = new HashSet<Document.SourceFile>(directFiles);
            var queue = new Queue<Document.SourceFile>(directFiles);

            while (queue.Count > 0) {
                var current = queue.Dequeue();

                foreach (var kvp in _solutionSourceFileGraph.GetAllResolvedIncludeEntries()) {
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

            if (!this.IsReady()) {
                return;
            }

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
        private void OnSolutionLoaded(string solutionName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionLoaded()");

            if (solutionName == _lastLoadedSolutionName) {
                return;
            }
            _lastLoadedSolutionName = solutionName;

            VsixThreadHelper.RunOnVsThread(() => {
                //await Task.Run(() => {
                    this.BuildSolutionGraph();
                //});
            });
        }


        private void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnProjectLoaded()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_solutionSourceFileGraph == null) {
                return;
            }

            if (e.TryGetRealHierarchy(out var realHierarchy)) {
                PackageServices.VsSolution.GetGuidOfProject(realHierarchy.VsHierarchy, out var projectGuid);

                var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
                var loadedProjectNode = solutionHierarchyAnalyzer.LoadedProjects
                    .FirstOrDefault(p => p.ProjectNode.ProjectGuid == projectGuid);

                if (loadedProjectNode != null) {
                    this.UpdateProjectGraph(loadedProjectNode);
                }
            }
        }


        private void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnProjectUnloaded()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_solutionSourceFileGraph == null) {
                return;
            }

            if (e.TryGetRealHierarchy(out var realHierarchy)) {
                var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(realHierarchy.VsHierarchy);

                var filesToRemove = _solutionSourceFileGraph.AllSourceFiles
                    .Where(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.ProjectNode.UniqueName, dteProject.UniqueName))
                    .ToList();

                foreach (var sf in filesToRemove) {
                    _solutionSourceFileGraph.RemoveSourceFile(sf);
                }
            }
        }


        private void OnSolutionDirectoryChanged(Helpers.DirectoryChangedEventArgs e) {
            lock (_pendingChangedFiles) {
                _pendingChangedFiles.Add(e);
            }

            _delayedFileChangeTimer.Stop();
            _delayedFileChangeTimer.Start();
        }


        private void OnDelayedFileChangeTimerTick() {
            _delayedFileChangeTimer.Stop();

            List<Helpers.DirectoryChangedEventArgs> changedFiles;
            lock (_pendingChangedFiles) {
                changedFiles = _pendingChangedFiles.ToList();
                _pendingChangedFiles.Clear();
            }

            this.ProcessChangedVcxProjects(ref changedFiles);

            foreach (var changedFile in changedFiles) {
                this.ProcessChangedFile(changedFile);
            }
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        /// <summary>
        /// Depends on SolutionHierarchyAnalyzerService.
        /// </summary>
        private void BuildSolutionGraph() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("BuildSolutionGraph()");
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                //Console.Beep(3000, 500);
                _buildingSolutionGraphInProcess = true;

                _msBuildSolutionWatcher?.Dispose();
                _solutionDirWatcher?.Dispose();


                var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
                var loadedProjectNodes = solutionHierarchyAnalyzer.LoadedProjects;
                //.Where(pn => EnvDteUtils.IsCppProject(pn.dteProject))
                //.ToList();

                var dteProjects = loadedProjectNodes
                    .Select(pn => pn.ShellProject.dteProject)
                    .ToList();

                _msBuildSolutionWatcher = new MsBuildSolutionWatcher(dteProjects);
                _solutionSourceFileGraph = new SolutionSourceFileGraph(_msBuildSolutionWatcher);

                foreach (var loadedProjectNode in loadedProjectNodes) {
                    this.UpdateProjectGraph(loadedProjectNode);
                }

                string? solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
                if (solutionDir != null && Directory.Exists(solutionDir)) {
                    _solutionDirWatcher = new Helpers.DirectoryWatcher(solutionDir);
                    _solutionDirWatcher.DirectoryChanged += this.OnSolutionDirectoryChanged;
                }

#if DEBUG
                Console.Beep(1500, 500);
#endif
                _buildingSolutionGraphInProcess = false;
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"[BuildSolutionGraph] exception: {ex}");
                System.Diagnostics.Debugger.Break();
                throw;
            }
        }


        private void UpdateProjectGraph(VsShell.Project.LoadedProjectNode loadedProjectNode) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("UpdateProjectGraph()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"project name = [{loadedProjectNode.ProjectNode.UniqueName}]");

            if (_solutionSourceFileGraph == null) {
                return;
            }
            //Console.Beep(1000, 500);
            _buildingProjectGraphInProcess = true;

            var stack = new Stack<EnvDTE.ProjectItem>();

            foreach (EnvDTE.ProjectItem item in loadedProjectNode.ShellProject.dteProject.ProjectItems) {
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
                        var newSourceFile = new Document.SourceFile(filePath, loadedProjectNode.ProjectNode);
                        var newIncludeEntries = this.ExtractRawIncludes(filePath);

                        if (_solutionSourceFileGraph.TryGetSourceFileRepresentations(filePath, out var candidates) &&
                            candidates.Any(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.ProjectNode.UniqueName, newSourceFile.ProjectNode.UniqueName))) {

                            _solutionSourceFileGraph.UpdateSourceFileWithIncludes(newSourceFile, newIncludeEntries);
                            Helpers.Diagnostic.Logger.LogDebug($"updated sourceFile: {filePath} [{loadedProjectNode.ProjectNode.UniqueName}]");
                        }
                        else {
                            _solutionSourceFileGraph.AddSourceFileWithIncludes(newSourceFile, newIncludeEntries);
                            Helpers.Diagnostic.Logger.LogDebug($"added sourceFile: {filePath} [{loadedProjectNode.ProjectNode.UniqueName}]");
                        }
                    }
                    else {
                        Helpers.Diagnostic.Logger.LogDebug($"non cpp file: {filePath} [{loadedProjectNode.ProjectNode.UniqueName}]");
                    }
                }

                // Всегда обходим вложенные элементы, даже если это была папка или файл без .h/.cpp
                if (current.ProjectItems != null) {
                    foreach (EnvDTE.ProjectItem child in current.ProjectItems) {
                        stack.Push(child);
                    }
                }
            }

            //Console.Beep(700, 500);
            _buildingProjectGraphInProcess = false;
        }


        private List<Document.IncludeEntry> ExtractRawIncludes(string filePath, int maxLinesToRead = 10) {
            var resultIncludeEntries = new List<Document.IncludeEntry>();

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
                        resultIncludeEntries.Add(new Document.IncludeEntry(rawInclude));
                    }
                }
            }

            return resultIncludeEntries;
        }


        private void ProcessChangedVcxProjects(ref List<Helpers.DirectoryChangedEventArgs> changedFiles) {
            var changedVcxProjects = new List<Helpers.DirectoryChangedEventArgs>();

            foreach (var changedFile in changedFiles) {
                var ext = Path.GetExtension(changedFile.FullPath);
                switch (ext) {
                    case ".vcxproj":
                    case ".filters":
                    case ".vcxitems":
                        changedVcxProjects.Add(changedFile);
                        break;
                }
            }

            if (changedVcxProjects.Count == 0) {
                return;
            }

            changedFiles.RemoveAll(changedVcxProjects.Contains);

            // Game.vcxproj
            // Game.vcxproj.filters
            // Helpers.Shared.vcxitems.filters
            //          
            //          |
            //          V
            //
            // Game
            // Helpers.Shared

            var changedUniqueVcxProjectNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var changedVcxProject in changedVcxProjects) {
                string fileName = Path.GetFileName(changedVcxProject.FullPath);

                string projectName = fileName;
                if (projectName.EndsWith(".filters", StringComparison.OrdinalIgnoreCase)) {
                    projectName = Path.GetFileNameWithoutExtension(projectName); // уберём .filters
                }

                projectName = Path.GetFileNameWithoutExtension(projectName); // уберём .vcxproj или .vcxitems
                changedUniqueVcxProjectNames.Add(projectName);
            }

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            foreach (var loadedProjectNode in solutionHierarchyAnalyzer.LoadedProjects) {
                foreach (var changedProjectName in changedUniqueVcxProjectNames) {
                    if (StringComparer.OrdinalIgnoreCase.Equals(changedProjectName, loadedProjectNode.ProjectNode.Name)) {
                        this.UpdateProjectGraph(loadedProjectNode);
                    }
                }
            }
        }


        private void ProcessChangedFile(Helpers.DirectoryChangedEventArgs changedFile) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var ext = Path.GetExtension(changedFile.FullPath);
            switch (ext) {
                case ".h":
                case ".hpp":
                case ".cpp":
                    break;

                default:
                    return;
            }

            switch (changedFile.ChangeType) {
                case Helpers.DirectoryChangeType.Changed: {
                        if (!File.Exists(changedFile.FullPath)) {
                            return;
                        }

                        if (_solutionSourceFileGraph.TryGetSourceFileRepresentations(changedFile.FullPath, out var candidates) && candidates.Count > 0) {
                            this.UpdateIncludesIfNeeded(changedFile.FullPath, candidates);
                        }
                    }
                    break;

                case Helpers.DirectoryChangeType.Created:
                case Helpers.DirectoryChangeType.Renamed: {
                        if (!File.Exists(changedFile.FullPath)) {
                            return;
                        }

                        if (_solutionSourceFileGraph.TryGetSourceFileRepresentations(changedFile.FullPath, out var candidates) && candidates.Count > 0) {
                            this.UpdateIncludesIfNeeded(changedFile.FullPath, candidates);
                            return;
                        }

                        // ⛔ Файл не найден в графе — ищем в проектах, возможно git checkout вернул файл
                        var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
                        foreach (var loadedProjectNode in solutionHierarchyAnalyzer.LoadedProjects) {
                            if (Utils.EnvDteUtils.IsFileInProject(changedFile.FullPath, loadedProjectNode.ShellProject.dteProject)) {
                                var newIncludes = this.ExtractRawIncludes(changedFile.FullPath);
                                var newSourceFile = new Document.SourceFile(changedFile.FullPath, loadedProjectNode.ProjectNode);

                                _solutionSourceFileGraph.AddSourceFileWithIncludes(newSourceFile, newIncludes);
                                Helpers.Diagnostic.Logger.LogDebug($"[auto re-added] {changedFile.FullPath} → found in {loadedProjectNode.ProjectNode.UniqueName}, graph updated by fswatcher");
                                break;
                            }
                        }
                    }
                    break;

                case Helpers.DirectoryChangeType.Deleted: {
                        if (!_solutionSourceFileGraph.TryGetSourceFileRepresentations(changedFile.FullPath, out var candidates)) {
                            return;
                        }

                        foreach (var sourceFile in candidates) {
                            _solutionSourceFileGraph.RemoveSourceFile(sourceFile);
                            Helpers.Diagnostic.Logger.LogDebug($"[deleted] {changedFile.FullPath} [{sourceFile.ProjectNode.UniqueName}] → removed from graph");
                        }
                    }
                    break;
            }
        }


        private void UpdateIncludesIfNeeded(string filePath, IReadOnlyList<Document.SourceFile> candidates) {
            var updated = new List<(Document.SourceFile OldFile, List<Document.IncludeEntry> NewIncludes)>();

            foreach (var oldFile in candidates) {
                var newIncludeEntries = this.ExtractRawIncludes(filePath);
                var oldIncludeEntries = _solutionSourceFileGraph.GetResolvedIncludes(oldFile)
                    .Select(resolvedInclude => resolvedInclude.IncludeEntry)
                    .ToList();

                bool changed = newIncludeEntries.Count != oldIncludeEntries.Count ||
                               !newIncludeEntries.SequenceEqual(oldIncludeEntries);

                if (changed) {
                    updated.Add((oldFile, newIncludeEntries));
                }
            }

            foreach (var (oldFile, newIncludeEntries) in updated) {
                var updatedFile = new Document.SourceFile(filePath, oldFile.ProjectNode);
                _solutionSourceFileGraph.UpdateSourceFileWithIncludes(updatedFile, newIncludeEntries);
                Helpers.Diagnostic.Logger.LogDebug($"[include changed] {filePath} [{oldFile.ProjectNode.UniqueName}] → includes updated");
            }
        }
    }
}