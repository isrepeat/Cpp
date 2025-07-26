using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public abstract class HierarchyItemMultiStateElementBase :
        HelpersV4.Collections.MultiStateContainer<
            _Details.HierarchyItemCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem> {

        protected HierarchyItemMultiStateElementBase(_Details.HierarchyItemCommonState commonState)
            : base(commonState) {
        }

        protected HierarchyItemMultiStateElementBase(
            _Details.HierarchyItemCommonState commonState,
            Func<_Details.HierarchyItemCommonState, HierarchyItem> factoryA,
            Func<_Details.HierarchyItemCommonState, InvalidatedHierarchyItem> factoryB)
            : base(commonState, factoryA, factoryB) {
        }
    }


    public class HierarchyItemMultiStateElement : HierarchyItemMultiStateElementBase {
        public HierarchyItemMultiStateElement(
            Microsoft.VisualStudio.Shell.Interop.IVsHierarchy vsHierarchy,
            uint itemId
            ) : base(new _Details.HierarchyItemCommonState(vsHierarchy, itemId)) {
        }
    }



    public class HierarchyItem :
        HierarchyItemCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public IHierarchy Hierarchy => this.CommonState.Hierarchy;

        public HierarchyItem(_Details.HierarchyItemCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is InvalidatedHierarchyItem) {
                throw new InvalidOperationException("Switching state from InvalidatedHierarchyItem is not supported");
            }
        }
        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedHierarchyItem) {
                base.CommonState.Hierarchy = null;
            }
        }

        public override string ToString() {
            return $"HierarchyItem(ItemId={this.ItemId}, Name='{this.Name}', CanonicalName='{this.CanonicalName}')";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);

            switch (e.PropertyName) {
                case nameof(_Details.HierarchyItemCommonState.Hierarchy):
                    this.OnPropertyChanged(nameof(this.Hierarchy));
                    break;
            }
        }
    }



    public class InvalidatedHierarchyItem :
        HierarchyItemCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedHierarchyItem(_Details.HierarchyItemCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedHierarchyItem> {this.CommonState.ToString()}";
        }
    }
}