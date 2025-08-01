using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Helpers.Text.Ex;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectSourcesAnalyzer {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SourcesChanged;

        private readonly IVsHierarchy _projectHierarchy;

        private readonly HashSet<Hierarchy.HierarchyItemEntry> _currentSources = new();

        public ProjectSourcesAnalyzer(
            IVsHierarchy projectHierarchy
            ) : this(projectHierarchy, new HashSet<Hierarchy.HierarchyItemEntry>()) {
        }

        public ProjectSourcesAnalyzer(
            IVsHierarchy projectHierarchy,
            HashSet<Hierarchy.HierarchyItemEntry> currentSources
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _currentSources = currentSources;
        }


        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentSources() {
            return _currentSources.ToList();
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var sourceItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                VSConstants.VSITEMID_ROOT,
                this.AcceptItemPredicate,
                this.ShouldVisitChildrenPredicate);

            var newSourceItems = new HashSet<Hierarchy.HierarchyItemEntry>(sourceItems);

            var removedItems = new HashSet<Hierarchy.HierarchyItemEntry>(_currentSources);
            removedItems.ExceptWith(newSourceItems);

            var addedItems = new HashSet<Hierarchy.HierarchyItemEntry>(newSourceItems);
            addedItems.ExceptWith(_currentSources);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.SourcesChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    addedItems.ToList(),
                    removedItems.ToList()
                ));
            }

            _currentSources.Clear();
            _currentSources.UnionWith(newSourceItems);
        }



        private bool AcceptItemPredicate(Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
            var hierarchyItem = hierarchyItemEntry.MultiState.As<Hierarchy.RealHierarchyItem>();
            return !this.IsSharedItem(hierarchyItem) && this.IsHeaderOrCppFile(hierarchyItem.CanonicalName);
        }

        private bool ShouldVisitChildrenPredicate(Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
            var hierarchyItem = hierarchyItemEntry.MultiState.As<Hierarchy.RealHierarchyItem>();
            return !hierarchyItem.Name.ex_IsGuidName();
        }

        private bool IsSharedItem(Hierarchy.RealHierarchyItem hierarchyItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy.GetProperty(
                hierarchyItem.ItemId,
                (int)__VSHPROPID7.VSHPROPID_IsSharedItem,
                out var isSharedItemObj);

            return isSharedItemObj is bool boolVal && boolVal;
        }

        private bool IsHeaderOrCppFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}