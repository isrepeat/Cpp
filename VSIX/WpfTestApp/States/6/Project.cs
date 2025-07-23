using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Helpers.Attributes;


namespace WpfTestApp.States.MultistateBehaviour {
    public partial class ProjectCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private string _name = "";

        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private string _projId = "";

        [ObservableMultiStateProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument
            > _documentMultiState;

        public ProjectCommonState(string vsHierarchyData) {
            _documentMultiState = new(new DocumentCommonState(vsHierarchyData));
            this.DocumentMultiState = _documentMultiState;
        }

        public void Dispose() {
            this.DocumentMultiState?.Dispose();
        }
    }



    public abstract class ProjectCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<ProjectCommonState> {

        public string Name => this.CommonState.Name;
        public object DocumentObj => this.CommonState.DocumentMultiState.Current;
        public string DocumentObjHash => this.CommonState.DocumentMultiState.CurrentHash;
        public IEnumerable<string> DocumentObjHashes => this.CommonState.DocumentMultiState.InstancesHashSet.ToList();

        protected ProjectCommonStateViewModel(ProjectCommonState commonState)
            : base(commonState) {
            this.UpdateHashes();
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(ProjectCommonState.Name)) {
                base.OnPropertyChanged(nameof(this.Name));
            }
            else if (e.PropertyName is nameof(ProjectCommonState.DocumentMultiState)) {
                base.OnPropertyChanged(nameof(this.DocumentObj));
                this.UpdateHashes();
            }
        }

        private void UpdateHashes() {
            base.OnPropertyChanged(nameof(this.DocumentObjHash));
            base.OnPropertyChanged(nameof(this.DocumentObjHashes));
        }
    }



    public class LoadedProject :
        ProjectCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public string ProjId => this.CommonState.ProjId;

        private int counter = 0;
        public LoadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.Name = "HelloWorld";
            base.CommonState.ProjId = "1234";

            _ = Task.Run(async () => {
                await Task.Delay(1000);
                base.CommonState.DocumentMultiState = new(new DocumentCommonState($"vsHierarchy_{++this.counter}"));
                base.CommonState.DocumentMultiState.SwitchTo<Document>();
            });
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is UnloadedProject) {
                base.CommonState.Name = "<unloaded>";
                base.CommonState.ProjId = "<unloaded>";
                base.CommonState.DocumentMultiState.SwitchTo<InvalidatedDocument>();
            }
        }

        public override string ToString() {
            return $"<LoadedProject>";
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnSharedStatePropertyChanged(sender, e);

            if (e.PropertyName is nameof(ProjectCommonState.ProjId)) {
                base.OnPropertyChanged(nameof(this.ProjId));
            }
        }
    }



    public class UnloadedProject :
        ProjectCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public UnloadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<UnloadedProject>";
        }
    }
}