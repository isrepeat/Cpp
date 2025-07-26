using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Project {
    namespace _Details {
        public partial class ProjectCommonState :
            HelpersV4.Collections.CommonStateBase,
            IDisposable {

            public ProjectNode ProjectNode { get; }
            public Helpers.Collections.DisposableList<Document.ExternalIncludeEntry> ExternalIncludes { get; } = new();
            public Helpers.Collections.DisposableList<Document.SharedItemEntry> SharedItems { get; } = new();
            public Helpers.Collections.DisposableList<Document.DocumentEntry> Sources { get; } = new();

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private bool _isDisposed;

            public ProjectCommonState(
                ProjectNode projectNode
                ) {
                this.ProjectNode = projectNode;
            }

            public void Dispose() {
                if (this.IsDisposed) {
                    return;
                }

                this.ExternalIncludes.ClearAndDispose();
                this.SharedItems.ClearAndDispose();
                this.Sources.ClearAndDispose();

                this.IsDisposed = true;
            }
        }
    } // namesace _Details



    public abstract class ProjectCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<_Details.ProjectCommonState> {
        public ProjectNode ProjectNode => base.CommonState.ProjectNode;
        public Helpers.Collections.DisposableList<Document.ExternalIncludeEntry> ExternalIncludes => base.CommonState.ExternalIncludes;
        public Helpers.Collections.DisposableList<Document.SharedItemEntry> SharedItems => base.CommonState.SharedItems;
        public Helpers.Collections.DisposableList<Document.DocumentEntry> Sources => base.CommonState.Sources;
        public bool IsDisposed => this.CommonState.IsDisposed;

        protected ProjectCommonStateViewModel(_Details.ProjectCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(_Details.ProjectCommonState.IsDisposed):
                    base.OnPropertyChanged(nameof(this.IsDisposed));
                    break;
            }
        }
    }


    // TODO: Add to CodeAnalyzer support generate generic classes.
    namespace _Details {
        public abstract class ProjectEntryBaseViewModel :
            Helpers.ObservableObject,
            IDisposable {
            public ProjectCommonStateViewModel BaseViewModel => this.MultiStateBase.AsViewModel<ProjectCommonStateViewModel>();
            protected ProjectMultiStateElementBase MultiStateBase { get; }

            public ProjectEntryBaseViewModel(ProjectMultiStateElementBase multiStateBase) {
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



    public abstract class ProjectEntryBase : _Details.ProjectEntryBaseViewModel {
        public bool IsLoaded => this.MultiStateBase.Current is LoadedProject;
        public bool IsUnloaded => this.MultiStateBase.Current is UnloadedProject;
        public ProjectEntryBase(ProjectMultiStateElementBase multiStateBase) : base(multiStateBase) {
        }
        protected override void OnMultiStateChanged() {
            base.OnPropertyChanged(nameof(this.IsLoaded));
            base.OnPropertyChanged(nameof(this.IsUnloaded));
        }
    }


    public partial class ProjectEntry : ProjectEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private ProjectMultiStateElement _multiState;

        public ProjectEntry(ProjectMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
        }
    }
}