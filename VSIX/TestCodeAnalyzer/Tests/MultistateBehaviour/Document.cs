using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using Helpers.Attributes;


namespace TestCodeAnalyzer.Tests.MultistateBehaviour {

    public class DocumentNode :
        IDisposable {

        private readonly Helpers.Collections.MultiStateContainer<Document, InvalidatedDocument, DocumentSharedState> _documentNodeState;
        public Helpers.Collections.MultiStateContainer<Document, InvalidatedDocument, DocumentSharedState> DocumentNodeState => _documentNodeState;

        public DocumentNode() {
            _documentNodeState = new Helpers.Collections.MultiStateContainer<Document, InvalidatedDocument, DocumentSharedState>(
                new Document(),
                new InvalidatedDocument(),
                new DocumentSharedState()
                );
        }

        public void Dispose() {
            _documentNodeState.Dispose();
        }
    }


    public class DocumentSharedState {
        public HierarchyNode HierarchyNode { get; } = new();
    }


    public class Document :
        IDisposable,
        Helpers.Collections.IMultiStateElement<DocumentSharedState> {
        public HierarchyItem? HierarchyItem { get; private set; } = null;

        public Document(HierarchyItem hierarchyItem) {
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<DocumentSharedState> e) {
            if (e.PreviousState is Helpers.Collections.UnknownMultiStateElement<DocumentSharedState>) {
                e.SharedState.HierarchyNode.HierarchyNodeState.SwitchTo<HierarchyItem>();
            }
            this.HierarchyItem = e.SharedState.HierarchyNode.HierarchyNodeState.Get<HierarchyItem>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<DocumentSharedState> e) {
            if (e.NextState is InvalidatedDocument) {
                e.SharedState.HierarchyNode.HierarchyNodeState.SwitchTo<InvalidatedHierarchyItem>();
            }
        }
    }


    public class InvalidatedDocument :
        IDisposable,
        Helpers.Collections.IMultiStateElement<DocumentSharedState> {
        public InvalidatedHierarchyItem? LastHierarchyItem { get; private set; } = null;

        public InvalidatedDocument() {
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<DocumentSharedState> e) {
            if (e.PreviousState is Helpers.Collections.UnknownMultiStateElement<DocumentSharedState>) {
                e.SharedState.HierarchyNode.HierarchyNodeState.SwitchTo<InvalidatedHierarchyItem>();
            }
            this.LastHierarchyItem = e.SharedState.HierarchyNode.HierarchyNodeState.Get<InvalidatedHierarchyItem>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<DocumentSharedState> e) {
        }
    }
}