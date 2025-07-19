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

            _externalIncludes.ClearAndDispose();
            _sharedItems.ClearAndDispose();
            _sources.ClearAndDispose();

            _disposed = true;
        }


        //
        // IMultiStateElement
        //
        public void OnStateEnabled(Helpers.Collections.IMultiStateElement previousState) {
            Helpers.Diagnostic.Logger.LogDebug($"OnStateEnabled(): {this}");
            // When project loaded

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            _projectHierarchyTracker = new ProjectHierarchyTracker(
                this.ProjectNode.ProjectHierarchy.VsHierarchy,
                _externalIncludes.Select(d => d.HierarchyItem).ToHashSet(),
                _sharedItems.Select(d => d.HierarchyItem).ToHashSet(),
                _sources.Select(d => d.HierarchyItem).ToHashSet()
                );

            _projectHierarchyTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;
            _projectHierarchyTracker.SharedItemsChanged += this.OnSharedItemsChanged;
            _projectHierarchyTracker.SourcesChanged += this.OnSourcesChanged;

            _projectHierarchyTracker.ExternalDependenciesChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SharedItemsChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SourcesChanged.InvokeForLastHandlerIfTriggered();
        }

        public void OnStateDisabled(Helpers.Collections.IMultiStateElement nextState) {
            Helpers.Diagnostic.Logger.LogDebug($"OnStateDisabled(): {this}");
            // When project unloaded

            if (_projectHierarchyTracker != null) {
                _projectHierarchyTracker.SourcesChanged -= this.OnSourcesChanged;
                _projectHierarchyTracker.SharedItemsChanged -= this.OnSharedItemsChanged;
                _projectHierarchyTracker.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
            }

            foreach (var documentNode in _externalIncludes) {
                documentNode.Detach(Helpers.TypeMarker<Hierarchy.HierarchyItem>.Instance);
            }
            foreach (var documentNode in _sharedItems) {
                documentNode.Detach(Helpers.TypeMarker<Hierarchy.HierarchyItem>.Instance);
            }
            foreach (var documentNode in _sources) {
                documentNode.Detach(Helpers.TypeMarker<Hierarchy.HierarchyItem>.Instance);
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
            Helpers.Collections.DisposableList<TDocumentNode> currentDocumentNodes,
            _EventArgs.ProjectHierarchyItemsChangedEventArgs e,
            Func<Hierarchy.HierarchyItem, TDocumentNode> createNewNodeFactory)
            where TDocumentNode : VsShell.Document.DocumentNode {

            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var hierarchyItem in e.Added) {
                var existNode = currentDocumentNodes
                    .FirstOrDefault(d => string.Equals(d.HierarchyItem.FilePath, hierarchyItem.FilePath, StringComparison.OrdinalIgnoreCase));

                if (existNode != null) {
                    existNode.Attach(hierarchyItem);
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Refresh: {existNode} [{this.ProjectNode.UniqueName}]");
                }
                else {
                    var newNode = createNewNodeFactory(hierarchyItem);
                    currentDocumentNodes.Add(newNode);
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Add: {newNode} [{this.ProjectNode.UniqueName}]");
                }
            }

            foreach (var hierarchyItem in e.Removed) {
                var existNode = currentDocumentNodes
                    .FirstOrDefault(d => string.Equals(d.HierarchyItem.FilePath, hierarchyItem.FilePath, StringComparison.OrdinalIgnoreCase));

                currentDocumentNodes.RemoveAndDispose(existNode);
            }
        }


        private VsShell.Document.ExternalInclude CreateExternalInclude(Hierarchy.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var newExternalInclude = new VsShell.Document.ExternalInclude(
                this.ProjectNode,
                hierarchyItem
            );
            return newExternalInclude;
        }


        private VsShell.Document.SharedItemNode CreateSharedItem(Hierarchy.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var newSharedItem = new VsShell.Document.SharedItemNode(
                this.ProjectNode,
                hierarchyItem
            );
            return newSharedItem;
        }


        private VsShell.Document.DocumentNode CreateSource(Hierarchy.HierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var newSource = new VsShell.Document.DocumentNode(
                this.ProjectNode,
                hierarchyItem
            );
            return newSource;
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
    }
}