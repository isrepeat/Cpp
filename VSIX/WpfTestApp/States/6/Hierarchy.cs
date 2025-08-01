using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;


namespace WpfTestApp.States.MultistateBehaviour {
    public class HierarchyCommonState :
        Helpers.MultiState._Details.CommonStateBase,
        IDisposable {

        private string _vsHierarchyData;
        public string VsHierarchyData {
            get {
                return _vsHierarchyData;
            }
            set {
                if (_vsHierarchyData != value) {
                    _vsHierarchyData = value;
                    base.OnCommonStatePropertyChanged();
                }
            }
        }

        public HierarchyCommonState(string vsHierarchyData) {
            _vsHierarchyData = vsHierarchyData;
        }

        public void Dispose() {
            this.VsHierarchyData = "<diposed>";
        }
    }



    public abstract class HierarchyCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<HierarchyCommonState> {
        public string VsHierarchyData => this.CommonState.VsHierarchyData;

        protected HierarchyCommonStateViewModel(HierarchyCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(HierarchyCommonState.VsHierarchyData)) {
                base.OnPropertyChanged(nameof(this.VsHierarchyData));
            }
        }
    }



    public class HierarchyItem :
        HierarchyCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public HierarchyItem(HierarchyCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedHierarchyItem) {
            }
        }

        public override string ToString() {
            return $"<HierarchyItem>";
        }
    }


    public class InvalidatedHierarchyItem :
        HierarchyCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public InvalidatedHierarchyItem(HierarchyCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }


        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedHierarchyItem>";
        }
    }
}