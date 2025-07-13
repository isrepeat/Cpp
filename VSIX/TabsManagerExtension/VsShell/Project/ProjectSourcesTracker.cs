using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectSourcesTracker {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SourcesChanged;

        private readonly IVsHierarchy _projectHierarchy;

        private readonly HashSet<Utils.VsHierarchyUtils.HierarchyItem> _currentSources = new();

        public ProjectSourcesTracker(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();
            _projectHierarchy = projectHierarchy;
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var sourceItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                VSConstants.VSITEMID_ROOT,
                item => this.IsHeaderOrCppFile(item.CanonicalName));

            var newSourceItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(sourceItems);

            var removedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(_currentSources);
            removedItems.ExceptWith(newSourceItems);

            var addedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(newSourceItems);
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


        private bool IsHeaderOrCppFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}