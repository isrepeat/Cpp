using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using TabsManagerExtension.VsShell.Document;


namespace TabsManagerExtension.VsShell.Project {
    public partial class ProjectCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        public ProjectNode ProjectNode { get; }

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private Helpers.Collections.DisposableList<ExteralIncludeMultiStateElement> _externalIncludes = new();
        
        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private Helpers.Collections.DisposableList<SharedItemMultiStateElement> _sharedItems = new();

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private Helpers.Collections.DisposableList<DocumentMultiStateElement> _sources = new();

        public ProjectCommonState(
            ProjectNode projectNode
            ) {
            this.ProjectNode = projectNode;
        }

        public void Dispose() {
            this.ExternalIncludes.ClearAndDispose();
            this.SharedItems.ClearAndDispose();
            this.Sources.ClearAndDispose();
        }
    }



    public abstract class ProjectCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<ProjectCommonState> {
        public ProjectNode ProjectNode => base.CommonState.ProjectNode;
        

        public Helpers.Collections.DisposableList<
            ExteralIncludeMultiStateElement
            > ExternalIncludes => base.CommonState.ExternalIncludes;


        public Helpers.Collections.DisposableList<
            SharedItemMultiStateElement
            > SharedItems => base.CommonState.SharedItems;


        public Helpers.Collections.DisposableList<
            DocumentMultiStateElement
            > Sources => base.CommonState.Sources;


        protected ProjectCommonStateViewModel(ProjectCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(ProjectCommonState.ProjectNode):
                    base.OnPropertyChanged(nameof(this.ProjectNode));
                    break;

                case nameof(ProjectCommonState.ExternalIncludes):
                    base.OnPropertyChanged(nameof(this.ExternalIncludes));
                    break;

                case nameof(ProjectCommonState.SharedItems):
                    base.OnPropertyChanged(nameof(this.SharedItems));
                    break;

                case nameof(ProjectCommonState.Sources):
                    base.OnPropertyChanged(nameof(this.Sources));
                    break;
            }
        }
    }
}