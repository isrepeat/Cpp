using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class SolutionProjectNode : Helpers.ObservableObject {
        public VsShell.Hierarchy.IVsHierarchy ProjectHierarchy { get; private set; }
        public Guid ProjectGuid { get; }
        public string Name { get; } = "<unknown>";
        public string UniqueName { get; } = "<unknown>";
        public string FullName { get; } = "<unknown>";
        public bool IsSharedProject { get; }


        private bool _isLoaded = false;
        public bool IsLoaded {
            get => _isLoaded;
            private set {
                if (_isLoaded != value) {
                    _isLoaded = value;
                    OnPropertyChanged();
                }
            }
        }

        private readonly Helpers.Variant<LoadedProjectNode, UnloadedProjectNode> _projectNodeVariant = new();
        public object? ProjectNodeObj => _projectNodeVariant.CurrentValue;
        public LoadedProjectNode LoadedProjectNode => _projectNodeVariant.Get<LoadedProjectNode>();
        public UnloadedProjectNode UnloadedProjectNode => _projectNodeVariant.Get<UnloadedProjectNode>();

        private LoadedProjectNode _cachedLoadedProjectNode;
        private UnloadedProjectNode _cachedUnloadedProjectNode;


        public SolutionProjectNode(VsShell.Hierarchy.IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.ProjectHierarchy = projectHierarchy;

            var vsSolution = PackageServices.VsSolution;
            var vsSolution2 = (IVsSolution2)PackageServices.VsSolution;

            // Guid
            vsSolution.GetGuidOfProject(projectHierarchy.VsHierarchy, out var guid);
            this.ProjectGuid = guid;

            // Name
            projectHierarchy.VsHierarchy.GetProperty(VSConstants.VSITEMID_ROOT, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            if (nameObj is string nameStr) {
                this.Name = nameStr;
            }

            // UniqueName, FullName
            string projRef = "";
            vsSolution2?.GetProjrefOfProject(projectHierarchy.VsHierarchy, out projRef);

            if (!string.IsNullOrEmpty(projRef)) {
                var parts = projRef.Split('|');
                if (parts.Length > 1) {
                    this.UniqueName = parts[1];

                    var solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
                    this.FullName = Path.GetFullPath(Path.Combine(solutionDir, this.UniqueName));
                }
            }

            this.IsSharedProject = this.FullName.EndsWith(".vcxitems", StringComparison.OrdinalIgnoreCase);
 
            this.UpdateLoadedState();
        }


        public void UpdateHierarchy(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (e.TryGetRealHierarchy(out var realHierarchy)) {
                PackageServices.VsSolution.GetGuidOfProject(realHierarchy.VsHierarchy, out var guid);
                if (guid != this.ProjectGuid) {
                    Helpers.Diagnostic.Logger.LogError($"[UpdateHierarchy] guid != this.ProjectGuid");
                    return;
                }

                this.ProjectHierarchy = e.NewHierarchy;
                this.UpdateLoadedState();
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


        private void UpdateLoadedState() {
            if (this.ProjectHierarchy is VsShell.Hierarchy.IVsRealHierarchy) {
                var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectHierarchy.VsHierarchy);

                if (_cachedLoadedProjectNode == null) {
                    _cachedLoadedProjectNode = new LoadedProjectNode(this, dteProject);
                }
                else {
                    var allProjectDocumentNodes = _cachedLoadedProjectNode.Sources
                        .Concat(_cachedLoadedProjectNode.SharedItems)
                        .Concat(_cachedLoadedProjectNode.ExternalIncludes);

                    foreach (var documentNode in allProjectDocumentNodes) {
                        documentNode.TryRefreshItemId();
                    }
                }

                _projectNodeVariant.Set(_cachedLoadedProjectNode);

                this.IsLoaded = true;
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateLoadedState] Set LoadedProjectNode for {this.UniqueName}");
            }
            else { // (this.ProjectHierarchy is VsShell.Hierarchy.IVsStubHierarchy)
                if (_cachedUnloadedProjectNode == null) {
                    _cachedUnloadedProjectNode = new UnloadedProjectNode(this);
                }

                _projectNodeVariant.Set(_cachedUnloadedProjectNode);
                
                this.IsLoaded = false;
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateLoadedState] Set UnloadedProjectNode for {this.UniqueName}");
            }
        }
    }
}