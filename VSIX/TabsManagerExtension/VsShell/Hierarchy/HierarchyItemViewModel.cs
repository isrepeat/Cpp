using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public abstract class HierarchyItemEntryBase :
        Helpers.MultiState.MultiStateEntryBase<_Details.HierarchyItemCommonState> {
        public bool IsRealHierarchy => this.MultiStateBase.Current is RealHierarchyItem;
        public bool IsStubHierarchy => this.MultiStateBase.Current is StubHierarchyItem;
        public HierarchyItemCommonStateViewModel BaseViewModel => this.MultiStateBase.AsViewModel<HierarchyItemCommonStateViewModel>();

        public HierarchyItemEntryBase(HierarchyItemMultiStateElementBase multiStateBase) : base(multiStateBase) {
        }

        protected override void OnMultiStateChanged() {
            base.OnPropertyChanged(nameof(this.IsRealHierarchy));
            base.OnPropertyChanged(nameof(this.IsStubHierarchy));
        }
    }


    public partial class HierarchyItemEntry : HierarchyItemEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private HierarchyItemMultiStateElement _multiState;

        private HierarchyItemEntry(HierarchyItemMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
        }

        public static HierarchyItemEntry CreateWithState<TState>(HierarchyItemMultiStateElement multiState)
            where TState : Helpers.MultiState.IMultiStateElement {

            var entry = new HierarchyItemEntry(multiState);
            entry.MultiState.SwitchTo<TState>();

            return entry;
        }
    }
}