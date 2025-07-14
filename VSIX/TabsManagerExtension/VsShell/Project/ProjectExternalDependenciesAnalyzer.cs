using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;
using Helpers.Text.Ex;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectExternalDependenciesAnalyzer {
        public event Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? ExternalDependenciesChanged;

        private IVsHierarchy _projectHierarchy;
        private uint _externalDependenciesItemId;

        private readonly HashSet<Utils.VsHierarchyUtils.HierarchyItem> _currentExternalDependenciesItems = new();

        public ProjectExternalDependenciesAnalyzer(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _externalDependenciesItemId = this.FindExternalDependenciesItemId();
        }


        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_externalDependenciesItemId == VSConstants.VSITEMID_NIL) {
                return;
            }

            var externalDependenciesItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                _projectHierarchy,
                _externalDependenciesItemId,
                item => this.IsExternalIncludeFile(item.CanonicalName));

            var newExternalDependenciesItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>();

            foreach (var item in externalDependenciesItems) {
                newExternalDependenciesItems.Add(item);
            }

            // Определяем удалённые элементы
            // ExceptWith удалит из removedIncludes все элементы, которые присутствуют в newExternalDependenciesItems,
            // оставив только те, которые были в _currentExternalDependenciesItems, но больше не встречаются
            var removedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(_currentExternalDependenciesItems);
            removedItems.ExceptWith(newExternalDependenciesItems);

            // Определяем добавленные элементы
            // ExceptWith удалит из addedIncludes все элементы, которые уже были в _currentExternalDependenciesItems,
            // оставив только новые, появившиеся в newExternalDependenciesItems
            var addedItems = new HashSet<Utils.VsHierarchyUtils.HierarchyItem>(newExternalDependenciesItems);
            addedItems.ExceptWith(_currentExternalDependenciesItems);

            if (addedItems.Count > 0 || removedItems.Count > 0) {
                this.ExternalDependenciesChanged?.Invoke(new _EventArgs.ProjectHierarchyItemsChangedEventArgs(
                    _projectHierarchy,
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


        private bool IsExternalIncludeFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}