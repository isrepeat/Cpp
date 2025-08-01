using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;
using Helpers.Text.Ex;
using System.Windows.Controls;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectExternalDependenciesAnalyzer {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? ExternalDependenciesChanged;

        private IVsHierarchy _projectHierarchy;
        private HashSet<Hierarchy.HierarchyItemEntry> _currentExternalDependenciesItems;
        private uint _externalDependenciesItemId;

        public ProjectExternalDependenciesAnalyzer(
            IVsHierarchy projectHierarchy
            ) : this(projectHierarchy, new HashSet<Hierarchy.HierarchyItemEntry>()) {
        }

        public ProjectExternalDependenciesAnalyzer(
            IVsHierarchy projectHierarchy,
            HashSet<Hierarchy.HierarchyItemEntry> currentExternalDependenciesItems
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _currentExternalDependenciesItems = currentExternalDependenciesItems;            
            _externalDependenciesItemId = this.FindExternalDependenciesItemId();
        }


        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentExternalDependenciesItems() {
            return _currentExternalDependenciesItems.ToList();
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_externalDependenciesItemId == VSConstants.VSITEMID_NIL) {
                return;
            }

            var externalDependenciesItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                _externalDependenciesItemId,
                this.AcceptItemPredicate);

            var newExternalDependenciesItems = new HashSet<Hierarchy.HierarchyItemEntry>();

            foreach (var item in externalDependenciesItems) {
                newExternalDependenciesItems.Add(item);
            }

            // Определяем удалённые элементы
            // ExceptWith удалит из removedIncludes все элементы, которые присутствуют в newExternalDependenciesItems,
            // оставив только те, которые были в _currentExternalDependenciesItems, но больше не встречаются
            var removedItems = new HashSet<Hierarchy.HierarchyItemEntry>(_currentExternalDependenciesItems);
            removedItems.ExceptWith(newExternalDependenciesItems);

            // Определяем добавленные элементы
            // ExceptWith удалит из addedIncludes все элементы, которые уже были в _currentExternalDependenciesItems,
            // оставив только новые, появившиеся в newExternalDependenciesItems
            var addedItems = new HashSet<Hierarchy.HierarchyItemEntry>(newExternalDependenciesItems);
            addedItems.ExceptWith(_currentExternalDependenciesItems);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.ExternalDependenciesChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    addedItems.ToList(),
                    removedItems.ToList()
                    ));
            }

            // UnionWith объединяет множество: оно становится равным объединению с newExternalDependenciesItems
            // (по сути для HashSet с одинаковым Comparer это эквивалентно Clear+AddRange, но быстрее)
            _currentExternalDependenciesItems.Clear();
            _currentExternalDependenciesItems.UnionWith(newExternalDependenciesItems);
        }


        private uint FindExternalDependenciesItemId() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var childId in Utils.VsHierarchyUtils.Walker.GetChildren(_projectHierarchy, VSConstants.VSITEMID_ROOT)) {
                _projectHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                if (name.ex_IsGuidName()) {
                    _projectHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Caption, out var captionObj);
                    var caption = captionObj as string ?? "";

                    if (caption == "External Dependencies") {
                        return childId;
                    }
                }
            }

            return VSConstants.VSITEMID_NIL;
        }


        private bool AcceptItemPredicate(Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
            var hierarchyItem = hierarchyItemEntry.MultiState.As<Hierarchy.RealHierarchyItem>();
            return this.IsExternalInclude(hierarchyItem);
        }

        private bool IsExternalInclude(Hierarchy.RealHierarchyItem hierarchyItem) {
            return !string.IsNullOrEmpty(hierarchyItem.CanonicalName) &&
                (hierarchyItem.CanonicalName.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 hierarchyItem.CanonicalName.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}