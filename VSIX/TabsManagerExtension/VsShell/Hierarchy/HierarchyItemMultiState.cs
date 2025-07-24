using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public class HierarchyItemMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            HierarchyItemCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem> {
        public HierarchyItemMultiStateElement(
            Microsoft.VisualStudio.Shell.Interop.IVsHierarchy vsHierarchy,
            uint itemId
            ) : base(new HierarchyItemCommonState(vsHierarchy, itemId)) {
        }
    }


    public class HierarchyItem :
        HierarchyItemCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public IHierarchy Hierarchy => this.CommonState.Hierarchy;

        public HierarchyItem(HierarchyItemCommonState commonState) : base(commonState) {
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
                case nameof(HierarchyItemCommonState.Hierarchy):
                    this.OnPropertyChanged(nameof(this.Hierarchy));
                    break;
            }
        }
    }



    public class InvalidatedHierarchyItem :
        HierarchyItemCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedHierarchyItem(HierarchyItemCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
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