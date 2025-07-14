using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class UnloadedProjectNode :
        Helpers.ObservableObject,
        Helpers.Collections.IMultiStateElement,
        IDisposable {

        public ProjectNode ProjectNode { get; }
        public bool HasLoadedProjectNodeInfo { get; private set; }
        public IReadOnlyList<VsShell.Document.DocumentNode> LastSources { get; private set; }
        public IReadOnlyList<VsShell.Document.SharedItemNode> LastSharedItems { get; private set; }
        public IReadOnlyList<VsShell.Document.ExternalInclude> LastExternalIncludes { get; private set; }
        
        private bool _disposed = false;

        public UnloadedProjectNode(ProjectNode projectNode) {
            this.ProjectNode = projectNode;
            this.HasLoadedProjectNodeInfo = false;
            this.LastSources = new List<VsShell.Document.DocumentNode>();
            this.LastSharedItems = new List<VsShell.Document.SharedItemNode>();
            this.LastExternalIncludes = new List<VsShell.Document.ExternalInclude>();
        }

        //
        // IDisposable
        //
        public void Dispose() {
            if (_disposed) {
                return;
            }
            _disposed = true;
        }


        //
        // IMultiStateElement
        //
        public void OnStateEnabled(Helpers.Collections.IMultiStateElement previousState) {
            if (previousState is LoadedProjectNode loadedProjectNode) {
                this.UpdateLastInfoFromLoadedProjectNode(loadedProjectNode);
            }
        }

        public void OnStateDisabled(Helpers.Collections.IMultiStateElement nextState) {
            this.LastSources = new List<VsShell.Document.DocumentNode>();
            this.LastSharedItems = new List<VsShell.Document.SharedItemNode>();
            this.LastExternalIncludes = new List<VsShell.Document.ExternalInclude>();
        }


        private void UpdateLastInfoFromLoadedProjectNode(LoadedProjectNode associatedLoadedProjectNode) {
            if (associatedLoadedProjectNode.ProjectNode != this.ProjectNode) {
                return;
            }
            this.LastSources = associatedLoadedProjectNode.Sources;
            this.LastSharedItems = associatedLoadedProjectNode.SharedItems;
            this.LastExternalIncludes = associatedLoadedProjectNode.ExternalIncludes;
            this.HasLoadedProjectNodeInfo = true;
        }

        public override string ToString() {
            return $"UnloadedProjectNode({this.ProjectNode.UniqueName})";
        }
    }
}