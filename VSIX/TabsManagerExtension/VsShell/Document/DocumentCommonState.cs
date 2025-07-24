using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Document {
    public partial class DocumentCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        public VsShell.Project.ProjectNode ProjectNode { get; }

        [ObservableMultiStateProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private Hierarchy.HierarchyItemMultiStateContainer _hierarchyItemMultiState;

        public DocumentCommonState(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemMultiStateContainer hierarchyItemMultiState
            ) {
            this.ProjectNode = projectNode;

            _hierarchyItemMultiState = hierarchyItemMultiState;
            this.HierarchyItemMultiState = _hierarchyItemMultiState;
        }

        public void Dispose() {
        }


        public override bool Equals(object? obj) {
            if (obj is not DocumentCommonState other) {
                return false;
            }

            return this.ProjectNode.ProjectGuid == other.ProjectNode.ProjectGuid;
        }


        public override int GetHashCode() {
            unchecked {
                int hash = 17;

                hash = hash * 31 + this.ProjectNode.ProjectGuid.GetHashCode();
                //hash = hash * 31 + this.HierarchyItemCached.ItemIdCached.GetHashCode();
                //hash = hash * 31 + (this.HierarchyItemCached.FilePathCached != null
                //    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.HierarchyItemCached.FilePathCached)
                //    : 0);

                return hash;
            }
        }

        public override string ToString() {
            return $"DocumentNode({this.ToStringCore()})";
        }

        protected string ToStringCore() {
            return $"FilePath='{this.HierarchyItemMultiState.FilePath}', Project='{this.ProjectNode.UniqueName}', ItemId={this.HierarchyMultiState.ItemId}";
        }
    }


    
    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<DocumentCommonState> {
        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            // ...
        }
    }
}