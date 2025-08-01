using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectSharedItemsAnalyzer {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SharedItemsChanged;

        private readonly IVsHierarchy _projectHierarchy;
        private readonly HashSet<Hierarchy.HierarchyItemEntry> _currentSharedItems = new();

        public ProjectSharedItemsAnalyzer(
            IVsHierarchy projectHierarchy
            ) : this(projectHierarchy, new HashSet<Hierarchy.HierarchyItemEntry>()) {
        }

        public ProjectSharedItemsAnalyzer(
            IVsHierarchy projectHierarchy,
            HashSet<Hierarchy.HierarchyItemEntry> currentSharedItems
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _currentSharedItems = currentSharedItems;
        }


        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentSharedItems() {
            return _currentSharedItems.ToList();
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var sharedItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                VSConstants.VSITEMID_ROOT,
                this.AcceptItemPredicate);

            var newSharedItems = new HashSet<Hierarchy.HierarchyItemEntry>(sharedItems);

            var removedItems = new HashSet<Hierarchy.HierarchyItemEntry>(_currentSharedItems);
            removedItems.ExceptWith(newSharedItems);

            var addedItems = new HashSet<Hierarchy.HierarchyItemEntry>(newSharedItems);
            addedItems.ExceptWith(_currentSharedItems);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.SharedItemsChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    addedItems.ToList(),
                    removedItems.ToList()
                ));
            }

            _currentSharedItems.Clear();
            _currentSharedItems.UnionWith(newSharedItems);
        }


        private bool AcceptItemPredicate(Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
            var hierarchyItem = hierarchyItemEntry.MultiState.As<Hierarchy.RealHierarchyItem>();
            return this.IsSharedItem(hierarchyItem);
        }

        private bool IsSharedItem(Hierarchy.RealHierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();
            
            _projectHierarchy.GetProperty(
                hierarchyItem.ItemId,
                (int)__VSHPROPID7.VSHPROPID_IsSharedItem,
                out var isSharedItemObj);

            return isSharedItemObj is bool boolVal && boolVal;
        }
    }
}