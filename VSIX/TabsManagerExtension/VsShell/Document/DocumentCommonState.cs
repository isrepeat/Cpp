using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using Helpers.Ex;


namespace TabsManagerExtension.VsShell.Document {
    namespace _Details {
        public partial class DocumentCommonState :
            Helpers.MultiState.StaticCommonStateBase,
            IDisposable {
            public Project.ProjectCommonStateViewModel ProjectBaseViewModel { get; set; }
            public Hierarchy.HierarchyItemEntry HierarchyItemEntry { get; }

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private bool _isDisposed;

            public DocumentCommonState(
                Project.ProjectCommonStateViewModel projectBaseViewModel,
                Hierarchy.HierarchyItemEntry hierarchyItemEntry
                ) {
                this.ProjectBaseViewModel = projectBaseViewModel;
                this.HierarchyItemEntry = hierarchyItemEntry;
            }

            public void Dispose() {
                if (this.IsDisposed) {
                    return;
                }
                this.HierarchyItemEntry.Dispose();
                this.IsDisposed = true;
            }


            public override bool Equals(object? obj) {
                if (obj is DocumentCommonState other) {
                    return
                        this.ProjectBaseViewModel.ProjectGuid == other.ProjectBaseViewModel.ProjectGuid &&
                        this.HierarchyItemEntry.BaseViewModel.ItemId == other.HierarchyItemEntry.BaseViewModel.ItemId &&
                        StringComparer.OrdinalIgnoreCase.Equals(
                            this.HierarchyItemEntry.BaseViewModel.FilePath,
                            other.HierarchyItemEntry.BaseViewModel.FilePath);
                }
                return false;
            }


            public override int GetHashCode() {
                unchecked {
                    int hash = 17;

                    hash = hash * 31 + this.ProjectBaseViewModel.ProjectGuid.GetHashCode();
                    hash = hash * 31 + this.HierarchyItemEntry.BaseViewModel.ItemId.GetHashCode();
                    hash = hash * 31 + (this.HierarchyItemEntry.BaseViewModel.FilePath != null
                        ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.HierarchyItemEntry.BaseViewModel.FilePath)
                        : 0);

                    return hash;
                }
            }

            public string ToStringCore() {
                if (this.IsDisposed) {
                    return "<disposed>";
                }

                var projectUniqueName = this.ProjectBaseViewModel.UniqueName;
                var filePath = System.IO.Path.GetFileName(this.HierarchyItemEntry.BaseViewModel.FilePath);
                var itemId = this.HierarchyItemEntry.BaseViewModel.ItemId;

                return $"Project='{projectUniqueName}', ItemId={itemId}, FileName='{filePath}'";
            }
        }
    } // namesace _Details



    public abstract class DocumentCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<_Details.DocumentCommonState> {
        public Project.ProjectCommonStateViewModel ProjectBaseViewModel => base.CommonState.ProjectBaseViewModel;
        public Hierarchy.HierarchyItemEntry HierarchyItemEntry => base.CommonState.HierarchyItemEntry;
        public bool IsDisposed => base.CommonState.IsDisposed;

        protected DocumentCommonStateViewModel(_Details.DocumentCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(_Details.DocumentCommonState.IsDisposed):
                    base.OnPropertyChanged(nameof(this.IsDisposed));
                    break;
            }
        }
    }
}