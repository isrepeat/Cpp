using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Helpers.Attributes;


namespace WpfTestApp.States.MultistateBehaviour {
    public class DocumentMultiStateElement :
        Helpers.MultiState.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument> {
        public DocumentMultiStateElement(string vsHierarchyData)
            : base(new DocumentCommonState(vsHierarchyData)) {
        }
    }

    public class ExteralIncludeMultiStateElement :
        Helpers.MultiState.MultiStateContainer<
            DocumentCommonState,
            ExternalInclude,
            InvalidatedDocument> {
        public ExteralIncludeMultiStateElement(string vsHierarchyData)
            : base(new DocumentCommonState(vsHierarchyData)) {
        }
    }



    public partial class DocumentCommonState :
        Helpers.MultiState._Details.CommonStateBase,
        IDisposable {

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private Helpers.MultiState.MultiStateContainer<
            HierarchyCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem
            > _hierarchyMultiState;

        public DocumentCommonState(string vsHierarchyData) {
            this.HierarchyMultiState = new(new HierarchyCommonState(vsHierarchyData));
        }

        public void Dispose() {
            this.HierarchyMultiState?.Dispose();
        }
    }



    public abstract class DocumentCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<DocumentCommonState> {

        public object HierarchyItemObj => this.CommonState.HierarchyMultiState.Current;

        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(DocumentCommonState.HierarchyMultiState)) {
                base.OnPropertyChanged(nameof(this.HierarchyItemObj));
            }
        }
    }




    public class Document :
        DocumentCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public Document(DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.HierarchyMultiState.SwitchTo<HierarchyItem>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                base.CommonState.HierarchyMultiState.SwitchTo<InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return $"<Document>";
        }
    }


    public class ExternalInclude : Document {
        public ExternalInclude(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }
        public override string ToString() {
            return $"<ExternalInclude>";
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument>";
        }
    }
}