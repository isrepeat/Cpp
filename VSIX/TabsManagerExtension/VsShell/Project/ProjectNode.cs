using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Helpers.Attributes;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace TabsManagerExtension.VsShell.Project {
    interface IProject {
        void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e);
        void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e);
    }


    public sealed partial class ProjectNode :
        Helpers.ObservableObject,
        IProject,
        IDisposable {

        public VsShell.Hierarchy.IHierarchy ProjectHierarchy { get; private set; }
        public Guid ProjectGuid { get; }
        public string Name { get; }
        public string UniqueName { get; }
        public string FullName { get; }
        public bool IsSharedProject { get; }

        [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private bool _isLoaded = false;

        public ProjectEntry ProjectEntry { get; }
        

        private bool _disposed = false;

        public ProjectNode(VsShell.Hierarchy.IHierarchy projectHierarchy) {
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

            this.ProjectEntry = new ProjectEntry(new ProjectMultiStateElement(this));

            this.UpdateLoadedState();
        }


        //
        // IDisposable
        //
        public void Dispose() {
            if (_disposed) {
                return;
            }

            this.ProjectEntry.Dispose();

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


        private void OnCommonStatePropertyChanged([CallerMemberName] string? propertyName = null) {
            // Dummy ...
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
            if (this.ProjectHierarchy is VsShell.Hierarchy.IRealHierarchy) {
                this.ProjectEntry.MultiState.SwitchTo<LoadedProject>();
                this.IsLoaded = true;
            }
            else { // (this.ProjectHierarchy is VsShell.Hierarchy.IVsStubHierarchy)
                this.ProjectEntry.MultiState.SwitchTo<UnloadedProject>();
                this.IsLoaded = false;
            }
        }
    }
}