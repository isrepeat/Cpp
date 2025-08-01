using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Project {
    public abstract class ProjectEntryBase :
        Helpers.MultiState.MultiStateEntryBase<_Details.ProjectCommonState> {
        public bool IsLoaded => base.MultiStateBase.Current is LoadedProject;
        public bool IsUnloaded => base.MultiStateBase.Current is UnloadedProject;
        public ProjectCommonStateViewModel BaseViewModel => base.MultiStateBase.AsViewModel<ProjectCommonStateViewModel>();
        
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