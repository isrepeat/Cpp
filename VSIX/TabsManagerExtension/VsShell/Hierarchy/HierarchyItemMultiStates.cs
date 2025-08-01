using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public abstract class HierarchyItemMultiStateElementBase :
        Helpers.MultiState.MultiStateContainer<
            _Details.HierarchyItemCommonState,
            RealHierarchyItem,
            StubHierarchyItem,
            InvalidatedHierarchyItem> {

        protected HierarchyItemMultiStateElementBase(_Details.HierarchyItemCommonState commonState)
            : base(commonState) {
        }

        protected HierarchyItemMultiStateElementBase(
            _Details.HierarchyItemCommonState commonState,
            Func<_Details.HierarchyItemCommonState, RealHierarchyItem> factoryA,
            Func<_Details.HierarchyItemCommonState, StubHierarchyItem> factoryB,
            Func<_Details.HierarchyItemCommonState, InvalidatedHierarchyItem> factoryC
            ) : base(commonState, factoryA, factoryB, factoryC) {
        }
    }


    public class HierarchyItemMultiStateElement : HierarchyItemMultiStateElementBase {
        public HierarchyItemMultiStateElement(
            IVsHierarchy vsHierarchy,
            uint itemId
            ) : base(new _Details.HierarchyItemCommonState(vsHierarchy, itemId)) {
        }
    }



    public class RealHierarchyItem :
        HierarchyItemCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {
        public IVsHierarchy VsRealHierarchy => base.CommonState.VsHierarchy;

        public RealHierarchyItem(_Details.HierarchyItemCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is Helpers.MultiState.UnknownMultiStateElement) {
                Helpers.ThrowableAssert.Require(Utils.VsHierarchyUtils.IsRealHierarchy(base.CommonState.VsHierarchy));
            }

            Helpers.ThrowableAssert.Require(
                e.PreviousState is not StubHierarchyItem &&
                e.PreviousState is not InvalidatedHierarchyItem
                );
        }
        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<RealHierarchyItem> ({base.CommonState.ToStringCore()})";
        }
    }



    public class StubHierarchyItem :
        HierarchyItemCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {
        public IVsHierarchy VsStubHierarchy => base.CommonState.VsHierarchy;

        public StubHierarchyItem(_Details.HierarchyItemCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is Helpers.MultiState.UnknownMultiStateElement) {
                Helpers.ThrowableAssert.Require(Utils.VsHierarchyUtils.IsStubHierarchy(base.CommonState.VsHierarchy));
            }

            Helpers.ThrowableAssert.Require(
                e.PreviousState is not RealHierarchyItem &&
                e.PreviousState is not InvalidatedHierarchyItem
                );
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<StubHierarchyItem> ({base.CommonState.ToStringCore()})";
        }
    }



    public class InvalidatedHierarchyItem :
        HierarchyItemCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public InvalidatedHierarchyItem(_Details.HierarchyItemCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedHierarchyItem> ({base.CommonState.ToStringCore()})";
        }
    }
}