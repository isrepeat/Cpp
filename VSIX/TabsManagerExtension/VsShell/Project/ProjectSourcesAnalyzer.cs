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

        private readonly HashSet<Hierarchy.HierarchyItem> _currentSources = new();

        public ProjectSourcesAnalyzer(
            IVsHierarchy projectHierarchy
            ) : this(projectHierarchy, new HashSet<Hierarchy.HierarchyItem>()) {
        }

        public ProjectSourcesAnalyzer(
            IVsHierarchy projectHierarchy,
            HashSet<Hierarchy.HierarchyItem> currentSources
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _currentSources = currentSources;
        }


        public IReadOnlyList<Hierarchy.HierarchyItem> GetCurrentSources() {
            return _currentSources.ToList();
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var sourceItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                VSConstants.VSITEMID_ROOT,
                predicate: item => !this.IsSharedItem(item) && this.IsHeaderOrCppFile(item.CanonicalName),
                shouldVisitChildren: item => !item.Name.ex_IsGuidName());

            var newSourceItems = new HashSet<Hierarchy.HierarchyItem>(sourceItems);

            var removedItems = new HashSet<Hierarchy.HierarchyItem>(_currentSources);
            removedItems.ExceptWith(newSourceItems);

            var addedItems = new HashSet<Hierarchy.HierarchyItem>(newSourceItems);
            addedItems.ExceptWith(_currentSources);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.SourcesChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    _projectHierarchy,
                    addedItems.ToList(),
                    removedItems.ToList()
                ));
            }

            _currentSources.Clear();
            _currentSources.UnionWith(newSourceItems);
        }


        private bool IsSharedItem(Hierarchy.HierarchyItem item) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy.GetProperty(
                item.ItemId,
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