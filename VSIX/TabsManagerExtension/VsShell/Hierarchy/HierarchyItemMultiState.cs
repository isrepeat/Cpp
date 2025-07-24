using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public class HierarchyItemMultiStateContainer :
        HelpersV4.Collections.MultiStateContainer<
            HierarchyItemCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem> {
        public HierarchyItemMultiStateContainer(HierarchyItemCommonState state)
            : base(state) {
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
            return this.CommonState.ToString();
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnSharedStatePropertyChanged(sender, e);

            if (e.PropertyName is nameof(HierarchyItemCommonState.Hierarchy)) {
                base.OnPropertyChanged(nameof(this.Hierarchy));
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