using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class SolutionProjectNode {
        public VsShell.Hierarchy.IVsHierarchy VsHierarchy { get; private set; }
        public Guid ProjectGuid { get; }
        public string Name { get; } = "<unknown>";
        public string UniqueName { get; } = "<unknown>";
        public string FullName { get; } = "<unknown>";
        public bool IsSharedProject { get; }

        public bool IsLoaded = false;

        public object? ProjectNodeObj => _projectNodeVariant.CurrentValue;
        public LoadedProjectNode LoadedProjectNode => _projectNodeVariant.Get<LoadedProjectNode>();
        public UnloadedProjectNode UnloadedProjectNode => _projectNodeVariant.Get<UnloadedProjectNode>();


        private readonly Helpers.Variant<LoadedProjectNode, UnloadedProjectNode> _projectNodeVariant = new();

        public SolutionProjectNode(VsShell.Hierarchy.IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.VsHierarchy = projectHierarchy;

            var vsSolution = PackageServices.VsSolution;
            var vsSolution2 = (IVsSolution2)PackageServices.VsSolution;

            // Guid
            vsSolution.GetGuidOfProject(projectHierarchy.Hierarchy, out var guid);
            this.ProjectGuid = guid;

            // Name
            projectHierarchy.Hierarchy.GetProperty(VSConstants.VSITEMID_ROOT, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            if (nameObj is string nameStr) {
                this.Name = nameStr;
            }

            // UniqueName, FullName
            string projRef = "";
            vsSolution2?.GetProjrefOfProject(projectHierarchy.Hierarchy, out projRef);

            if (!string.IsNullOrEmpty(projRef)) {
                var parts = projRef.Split('|');
                if (parts.Length > 1) {
                    this.UniqueName = parts[1];

                    var solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
                    this.FullName = Path.GetFullPath(Path.Combine(solutionDir, this.UniqueName));
                }
            }

            this.IsSharedProject = this.FullName.EndsWith(".vcxitems", StringComparison.OrdinalIgnoreCase);

            this.UpdateHierarchyState(projectHierarchy);
        }


        public void UpdateHierarchyState(VsShell.Hierarchy.IVsHierarchy associatedProjectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            PackageServices.VsSolution.GetGuidOfProject(associatedProjectHierarchy, out var guid);
            if (guid != this.ProjectGuid) {
                Helpers.Diagnostic.Logger.LogError($"[UpdateHierarchyState] guid != this.ProjectGuid");
                return;
            }

            this.IsLoaded = associatedProjectHierarchy is IVsProject;

            if (this.IsLoaded) {
                // this.VsHierarchy still may be StubHierarchy, so use
                // associatedProjectHierarchy to determine dteProject.
                var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(associatedProjectHierarchy);

                _projectNodeVariant.Set(new LoadedProjectNode(this, dteProject));
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateHierarchyState] Set LoadedProjectNode for {this.UniqueName}");
            }
            else {
                _projectNodeVariant.Set(new UnloadedProjectNode(this));
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateHierarchyState] Set UnloadedProjectNode for {this.UniqueName}");
            }
        }


        public override bool Equals(object? obj) {
            if (obj is not SolutionProjectNode other) {
                return false;
            }

            return this.ProjectGuid == other.ProjectGuid;
        }

        public override int GetHashCode() {
            return this.ProjectGuid.GetHashCode();
        }

        public override string ToString() {
            return $"SolutionProjectNode({this.UniqueName}, IsLoaded={this.IsLoaded})";
        }
    }
}