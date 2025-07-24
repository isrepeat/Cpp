using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using System.Threading.Tasks;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument> {
        public DocumentMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemMultiStateElement hierarchyItemMultiState
            ) : base(new DocumentCommonState(projectNode, hierarchyItemMultiState)) {
        }
    }


    public class SharedItemMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            SharedItem,
            InvalidatedDocument> {
        public SharedItemMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemMultiStateElement hierarchyItemMultiState
            ) : base(new DocumentCommonState(projectNode, hierarchyItemMultiState)) {
        }
    }


    public class ExteralIncludeMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            ExternalInclude,
            InvalidatedDocument> {
        public ExteralIncludeMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemMultiStateElement hierarchyItemMultiState
            ) : base(new DocumentCommonState(projectNode, hierarchyItemMultiState)) {
        }
    }




    public partial class Document :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private bool _isOppenedWithProjectContext = false;

        public Document(DocumentCommonState commonState) : base(commonState) {
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
            return $"Document({base.CommonState.ToStringCore()})";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);
        }

        protected void OpenWithProjectContext() {
            // ...
        }
    }



    //
    // SharedItem
    //
    public sealed class SharedItem : Document {
        public SharedItem(DocumentCommonState commonState) : base(commonState) {
        }

        public new void OpenWithProjectContext() {
            // ...
        }

        public override string ToString() {
            return $"SharedItem({base.CommonState.ToStringCore()})";
        }
    }



    //
    // ExternalInclude
    //
    public sealed class ExternalInclude : Document {
        public ExternalInclude(DocumentCommonState commonState) : base(commonState) {
        }

        public new void OpenWithProjectContext() {
            base.OpenWithProjectContext();
        }

        public override string ToString() {
            return $"ExternalInclude({base.CommonState.ToStringCore()})";
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