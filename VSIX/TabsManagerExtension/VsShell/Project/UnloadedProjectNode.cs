using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    //public sealed partial class SolutionProjectNode {
        public sealed class UnloadedProjectNode {
            public SolutionProjectNode SolutionProjectNode { get; }

            public UnloadedProjectNode(SolutionProjectNode solutionProjectNode) {
                this.SolutionProjectNode = solutionProjectNode;
            }

            public override string ToString() {
                return $"UnloadedProjectNode({this.SolutionProjectNode.UniqueName})";
            }
        }
    //}
}