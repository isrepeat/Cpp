using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class UnloadedProjectNode {
        public SolutionProjectNode SolutionProjectNode { get; }
        public bool HasLoadedProjectNodeInfo { get; private set; }
        public IReadOnlyList<VsShell.Document.DocumentNode> LastSources { get; private set; }
        public IReadOnlyList<VsShell.Document.SharedItemNode> LastSharedItems { get; private set; }
        public IReadOnlyList<VsShell.Document.ExternalInclude> LastExternalIncludes { get; private set; }

        public UnloadedProjectNode(SolutionProjectNode solutionProjectNode) {
            this.SolutionProjectNode = solutionProjectNode;
            this.HasLoadedProjectNodeInfo = false;
            this.LastSources = new List<VsShell.Document.DocumentNode>();
            this.LastSharedItems = new List<VsShell.Document.SharedItemNode>();
            this.LastExternalIncludes = new List<VsShell.Document.ExternalInclude>();
        }

        public void UpdateLastInfoFromLoadedProjectNode(LoadedProjectNode associatedLoadedProjectNode) {
            if (associatedLoadedProjectNode.SolutionProjectNode != this.SolutionProjectNode) {
                return;
            }
            this.LastSources = associatedLoadedProjectNode.Sources;
            this.LastSharedItems = associatedLoadedProjectNode.SharedItems;
            this.LastExternalIncludes = associatedLoadedProjectNode.ExternalIncludes;
            this.HasLoadedProjectNodeInfo = true;
        }

        public override string ToString() {
            return $"UnloadedProjectNode({this.SolutionProjectNode.UniqueName})";
        }
    }
}