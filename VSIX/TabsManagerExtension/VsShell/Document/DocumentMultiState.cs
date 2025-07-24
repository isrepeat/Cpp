using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentMultiStateContainer :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument> {
        public DocumentMultiStateContainer(DocumentCommonState state)
            : base(state) {
        }
    }


    public class Document :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {


        public Document(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is InvalidatedDocument) {
                throw new InvalidOperationException("Switching state from InvalidatedDocument is not supported");
            }
            base.CommonState.HierarchyItemMultiState.SwitchTo<Hierarchy.HierarchyItem>();
        }
        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                base.CommonState.HierarchyItemMultiState.SwitchTo<Hierarchy.InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return this.CommonState.ToString();
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnSharedStatePropertyChanged(sender, e);

            if (e.PropertyName is nameof(DocumentCommonState.Hierarchy)) {
                base.OnPropertyChanged(nameof(this.Hierarchy));
            }
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument> {this.CommonState.ToString()}";
        }
    }
}