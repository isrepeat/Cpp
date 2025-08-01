using System;
using System.IO;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using System.Collections.Generic;
using Helpers.Ex;


namespace TabsManagerExtension.VsShell.Project {
    namespace _Details {
        public partial class ProjectCommonState :
            Helpers.MultiState.StaticCommonStateBase,
            IDisposable {

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private VsShell.Hierarchy.HierarchyItemEntry _projectHierarchy;

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _name;

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _uniqueName;

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _fullName;
            public Guid ProjectGuid { get; }
            public bool IsSharedProject { get; }
            public Helpers.Collections.DisposableList<Document.ExternalIncludeEntry> ExternalIncludes { get; } = new();
            public Helpers.Collections.DisposableList<Document.SharedItemEntry> SharedItems { get; } = new();
            public Helpers.Collections.DisposableList<Document.DocumentEntry> Sources { get; } = new();

            public readonly Helpers.Events.Action<IReadOnlyList<Document.ExternalIncludeEntry>> ExternalIncludesChanged = new();
            public readonly Helpers.Events.Action<IReadOnlyList<Document.SharedItemEntry>> SharedItemsChanged = new();
            public readonly Helpers.Events.Action<IReadOnlyList<Document.DocumentEntry>> SourcesChanged = new();

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private bool _isDisposed;

            public ProjectCommonState(
                VsShell.Hierarchy.HierarchyItemEntry projectHierarchy
                ) {
                ThreadHelper.ThrowIfNotOnUIThread();

                this.ProjectHierarchy = projectHierarchy;
                
                IVsHierarchy vsProjectHierarchy = null;

                if (this.ProjectHierarchy.MultiState.Current is Hierarchy.RealHierarchyItem realHierarchyItem) {
                    vsProjectHierarchy = realHierarchyItem.VsRealHierarchy;
                }
                else if (this.ProjectHierarchy.MultiState.Current is Hierarchy.StubHierarchyItem stubHierarchyItem) {
                    vsProjectHierarchy = stubHierarchyItem.VsStubHierarchy;
                }
                else {
                    throw new InvalidOperationException("Unsupported hierarchyItem state.");
                }

                var vsSolution = PackageServices.VsSolution;
                var vsSolution2 = (IVsSolution2)PackageServices.VsSolution;

                // Name
                vsProjectHierarchy.GetProperty(VSConstants.VSITEMID_ROOT, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                if (nameObj is string nameStr) {
                    this.Name = nameStr;
                }

                // UniqueName, FullName
                string projRef = "";
                vsSolution2?.GetProjrefOfProject(vsProjectHierarchy, out projRef);

                if (!string.IsNullOrEmpty(projRef)) {
                    var parts = projRef.Split('|');
                    if (parts.Length > 1) {
                        this.UniqueName = parts[1];

                        var solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
                        this.FullName = Path.GetFullPath(Path.Combine(solutionDir, this.UniqueName));
                    }
                }

                // Guid
                vsSolution.GetGuidOfProject(vsProjectHierarchy, out var guid);
                this.ProjectGuid = guid;

                this.IsSharedProject = this.FullName.EndsWith(".vcxitems", StringComparison.OrdinalIgnoreCase);
            }

            public void Dispose() {
                if (this.IsDisposed) {
                    return;
                }

                this.ProjectHierarchy.Dispose();
                this.Name = "<disposed>";
                this.UniqueName = "<disposed>";
                this.FullName = "<disposed>";
                this.ExternalIncludes.ClearAndDispose();
                this.SharedItems.ClearAndDispose();
                this.Sources.ClearAndDispose();

                this.IsDisposed = true;
            }

            public override bool Equals(object? obj) {
                if (obj is not ProjectCommonState other) {
                    return false;
                }
                return this.ProjectGuid == other.ProjectGuid;
            }

            public override int GetHashCode() {
                return this.ProjectGuid.GetHashCode();
            }
        }
    } // namesace _Details



    public abstract class ProjectCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<_Details.ProjectCommonState> {

        public string Name => base.CommonState.Name;
        public string UniqueName => base.CommonState.UniqueName;
        public string FullName => base.CommonState.FullName;
        public Guid ProjectGuid => base.CommonState.ProjectGuid;
        public bool IsSharedProject => base.CommonState.IsSharedProject;
        
        public IReadOnlyList<Document.ExternalIncludeEntry> ExternalIncludes => base.CommonState.ExternalIncludes;
        public IReadOnlyList<Document.SharedItemEntry> SharedItems => base.CommonState.SharedItems;
        public IReadOnlyList<Document.DocumentEntry> Sources => base.CommonState.Sources;

        public Helpers.Events.Action<IReadOnlyList<Document.ExternalIncludeEntry>> ExternalIncludesChanged => base.CommonState.ExternalIncludesChanged;
        public Helpers.Events.Action<IReadOnlyList<Document.SharedItemEntry>> SharedItemsChanged => base.CommonState.SharedItemsChanged;
        public Helpers.Events.Action<IReadOnlyList<Document.DocumentEntry>> SourcesChanged => base.CommonState.SourcesChanged;
        
        public bool IsDisposed => base.CommonState.IsDisposed;

        protected ProjectCommonStateViewModel(_Details.ProjectCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(_Details.ProjectCommonState.Name):
                    base.OnPropertyChanged(nameof(this.Name));
                    break;

                case nameof(_Details.ProjectCommonState.UniqueName):
                    base.OnPropertyChanged(nameof(this.UniqueName));
                    break;

                case nameof(_Details.ProjectCommonState.FullName):
                    base.OnPropertyChanged(nameof(this.FullName));
                    break;

                case nameof(_Details.ProjectCommonState.IsDisposed):
                    base.OnPropertyChanged(nameof(this.IsDisposed));
                    break;
            }
        }
    }
}