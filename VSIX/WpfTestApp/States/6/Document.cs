using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace WpfTestApp.States.MultistateBehaviour {
    public class DocumentCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {
        
        private HelpersV4.Collections.MultiStateContainer<
            HierarchyCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem>?
            _hierarchyMultiState;

        public HelpersV4.Collections.MultiStateContainer<
            HierarchyCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem>
            HierarchyMultiState {
            get => _hierarchyMultiState!;
            set {
                if (_hierarchyMultiState != value) {
                    if (_hierarchyMultiState != null) {
                        _hierarchyMultiState.StateChanged -= this.OnHierarchyMultiStateChanged;
                        _hierarchyMultiState.Dispose();
                    }

                    _hierarchyMultiState = value;
                    _hierarchyMultiState.StateChanged += this.OnHierarchyMultiStateChanged;

                    base.OnSharedStatePropertyChanged(nameof(this.HierarchyMultiState));
                }
            }
        }


        public DocumentCommonState(string vsHierarchyData) {
            this.HierarchyMultiState = new(new HierarchyCommonState(vsHierarchyData));
        }

        public void Dispose() {
            this.HierarchyMultiState?.Dispose();
        }

        private void OnHierarchyMultiStateChanged() {
            base.OnSharedStatePropertyChanged(nameof(this.HierarchyMultiState));
        }
    }



    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<DocumentCommonState> {

        public object HierarchyItemObj => this.CommonState.HierarchyMultiState.Current;
        public string HierarchyItemObjHash => this.CommonState.HierarchyMultiState.CurrentHash;
        public IEnumerable<string> HierarchyItemObjHashes => this.CommonState.HierarchyMultiState.InstancesHashSet.ToList();

        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
            this.UpdateHashes();
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(DocumentCommonState.HierarchyMultiState)) {
                base.OnPropertyChanged(nameof(this.HierarchyItemObj));
                this.UpdateHashes();
            }
        }

        private void UpdateHashes() {
            base.OnPropertyChanged(nameof(this.HierarchyItemObjHash));
            base.OnPropertyChanged(nameof(this.HierarchyItemObjHashes));
        }
    }



    public class Document :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public Document(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.HierarchyMultiState.SwitchTo<HierarchyItem>();
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                base.CommonState.HierarchyMultiState.SwitchTo<InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return $"<Document>";
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument>";
        }
    }
}