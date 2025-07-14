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
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// SolutionHierarchyRepresentationsTable
    /// </summary>
    public sealed class SolutionHierarchyRepresentationsTable
        : Helpers.RepresentationsTableBase<VsShell.Document.DocumentNode> {

        private Dictionary<string, List<VsShell.Project.ProjectNode>> _mapFilePathToListProject = new();
        private Dictionary<VsShell.Project.ProjectNode, Dictionary<string, VsShell.Document.DocumentNode>> _mapProjectToDictFilePathToDocument = new();

        public override void BuildRepresentations() {
            _mapFilePathToListProject.Clear();
            _mapProjectToDictFilePathToDocument.Clear();

            _mapFilePathToListProject = base.Records
                .GroupBy(r => r.FilePath, StringComparer.OrdinalIgnoreCase)
                .ToDictionary(
                    g => g.Key,
                    g => g.Select(r => r.ProjectNode).Distinct().ToList(),
                    StringComparer.OrdinalIgnoreCase);

            foreach (var record in base.Records) {
                var projectNode = record.ProjectNode;
                var documenPath = record.FilePath;

                if (!_mapProjectToDictFilePathToDocument.TryGetValue(projectNode, out var dictFilePathToDocument)) {
                    dictFilePathToDocument = new Dictionary<string, VsShell.Document.DocumentNode>(StringComparer.OrdinalIgnoreCase);
                    _mapProjectToDictFilePathToDocument[projectNode] = dictFilePathToDocument;
                }

                dictFilePathToDocument[documenPath] = record;
            }
        }

        public IReadOnlyCollection<VsShell.Project.ProjectNode> GetAllProjects() {
            return _mapProjectToDictFilePathToDocument.Keys;
        }

        public IReadOnlyList<VsShell.Project.ProjectNode> GetProjectsByDocumentPath(string documentPath) {
            if (_mapFilePathToListProject.TryGetValue(documentPath, out var projectsList)) {
                return projectsList;
            }
            return Array.Empty<VsShell.Project.ProjectNode>();
        }

        public VsShell.Document.DocumentNode? GetDocumentByProjectAndDocumentPath(VsShell.Project.ProjectNode projectNode, string documentPath) {
            if (_mapProjectToDictFilePathToDocument.TryGetValue(projectNode, out var dictFilePathToDocument)) {
                if (dictFilePathToDocument.TryGetValue(documentPath, out var documentNode)) {
                    return documentNode;
                }
            }
            return null;
        }
    }


    /// <summary>
    /// SolutionHierarchyAnalyzerService
    /// </summary>
    // TODO: перед использованием класса убедись что анализ ProjectNodes закончен.
    // Сделай что-то типа отложнного выполнений дальнешей логики в caller после завершения анализа, 
    // например когда происходит выгрузка / загрука проекта.
    public class SolutionHierarchyAnalyzerService :
        TabsManagerExtension.Services.SingletonServiceBase<SolutionHierarchyAnalyzerService>,
        TabsManagerExtension.Services.IExtensionService {

        private readonly Helpers.Collections.DisposableList<VsShell.Project.ProjectNode> _solutionProjects = new();
        public IReadOnlyList<VsShell.Project.ProjectNode> SolutionProjects => _solutionProjects;

        public IReadOnlyList<VsShell.Project.LoadedProjectNode> LoadedProjects =>
            _solutionProjects
                .Select(p => p.CurrentProjectNodeStateObj)
                .OfType<VsShell.Project.LoadedProjectNode>()
                .ToList();

        public IReadOnlyList<VsShell.Project.UnloadedProjectNode> UnloadedProjects =>
            _solutionProjects
                .Select(p => p.CurrentProjectNodeStateObj)
                .OfType<VsShell.Project.UnloadedProjectNode>()
                .ToList();


        private readonly SolutionHierarchyRepresentationsTable _externalIncludeRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable ExternalIncludeRepresentationsTable => _externalIncludeRepresentationsTable;


        private readonly SolutionHierarchyRepresentationsTable _sharedItemsRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable SharedItemsRepresentationsTable => _sharedItemsRepresentationsTable;


        private readonly SolutionHierarchyRepresentationsTable _sourcesRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable SourcesRepresentationsTable => _sourcesRepresentationsTable;



        //private Helpers.DirectoryWatcher? _solutionDirectoryWatcher;
        //private DispatcherTimer _delayedFileChangeTimer;

        private readonly HashSet<Helpers.DirectoryChangedEventArgs> _pendingChangedFiles = new();
        private string _lastLoadedSolutionName;
        private bool _analyzingInProcess = false;

        public SolutionHierarchyAnalyzerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return new[] {
                typeof(VsShell.Services.VsIDEStateFlagsTrackerService),
                typeof(VsShell.Solution.Services.VsSolutionEventsTrackerService),
            };
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded += this.OnSolutionLoaded;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.InvokeForLastHandlerIfTriggered();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed += this.OnSolutionClosed;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.InvokeForLastHandlerIfTriggered();

            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded += this.OnProjectLoaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded += this.OnProjectUnloaded;

            //string? solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
            //if (solutionDir != null && Directory.Exists(solutionDir)) {
            //    _solutionDirectoryWatcher = new Helpers.DirectoryWatcher(solutionDir);
            //    _solutionDirectoryWatcher.DirectoryChanged += this.OnSolutionDirectoryChanged;
            //}

            //// Используем таймер для отложенной обработки изменённых файлов:
            //// обработка произойдёт только через заданный интервал после последнего события.
            //_delayedFileChangeTimer = new DispatcherTimer {
            //    Interval = TimeSpan.FromMilliseconds(500)
            //};
            //_delayedFileChangeTimer.Tick += (_, _) => this.OnDelayedFileChangeTimerTick();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            //_delayedFileChangeTimer.Stop();
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded -= this.OnProjectUnloaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded -= this.OnProjectLoaded;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed -= this.OnSolutionClosed;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded -= this.OnSolutionLoaded;

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Disposed.");
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public void AnalyzeDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _analyzingInProcess = true;
            _sourcesRepresentationsTable.Clear();
            _sharedItemsRepresentationsTable.Clear();

            foreach (var projectNode in _solutionProjects) {
                if (projectNode.CurrentProjectNodeStateObj is VsShell.Project.LoadedProjectNode loadedProjectNode) {
                    _sourcesRepresentationsTable.AddRange(loadedProjectNode.Sources);
                    _sharedItemsRepresentationsTable.AddRange(loadedProjectNode.SharedItems);
                }
                else if (projectNode.CurrentProjectNodeStateObj is VsShell.Project.UnloadedProjectNode unloadedProjectNode) {
                    _sourcesRepresentationsTable.AddRange(unloadedProjectNode.LastSources);
                    _sharedItemsRepresentationsTable.AddRange(unloadedProjectNode.LastSharedItems);
                }
            }

            _sourcesRepresentationsTable.BuildRepresentations();
            _sharedItemsRepresentationsTable.BuildRepresentations();
            _analyzingInProcess = false;
        }


        // Держим отдельно ExternalIncludeRepresentationsTable чтобы не было пересечения filePath с SharedItems.
        // Да и к тому же ExternalIncludes часто приходится обновлять при запросе, в то время как projectNode.Sources и
        // projectNode.SharedItems обновлять можно по ивентам загрузки / выгрузки решения или изменения файлов проекта.
        public void AnalyzeExternalIncludes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _analyzingInProcess = true;
            _externalIncludeRepresentationsTable.Clear();

            foreach (var projectNode in _solutionProjects) {
                if (projectNode.CurrentProjectNodeStateObj is VsShell.Project.LoadedProjectNode loadedProjectNode) {
                    _externalIncludeRepresentationsTable.AddRange(loadedProjectNode.ExternalIncludes);
                }
                else if (projectNode.CurrentProjectNodeStateObj is VsShell.Project.UnloadedProjectNode unloadedProjectNode) {
                    _externalIncludeRepresentationsTable.AddRange(unloadedProjectNode.LastExternalIncludes);
                }
            }

            _externalIncludeRepresentationsTable.BuildRepresentations();
            _analyzingInProcess = false;
        }


        public bool IsReady() {
            return !_analyzingInProcess;
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnSolutionLoaded(string solutionName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionLoaded()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (solutionName == _lastLoadedSolutionName) {
                return;
            }
            _lastLoadedSolutionName = solutionName;

            this.AnalyzeSolutionProjects();
            this.AnalyzeDocuments();
            this.AnalyzeExternalIncludes();
        }

        private void OnSolutionClosed(string solutionName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionClosed()");
            ThreadHelper.ThrowIfNotOnUIThread();

            _solutionProjects.Clear();
        }


        private void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnProjectLoaded()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (e.TryGetRealHierarchy(out var realHierarchy)) {
                PackageServices.VsSolution.GetGuidOfProject(realHierarchy.VsHierarchy, out var projectGuid);
                var existingSolutionProject = _solutionProjects.FirstOrDefault(p => p.ProjectGuid == projectGuid);
                if (existingSolutionProject == null) {
                    return;
                }

                existingSolutionProject.UpdateHierarchy(e);
                this.AnalyzeDocuments();
                this.AnalyzeExternalIncludes();
            }               
        }

        private void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (e.TryGetRealHierarchy(out var realHierarchy)) {
                PackageServices.VsSolution.GetGuidOfProject(realHierarchy.VsHierarchy, out var projectGuid);
                var existingSolutionProject = _solutionProjects.FirstOrDefault(p => p.ProjectGuid == projectGuid);
                if (existingSolutionProject == null) {
                    return;
                }

                existingSolutionProject.UpdateHierarchy(e);
                this.AnalyzeDocuments();
                this.AnalyzeExternalIncludes();
            }
        }


        private void OnSolutionDirectoryChanged(Helpers.DirectoryChangedEventArgs e) {
            lock (_pendingChangedFiles) {
                _pendingChangedFiles.Add(e);
            }

            //_delayedFileChangeTimer.Stop();
            //_delayedFileChangeTimer.Start();
        }


        //private void OnDelayedFileChangeTimerTick() {
        //    _delayedFileChangeTimer.Stop();

        //    List<Helpers.DirectoryChangedEventArgs> changedFiles;
        //    lock (_pendingChangedFiles) {
        //        changedFiles = _pendingChangedFiles.ToList();
        //        _pendingChangedFiles.Clear();
        //    }

        //    foreach (var changedFile in changedFiles) {
        //        this.ProcessChangedFile(changedFile);
        //    }
        //}


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void AnalyzeSolutionProjects() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _solutionProjects.Clear();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_ALLINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];
                if (hierarchy == null) {
                    continue;
                }

                var pHierarchy = VsShell.Hierarchy.VsHierarchyFactory.CreateHierarchy(hierarchy);
                _solutionProjects.Add(new VsShell.Project.ProjectNode(pHierarchy));
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

            Helpers.Diagnostic.Logger.LogDebug($"changedFile = {changedFile.FullPath}");

            switch (changedFile.ChangeType) {
                case Helpers.DirectoryChangeType.Changed:
                case Helpers.DirectoryChangeType.Created:
                case Helpers.DirectoryChangeType.Renamed:
                case Helpers.DirectoryChangeType.Deleted:
                    //this.Analyze();
                    break;
            }
        }
    }
}