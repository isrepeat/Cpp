using System;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;
using System.Windows.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using Helpers.Attributes;


namespace WpfTestApp.States.MultistateBehaviour {
    public partial class ProjectCommonState :
        Helpers.MultiState._Details.CommonStateBase,
        IDisposable {

        [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private string _name = "";

        [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private string _projId = "";

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private DocumentMultiStateElement _documentMultiState;


        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private ExteralIncludeMultiStateElement _externlIncludeMultiState;

        public ProjectCommonState(string vsHierarchyData) {
            this.DocumentMultiState = new DocumentMultiStateElement(vsHierarchyData);
            this.ExternlIncludeMultiState = new ExteralIncludeMultiStateElement(vsHierarchyData);
        }

        public void Dispose() {
            this.DocumentMultiState?.Dispose();
        }
    }



    public abstract class ProjectCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<ProjectCommonState> {

        public string Name => this.CommonState.Name;
        public object DocumentObj => this.CommonState.DocumentMultiState.Current;

        protected ProjectCommonStateViewModel(ProjectCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(ProjectCommonState.Name)) {
                base.OnPropertyChanged(nameof(this.Name));
            }
            else if (e.PropertyName is nameof(ProjectCommonState.DocumentMultiState)) {
                base.OnPropertyChanged(nameof(this.DocumentObj));
            }
        }
    }



    public class LoadedProject :
        ProjectCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public string ProjId => this.CommonState.ProjId;

        private int counter = 0;
        public LoadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.Name = "HelloWorld";
            base.CommonState.ProjId = "1234";

            _ = Task.Run(async () => {
                await Task.Delay(1000);
                //base.CommonState.DocumentMultiState = new DocumentMultiStateElement($"vsHierarchy_{++this.counter}");
                await Task.Delay(500);
                base.CommonState.DocumentMultiState.SwitchTo<Document>();
            });
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is UnloadedProject) {
                base.CommonState.Name = "<unloaded>";
                base.CommonState.ProjId = "<unloaded>";
                base.CommonState.DocumentMultiState.SwitchTo<InvalidatedDocument>();
            }
        }

        public override string ToString() {
            return $"<LoadedProject>";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);

            if (e.PropertyName is nameof(ProjectCommonState.ProjId)) {
                base.OnPropertyChanged(nameof(this.ProjId));
            }
        }
    }



    public class UnloadedProject :
        ProjectCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public UnloadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<UnloadedProject>";
        }
    }
}