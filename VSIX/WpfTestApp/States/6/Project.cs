using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;


namespace WpfTestApp.States.MultistateBehaviour {
    public class ProjectCommonState :
        HelpersV3.Collections.CommonStateBase,
        IDisposable {

        private string _name = "";
        public string Name {
            get {
                return _name;
            }
            set {
                if (_name != value) {
                    _name = value;
                    base.OnSharedStatePropertyChanged();
                }
            }
        }


        private string _projId = "";
        public string ProjId {
            get {
                return _projId;
            }
            set {
                if (_projId != value) {
                    _projId = value;
                    base.OnSharedStatePropertyChanged();
                }
            }
        }

        public HelpersV3.Collections.MultiStateContainer<DocumentCommonState, Document, InvalidatedDocument> DocumentMultiState { get; }

        public ProjectCommonState() {
            this.DocumentMultiState = new();
            this.DocumentMultiState.StateChanged += () => base.OnSharedStatePropertyChanged(nameof(this.DocumentMultiState));

        }

        public void Dispose() {
            if (this.DocumentMultiState is IDisposable documentMultiStateDisposable) {
                documentMultiStateDisposable.Dispose();
            }
        }
    }



    public abstract class ProjectCommonStateViewModel :
        HelpersV3.Collections.CommonStateViewModelBase<ProjectCommonState> {

        public string Name => this.CommonState.Name;
        public object DocumentObj => this.CommonState.DocumentMultiState.Current;

        protected ProjectCommonStateViewModel(ProjectCommonState commonState)
            : base(commonState) {
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
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
        HelpersV3.Collections.IMultiStateElement {

        public string ProjId => this.CommonState.ProjId;

        public LoadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.Name = "HelloWorld";
            base.CommonState.ProjId = "1234";

            _ = Task.Run(async () => {
                await Task.Delay(1000);
                base.CommonState.DocumentMultiState.SwitchTo<Document>();
            });
        }

        public void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e) {
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
        HelpersV3.Collections.IMultiStateElement {

        public UnloadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<UnloadedProject>";
        }
    }
}