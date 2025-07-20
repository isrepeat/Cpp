using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;


namespace WpfTestApp.States.MultistateBehaviour {
    public class HierarchyCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        private string _vsHierarchyData;
        public string VsHierarchyData {
            get {
                return _vsHierarchyData;
            }
            set {
                if (_vsHierarchyData != value) {
                    _vsHierarchyData = value;
                    base.OnSharedStatePropertyChanged();
                }
            }
        }

        public HierarchyCommonState(string vsHierarchyData) {
            _vsHierarchyData = vsHierarchyData;
        }

        public void Dispose() {
        }
    }



    public abstract class HierarchyCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<HierarchyCommonState> {
        public string VsHierarchyData => this.CommonState.VsHierarchyData;

        protected HierarchyCommonStateViewModel(HierarchyCommonState commonState)
            : base(commonState) {
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(HierarchyCommonState.VsHierarchyData)) {
                base.OnPropertyChanged(nameof(this.VsHierarchyData));
            }
        }
    }



    public class HierarchyItem :
        HierarchyCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public HierarchyItem(HierarchyCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedHierarchyItem) {
            }
        }

        public override string ToString() {
            return $"<HierarchyItem>";
        }
    }


    public class InvalidatedHierarchyItem :
        HierarchyCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedHierarchyItem(HierarchyCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }


        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedHierarchyItem>";
        }
    }
}