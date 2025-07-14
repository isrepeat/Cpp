using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using EnvDTE;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectSharedItemsAnalyzer {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SharedItemsChanged;

        private readonly IVsHierarchy _projectHierarchy;
        private readonly HashSet<Utils.VsHierarchyUtils.HierarchyItem> _currentSharedItems = new();

        public ProjectSharedItemsAnalyzer(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();
            _projectHierarchy = projectHierarchy;
        }

        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var sharedItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                VSConstants.VSITEMID_ROOT,
                item => this.IsSharedItem(item));

            var newSharedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(sharedItems);

            var removedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(_currentSharedItems);
            removedItems.ExceptWith(newSharedItems);

            var addedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(newSharedItems);
            addedItems.ExceptWith(_currentSharedItems);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.SharedItemsChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    _projectHierarchy,
                    addedItems.ToList(),
                    removedItems.ToList()
                ));
            }

            _currentSharedItems.Clear();
            _currentSharedItems.UnionWith(newSharedItems);
        }

        private bool IsSharedItem(Utils.VsHierarchyUtils.HierarchyItem item) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy.GetProperty(
                item.ItemId,
                (int)__VSHPROPID7.VSHPROPID_IsSharedItem,
                out var isSharedItemObj);

            return isSharedItemObj is bool boolVal && boolVal;
        }
    }
}