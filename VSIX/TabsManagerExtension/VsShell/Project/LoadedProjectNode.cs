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


        private ProjectHierarchyTracker _projectHierarchyTracker;

        private bool _disposed = false;

        public LoadedProjectNode(ProjectNode projectNode) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.ProjectNode = projectNode;
        }

        //
        // IDisposable
        //
        public void Dispose() {
            if (_disposed) {
                return;
            }

            this.OnStateDisabled(null);
            
            _disposed = true;
        }


        //
        // IMultiStateElement
        //
        public void OnStateEnabled(Helpers.Collections.IMultiStateElement previousState) {
            // When project loaded

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            _projectHierarchyTracker = new ProjectHierarchyTracker(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            _projectHierarchyTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;
            _projectHierarchyTracker.SharedItemsChanged += this.OnSharedItemsChanged;
            _projectHierarchyTracker.SourcesChanged += this.OnSourcesChanged;

            _projectHierarchyTracker.ExternalDependenciesChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SharedItemsChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SourcesChanged.InvokeForLastHandlerIfTriggered();
        }

        public void OnStateDisabled(Helpers.Collections.IMultiStateElement nextState) {
            // When project unloaded

            if (_projectHierarchyTracker != null) {
                _projectHierarchyTracker.SourcesChanged -= this.OnSourcesChanged;
                _projectHierarchyTracker.SharedItemsChanged -= this.OnSharedItemsChanged;
                _projectHierarchyTracker.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
            }

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
    }
}