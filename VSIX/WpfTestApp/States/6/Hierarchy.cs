using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using Helpers.Attributes;


namespace WpfTestApp.States.MultistateBehaviour {
    public class HierarchyNode :
        IDisposable {

        private readonly Helpers.Collections.MultiStateContainer<HierarchyItem, InvalidatedHierarchyItem, HierarchyItemSharedState> _hierarchyNodeState;
        public Helpers.Collections.MultiStateContainer<HierarchyItem, InvalidatedHierarchyItem, HierarchyItemSharedState> HierarchyNodeState => _hierarchyNodeState;

        public HierarchyNode() {
            _hierarchyNodeState = new Helpers.Collections.MultiStateContainer<HierarchyItem, InvalidatedHierarchyItem, HierarchyItemSharedState>(
                new HierarchyItem(),
                new InvalidatedHierarchyItem(),
                new HierarchyItemSharedState()
                );
        }

        public void Dispose() {
            _hierarchyNodeState.Dispose();
        }
    }


    public class HierarchyItemSharedState {
    }

    public class HierarchyItem :
        IDisposable,
        Helpers.Collections.IMultiStateElement<HierarchyItemSharedState> {

        public uint ItemId { get; private set; }

        public HierarchyItem() {
            ItemId = 11;
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<HierarchyItemSharedState> e) {
            if (e.PreviousState is Helpers.Collections.UnknownMultiStateElement<HierarchyItemSharedState>) {
            }
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<HierarchyItemSharedState> e) {
            if (e.NextState is InvalidatedHierarchyItem) {
            }
        }
    }


    public class InvalidatedHierarchyItem :
        IDisposable,
        Helpers.Collections.IMultiStateElement<HierarchyItemSharedState> {

        public uint LastItemId { get; private set; }

        public InvalidatedHierarchyItem() {
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<HierarchyItemSharedState> e) {
            if (e.PreviousState is Helpers.Collections.UnknownMultiStateElement<HierarchyItemSharedState>) {
            }
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<HierarchyItemSharedState> e) {
        }
    }
}