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
    public class SolutionHierarchyAnalyzerService :
        TabsManagerExtension.Services.SingletonServiceBase<SolutionHierarchyAnalyzerService>,
        TabsManagerExtension.Services.IExtensionService {

        private readonly List<VsShell.Project.ProjectNode> _projectNodes = new();
        public IReadOnlyList<VsShell.Project.ProjectNode> ProjectNodes => _projectNodes;


        private readonly SolutionHierarchyRepresentationsTable _sourcesRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable SourcesRepresentationsTable => _sourcesRepresentationsTable;


        private readonly SolutionHierarchyRepresentationsTable _sharedItemsRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable SharedItemsRepresentationsTable => _sharedItemsRepresentationsTable;


        private readonly SolutionHierarchyRepresentationsTable _externalIncludeRepresentationsTable = new();
        public SolutionHierarchyRepresentationsTable ExternalIncludeRepresentationsTable => _externalIncludeRepresentationsTable;


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
            };
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded += this.OnSolutionLoaded;

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

            foreach (var projectNode in _projectNodes) {
                projectNode.UpdateDocuments();
                _sourcesRepresentationsTable.AddRange(projectNode.Sources);
                _sharedItemsRepresentationsTable.AddRange(projectNode.SharedItems);
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

            foreach (var projectNode in _projectNodes) {
                projectNode.UpdateExternalIncludes();
                _externalIncludeRepresentationsTable.AddRange(projectNode.ExternalIncludes);
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

            if (solutionName == _lastLoadedSolutionName) {
                return;
            }
            _lastLoadedSolutionName = solutionName;

            this.UpdateProjectNodes();
            this.AnalyzeDocuments();
            //this.Analyze(AnalyzeType.Documents);
            //this.StartAnalyzeRoutineInBackground();
        }


        private void OnSolutionDirectoryChanged(Helpers.DirectoryChangedEventArgs e) {
            lock (_pendingChangedFiles) {
                _pendingChangedFiles.Add(e);
            }

            //_delayedFileChangeTimer.Stop();
            //_delayedFileChangeTimer.Start();
        }


        private void OnDelayedFileChangeTimerTick() {
            //_delayedFileChangeTimer.Stop();

            List<Helpers.DirectoryChangedEventArgs> changedFiles;
            lock (_pendingChangedFiles) {
                changedFiles = _pendingChangedFiles.ToList();
                _pendingChangedFiles.Clear();
            }

            foreach (var changedFile in changedFiles) {
                this.ProcessChangedFile(changedFile);
            }
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void UpdateProjectNodes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectNodes.Clear();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_LOADEDINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];

                if (hierarchy is IVsProject) {
                    var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(hierarchy);
                    if (dteProject != null) {
                        var projectNode = new VsShell.Project.ProjectNode(dteProject, hierarchy);
                        _projectNodes.Add(projectNode);
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