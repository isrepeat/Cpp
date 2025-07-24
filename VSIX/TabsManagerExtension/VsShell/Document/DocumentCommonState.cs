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

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private Hierarchy.HierarchyItemMultiStateElement _hierarchyItemMultiState;


        public DocumentCommonState(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemMultiStateElement hierarchyItemMultiState
            ) {
            this.ProjectNode = projectNode;

            _hierarchyItemMultiState = hierarchyItemMultiState;
            this.HierarchyItemMultiState = _hierarchyItemMultiState;
        }

        public void Dispose() {
            this.HierarchyItemMultiState.Dispose();
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
                return hash;
            }
        }
        
        public string ToStringCore() {
            this.HierarchyItemMultiState.ReadCommonState(
                Hierarchy.HierarchyItemCommonState.PropertyInfo.ItemId, out var itemId);

            this.HierarchyItemMultiState.ReadCommonState(
                Hierarchy.HierarchyItemCommonState.PropertyInfo.FilePath, out var filePath);

            return $"FilePath='{filePath}', Project='{this.ProjectNode.UniqueName}', ItemId={itemId}";
        }
    }



    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<DocumentCommonState> {
        public VsShell.Project.ProjectNode ProjectNode => this.CommonState.ProjectNode;
        public object HierarchyItemObj => this.CommonState.HierarchyItemMultiState.Current;

        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {

                case nameof(DocumentCommonState.HierarchyItemMultiState):
                    base.OnPropertyChanged(nameof(this.HierarchyItemObj));
                    break;
            }
        }
    }
}