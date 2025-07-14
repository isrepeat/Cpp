using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Project {
    interface IProject {
        void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e);
        void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e);
    }


    public sealed class ProjectNode :
        Helpers.ObservableObject,
        IProject,
        IDisposable {

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

        private readonly Helpers.Collections.MultiStateContainer<LoadedProjectNode, UnloadedProjectNode> _projectNodeState;
        private Helpers.Collections.MultiStateContainer<LoadedProjectNode, UnloadedProjectNode> ProjectNodeState => _projectNodeState;
        public object? CurrentProjectNodeStateObj => _projectNodeState.Current;
        

        private bool _disposed = false;

        public ProjectNode(VsShell.Hierarchy.IVsHierarchy projectHierarchy) {
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

            _projectNodeState = new Helpers.Collections.MultiStateContainer<LoadedProjectNode, UnloadedProjectNode>(
                  new LoadedProjectNode(this),
                  new UnloadedProjectNode(this)
                );

            this.UpdateLoadedState();
        }


        //
        // IDisposable
        //
        public void Dispose() {
            if (_disposed) {
                return;
            }

            if (_projectNodeState.Current is IDisposable disposable) { 
                disposable.Dispose();
            }
            _projectNodeState.ForEachOther((Helpers.Collections.IMultiStateElement element) => {
                if (element is IDisposable disposable) {
                    disposable.Dispose();
                }
            });

            _disposed = true;
        }


        //
        // IProject
        //
        public void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            this.UpdateHierarchy(e);
        }

        public void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            this.UpdateHierarchy(e);
        }


        //
        // Api
        //
        public override bool Equals(object? obj) {
            if (obj is not ProjectNode other) {
                return false;
            }

            return this.ProjectGuid == other.ProjectGuid;
        }

        public override int GetHashCode() {
            return this.ProjectGuid.GetHashCode();
        }

        public override string ToString() {
            return $"ProjectNode({this.UniqueName}, IsLoaded={this.IsLoaded})";
        }


        //
        // Internal logic
        //
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


        private void UpdateLoadedState() {
            if (this.ProjectHierarchy is VsShell.Hierarchy.IVsRealHierarchy) {
                _projectNodeState.SwitchTo<LoadedProjectNode>();

                this.IsLoaded = true;
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateLoadedState] Set LoadedProjectNode for {this.UniqueName}");
            }
            else { // (this.ProjectHierarchy is VsShell.Hierarchy.IVsStubHierarchy)
                _projectNodeState.SwitchTo<UnloadedProjectNode>();

                this.IsLoaded = false;
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateLoadedState] Set UnloadedProjectNode for {this.UniqueName}");
            }
        }
    }
}