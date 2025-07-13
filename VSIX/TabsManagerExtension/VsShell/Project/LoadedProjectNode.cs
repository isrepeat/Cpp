using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class LoadedProjectNode :
        Helpers.ObservableObject,
        Helpers.Collections.IMultiStateElement,
        IDisposable {

        public ProjectNode ProjectNode { get; }


        private ShellProject? _shellProject = null;
        public ShellProject ShellProject { 
            get {
                if (_shellProject == null) {
                    System.Diagnostics.Debugger.Break();
                }
                return _shellProject;
            }
            private set {
                if (_shellProject != value) {
                    _shellProject = value;
                }
            }
        }

        private readonly Helpers.Collections.DisposableList<VsShell.Document.ExternalInclude> _externalIncludes = new();
        public IReadOnlyList<VsShell.Document.ExternalInclude> ExternalIncludes => _externalIncludes;


        private readonly Helpers.Collections.DisposableList<VsShell.Document.SharedItemNode> _sharedItems = new();
        public IReadOnlyList<VsShell.Document.SharedItemNode> SharedItems => _sharedItems;


        private readonly Helpers.Collections.DisposableList<VsShell.Document.DocumentNode> _sources = new();
        public IReadOnlyList<VsShell.Document.DocumentNode> Sources => _sources;

        //public bool IsIncludeSharedItems => _sharedItems.Count > 0;


        private ProjectExternalDependenciesTracker _projectExternalDependenciesTracker;
        private ProjectSharedItemsTracker _projectSharedItemsTracker;
        private ProjectSourcesTracker _projectSourcesTracker;

        public LoadedProjectNode(ProjectNode projectNode) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.ProjectNode = projectNode;
        }

        //
        // IDisposable
        //
        public void Dispose() {
            this.OnStateDisabled(null);
        }


        //
        // IMultiStateElement
        //
        public void OnStateEnabled(Helpers.Collections.IMultiStateElement previousState) {
            // When project loaded

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            _projectExternalDependenciesTracker = new ProjectExternalDependenciesTracker(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            _projectExternalDependenciesTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;

            _projectSharedItemsTracker = new ProjectSharedItemsTracker(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            _projectSharedItemsTracker.SharedItemsChanged += this.OnSharedItemsChanged;

            _projectSourcesTracker = new ProjectSourcesTracker(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            _projectSourcesTracker.SourcesChanged += this.OnSourcesChanged;

            TabsManagerExtension.Services.TimeManagerService.Instance.Subscribe(Enums.TimerType._3s, this.OnRefreshExternalDependencies);
            
            _projectSharedItemsTracker.Refresh(); // TODO: call also when project add / remove references.
            _projectSourcesTracker.Refresh(); // TODO: call also when project add / remove items.
        }

        public void OnStateDisabled(Helpers.Collections.IMultiStateElement nextState) {
            // When project unloaded

            TabsManagerExtension.Services.TimeManagerService.Instance.Unsubscribe(Enums.TimerType._3s, this.OnRefreshExternalDependencies);

            _projectExternalDependenciesTracker.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
            _projectExternalDependenciesTracker = null;

            _projectSharedItemsTracker.SharedItemsChanged -= this.OnSharedItemsChanged;
            _projectSharedItemsTracker = null;

            _projectSourcesTracker.SourcesChanged -= this.OnSourcesChanged;
            _projectSourcesTracker = null;

            foreach (var item in _externalIncludes) {
                item.IsEnabled = false;
            }
            foreach (var item in _sharedItems) {
                item.IsEnabled = false;
            }
            foreach (var item in _sources) {
                item.IsEnabled = false;
            }

            this.ShellProject = null;
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public override string ToString() {
            return $"LoadedProjectNode({this.ProjectNode.UniqueName})";
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnRefreshExternalDependencies() {
            _projectExternalDependenciesTracker.Refresh();
        }

        private void OnExternalDependenciesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(_externalIncludes, e, this.CreateExternalInclude);
        }

        private void OnSharedItemsChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(_sharedItems, e, this.CreateSharedItem);
        }

        private void OnSourcesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(_sources, e, this.CreateSource);
        }


        private void UpdateProjectHierarchyItems<TDocumentNode>(
            List<TDocumentNode> list,
            _EventArgs.ProjectHierarchyItemsChangedEventArgs e,
            Func<Utils.VsHierarchyUtils.HierarchyItem, TDocumentNode> createNewNodeFactory)
            where TDocumentNode : VsShell.Document.DocumentNode {

            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var hierarchyItem in e.Added) {
                hierarchyItem.CalculateNormilizedPath();

                var existNode = list
                    .FirstOrDefault(item => string.Equals(item.FilePath, hierarchyItem.NormalizedPath, StringComparison.OrdinalIgnoreCase));

                if (existNode != null) {
                    existNode.IsEnabled = true;
                    existNode.Update(hierarchyItem);
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Refresh: {existNode} [{this.ProjectNode.UniqueName}]");
                }
                else {
                    var newNode = createNewNodeFactory(hierarchyItem);
                    list.Add(newNode);
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Add: {newNode} [{this.ProjectNode.UniqueName}]");
                }
            }

            foreach (var hierarchyItem in e.Removed) {
                hierarchyItem.CalculateNormilizedPath();

                var existNode = list
                    .FirstOrDefault(item => string.Equals(item.FilePath, hierarchyItem.NormalizedPath, StringComparison.OrdinalIgnoreCase));

                list.Remove(existNode);
            }
        }


        private VsShell.Document.ExternalInclude CreateExternalInclude(Utils.VsHierarchyUtils.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var newExternalInclude = new VsShell.Document.ExternalInclude(
                hierarchyItem.ItemId,
                hierarchyItem.NormalizedPath,
                this.ProjectNode
            );
            return newExternalInclude;
        }


        private VsShell.Document.SharedItemNode CreateSharedItem(Utils.VsHierarchyUtils.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            IVsHierarchy? sharedProjectHierarchy = null;
            this.ProjectNode.ProjectHierarchy.VsHierarchy.GetProperty(
                hierarchyItem.ItemId,
                (int)__VSHPROPID7.VSHPROPID_SharedProjectHierarchy,
                out var sharedProjectHierarchyObj);

            if (sharedProjectHierarchyObj is IVsHierarchy hierarchy) {
                sharedProjectHierarchy = hierarchy;
            }

            var newSharedItem = new VsShell.Document.SharedItemNode(
                hierarchyItem.ItemId,
                hierarchyItem.NormalizedPath,
                this.ProjectNode,
                sharedProjectHierarchy
            );
            return newSharedItem;
        }


        private VsShell.Document.DocumentNode CreateSource(Utils.VsHierarchyUtils.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var newSource = new VsShell.Document.DocumentNode(
                hierarchyItem.ItemId,
                hierarchyItem.NormalizedPath,
                this.ProjectNode
            );
            return newSource;
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private bool IsGuidName(string? name) {
            return !string.IsNullOrEmpty(name) && name.StartsWith("{") && name.EndsWith("}");
        }

        private bool IsHeaderOrCppFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}