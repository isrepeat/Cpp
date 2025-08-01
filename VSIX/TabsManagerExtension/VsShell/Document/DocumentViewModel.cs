using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Document {
    public abstract class DocumentEntryBase :
        Helpers.MultiState.MultiStateEntryBase<_Details.DocumentCommonState> {
        public bool IsInvalidated => base.MultiStateBase.Current is InvalidatedDocument;
        public DocumentCommonStateViewModel BaseViewModel => base.MultiStateBase.AsViewModel<DocumentCommonStateViewModel>();

        public DocumentEntryBase(DocumentMultiStateElementBase multiStateBase) : base(multiStateBase) {
        }

        protected override void OnMultiStateChanged() {
            base.OnPropertyChanged(nameof(this.IsInvalidated));
        }
    }


    public partial class DocumentEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private DocumentMultiStateElement _multiState;

        private DocumentEntry(DocumentMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
        }

        public static DocumentEntry CreateWithState<TState>(DocumentMultiStateElement multiState)
            where TState : Helpers.MultiState.IMultiStateElement {

            var entry = new DocumentEntry(multiState);
            entry.MultiState.SwitchTo<TState>();

            return entry;
        }
    }


    public partial class SharedItemEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private SharedItemMultiStateElement _multiState;

        private SharedItemEntry(SharedItemMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
        }

        public static SharedItemEntry CreateWithState<TState>(SharedItemMultiStateElement multiState)
            where TState : Helpers.MultiState.IMultiStateElement {

            var entry = new SharedItemEntry(multiState);
            entry.MultiState.SwitchTo<TState>();

            return entry;
        }
    }


    public partial class ExternalIncludeEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private ExternalIncludeMultiStateElement _multiState;

        private ExternalIncludeEntry(ExternalIncludeMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
        }

        public static ExternalIncludeEntry CreateWithState<TState>(ExternalIncludeMultiStateElement multiState)
            where TState : Helpers.MultiState.IMultiStateElement {

            var entry = new ExternalIncludeEntry(multiState);
            entry.MultiState.SwitchTo<TState>();

            return entry;
        }
    }
}