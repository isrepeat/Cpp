using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Document {
    namespace _Details {
        public partial class DocumentCommonState :
            HelpersV4.Collections.CommonStateBase,
            IDisposable {

            public VsShell.Project.ProjectNode ProjectNode { get; }
            public Hierarchy.HierarchyItemEntry HierarchyItemEntry { get; }

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private bool _isDisposed;

            public DocumentCommonState(
                VsShell.Project.ProjectNode projectNode,
                Hierarchy.HierarchyItemEntry hierarchyItemEntry
                ) {
                this.ProjectNode = projectNode;
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
                var projectUniqueName = this.ProjectNode.UniqueName;
                var filePath = this.HierarchyItemEntry.BaseViewModel.FilePath;
                var itemId = this.HierarchyItemEntry.BaseViewModel.ItemId;

                return $"FilePath='{filePath}', Project='{projectUniqueName}', ItemId={itemId}";
            }
        }
    } // namesace _Details



    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<_Details.DocumentCommonState> {
        public VsShell.Project.ProjectNode ProjectNode => this.CommonState.ProjectNode;
        public Hierarchy.HierarchyItemEntry HierarchyItemEntry => this.CommonState.HierarchyItemEntry;
        public bool IsDisposed => this.CommonState.IsDisposed;

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



    // TODO: Add to CodeAnalyzer support generate generic classes.
    namespace _Details {
        public abstract class DocumentEntryBaseViewModel :
            Helpers.ObservableObject,
            IDisposable {
            public DocumentCommonStateViewModel BaseViewModel => this.MultiStateBase.AsViewModel<DocumentCommonStateViewModel>();
            protected DocumentMultiStateElementBase MultiStateBase { get; }

            public DocumentEntryBaseViewModel(DocumentMultiStateElementBase multiStateBase) {
                this.MultiStateBase = multiStateBase;
                this.MultiStateBase.StateChanged += this.OnMultiStateChanged;
            }
            public void Dispose() {
                this.MultiStateBase.StateChanged -= this.OnMultiStateChanged;
                this.MultiStateBase.Dispose();
            }

            public override string ToString() {
                return this.MultiStateBase.ToString();
            }

            protected abstract void OnMultiStateChanged();
        }
    } // namesace _Details



    public abstract class DocumentEntryBase : _Details.DocumentEntryBaseViewModel {
        public bool IsInvalidated => this.MultiStateBase.Current is InvalidatedDocument;
        public DocumentEntryBase(DocumentMultiStateElementBase multiStateBase) : base(multiStateBase) {
        }
        protected override void OnMultiStateChanged() {
            base.OnPropertyChanged(nameof(this.IsInvalidated));
        }
    }


    public partial class DocumentEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private DocumentMultiStateElement _multiState;

        public DocumentEntry(DocumentMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
            this.MultiState.SwitchTo<Document>();
        }
    }


    public partial class SharedItemEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private SharedItemMultiStateElement _multiState;

        public SharedItemEntry(SharedItemMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
            this.MultiState.SwitchTo<Document>();
        }
    }


    public partial class ExternalIncludeEntry : DocumentEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private ExternalIncludeMultiStateElement _multiState;

        public ExternalIncludeEntry(ExternalIncludeMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
            this.MultiState.SwitchTo<Document>();
        }
    }
}