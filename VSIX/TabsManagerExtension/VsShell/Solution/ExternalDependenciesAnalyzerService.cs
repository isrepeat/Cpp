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
using EnvDTE;


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// ExternalIncludeRepresentationsTable
    /// </summary>
    public sealed class ExternalIncludeRepresentationsTable 
        : Helpers.RepresentationsTableBase<VsShell.Document.ExternalInclude> {

        private Dictionary<string, List<VsShell.Project.ProjectNode>> _mapFilePathToListProject = new();
        private Dictionary<VsShell.Project.ProjectNode, Dictionary<string, VsShell.Document.ExternalInclude>> _mapProjectToDictFilePathToInclude = new();

        public override void BuildRepresentations() {
            _mapFilePathToListProject.Clear();
            _mapProjectToDictFilePathToInclude.Clear();

            _mapFilePathToListProject = base.Records
                .GroupBy(r => r.FilePath, StringComparer.OrdinalIgnoreCase)
                .ToDictionary(
                    g => g.Key,
                    g => g.Select(r => r.ProjectNode).Distinct().ToList(),
                    StringComparer.OrdinalIgnoreCase);

            foreach (var record in base.Records) {
                var projectNode = record.ProjectNode;
                var externalIncludePath = record.FilePath;
                var externalInclude = new VsShell.Document.ExternalInclude(
                    projectNode,
                    externalIncludePath,
                    record.ItemId);

                if (!_mapProjectToDictFilePathToInclude.TryGetValue(projectNode, out var dictFilePathToInclude)) {
                    dictFilePathToInclude = new Dictionary<string, VsShell.Document.ExternalInclude>(StringComparer.OrdinalIgnoreCase);
                    _mapProjectToDictFilePathToInclude[projectNode] = dictFilePathToInclude;
                }

                dictFilePathToInclude[externalIncludePath] = externalInclude;
            }
        }

        public IReadOnlyCollection<VsShell.Project.ProjectNode> GetAllProjects() {
            return _mapProjectToDictFilePathToInclude.Keys;
        }

        public IReadOnlyList<VsShell.Project.ProjectNode> GetProjectsByExternalIncludePath(string externalIncludePath) {
            if (_mapFilePathToListProject.TryGetValue(externalIncludePath, out var projectsList)) {
                return projectsList;
            }
            return Array.Empty<VsShell.Project.ProjectNode>();
        }

        public VsShell.Document.ExternalInclude? GetExternalIncludeByProjectAndIncludePath(VsShell.Project.ProjectNode projectNode, string externalIncludePath) {
            if (_mapProjectToDictFilePathToInclude.TryGetValue(projectNode, out var dictFilePathToInclude)) {
                if (dictFilePathToInclude.TryGetValue(externalIncludePath, out var externalInclude)) {
                    return externalInclude;
                }
            }
            return null;
        }
    }


    /// <summary>
    /// ExternalDependenciesAnalyzerService
    /// </summary>
    public class ExternalDependenciesAnalyzerService :
        TabsManagerExtension.Services.SingletonServiceBase<ExternalDependenciesAnalyzerService>,
        TabsManagerExtension.Services.IExtensionService {

        private readonly ExternalIncludeRepresentationsTable _externalIncludeRepresentationsTable = new();
        public ExternalIncludeRepresentationsTable ExternalIncludeRepresentationsTable => _externalIncludeRepresentationsTable;


        private Helpers.DirectoryWatcher? _solutionDirectoryWatcher;
        private DispatcherTimer _delayedFileChangeTimer;

        private readonly HashSet<Helpers.DirectoryChangedEventArgs> _pendingChangedFiles = new();
        private string _lastLoadedSolutionName;

        public ExternalDependenciesAnalyzerService() { }

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

            string? solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
            if (solutionDir != null && Directory.Exists(solutionDir)) {
                _solutionDirectoryWatcher = new Helpers.DirectoryWatcher(solutionDir);
                _solutionDirectoryWatcher.DirectoryChanged += this.OnSolutionDirectoryChanged;
            }

            // Используем таймер для отложенной обработки изменённых файлов:
            // обработка произойдёт только через заданный интервал после последнего события.
            _delayedFileChangeTimer = new DispatcherTimer {
                Interval = TimeSpan.FromMilliseconds(300)
            };
            _delayedFileChangeTimer.Tick += (_, _) => this.OnDelayedFileChangeTimerTick();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _delayedFileChangeTimer.Stop();
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded -= this.OnSolutionLoaded;

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Disposed.");
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public void Analyze() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_LOADEDINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];

                if (hierarchy is IVsProject vsProject) {
                    var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(hierarchy);
                    if (dteProject != null) {
                        var projectNode = new VsShell.Project.ProjectNode(dteProject, hierarchy);
                        projectNode.UpdateExternalIncludes();

                        _externalIncludeRepresentationsTable.AddRange(projectNode.ExternalIncludes);
                    }
                }
            }

            _externalIncludeRepresentationsTable.BuildRepresentations();

            this.LogGraph();
        }


        public void LogAllItemIdsViaHierarchy() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_LOADEDINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];

                string projectName = Utils.EnvDteUtils.GetDteProjectUniqueNameFromVsHierarchy(hierarchy);
                Helpers.Diagnostic.Logger.LogDebug($"[Hierarchy] {projectName} (VSITEMID_ROOT)");

                this.LogHierarchyItemsRecursive(hierarchy, VSConstants.VSITEMID_ROOT, 0);
            }
        }


        public void LogGraph() {
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Graph dump:");
            foreach (var projectNode in _externalIncludeRepresentationsTable.GetAllProjects()) {
                Helpers.Diagnostic.Logger.LogDebug($"  Project: {projectNode.Project.UniqueName}");

                foreach (var inc in projectNode.ExternalIncludes) {
                    Helpers.Diagnostic.Logger.LogDebug($"    └─ ExternalInclude: {inc.FilePath}");
                }
            }
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

            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                await Task.Delay(TimeSpan.FromSeconds(8));
                this.Analyze();
            });
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

            //this.ProcessChangedVcxProjects(ref changedFiles);

            foreach (var changedFile in changedFiles) {
                this.ProcessChangedFile(changedFile);
            }
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
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
                case Helpers.DirectoryChangeType.Changed:
                case Helpers.DirectoryChangeType.Created:
                case Helpers.DirectoryChangeType.Renamed:
                case Helpers.DirectoryChangeType.Deleted:
                    ThreadHelper.JoinableTaskFactory.Run(async () => {
                        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                        this.Analyze();
                    });
                    break;
            }
        }


        private void LogHierarchyItemsRecursive(IVsHierarchy hierarchy, uint itemId, int indent) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (itemId != VSConstants.VSITEMID_ROOT) {
                hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string ?? "(null)";

                hierarchy.GetCanonicalName(itemId, out var canonicalName);

                string indentStr = new string(' ', indent * 2);
                Helpers.Diagnostic.Logger.LogDebug($"[{itemId}]{indentStr}{name} ({canonicalName})");
            }

            foreach (var childId in Utils.VsHierarchyWalker.GetChildren(hierarchy, itemId)) {
                this.LogHierarchyItemsRecursive(hierarchy, childId, indent + 1);
            }
        }
    }
}