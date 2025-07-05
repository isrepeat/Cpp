using System;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;


namespace TabsManagerExtension.VsShell.Utils {
    public static class VsHierarchy {
        public sealed class HierarchyItem {
            public IVsHierarchy Hierarchy { get; }
            public uint ItemId { get; }
            public string? Name { get; }
            public string? CanonicalName { get; }

            public HierarchyItem(IVsHierarchy hierarchy, uint itemId, string? name, string? canonicalName) {
                this.Hierarchy = hierarchy;
                this.ItemId = itemId;
                this.Name = name;
                this.CanonicalName = canonicalName;
            }

            public override string ToString() {
                return $"HierarchyItem(ItemId={this.ItemId}, Name='{this.Name}', CanonicalName='{this.CanonicalName}')";
            }
        }


        /// <summary>
        /// Walker
        /// </summary>
        public static class Walker {
            public static List<uint> GetChildren(IVsHierarchy hierarchy, uint parentId) {
                var result = new List<uint>();

                if (Walker.TryGetFirstChild(hierarchy, parentId, out var childId)) {
                    do {
                        result.Add(childId);
                    }
                    while (Walker.TryGetNextSibling(hierarchy, childId, out childId));
                }

                return result;
            }

            public static bool TryGetFirstChild(IVsHierarchy hierarchy, uint itemId, out uint childId) {
                childId = VSConstants.VSITEMID_NIL;

                if (hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_FirstChild, out var childObj) != VSConstants.S_OK ||
                    !(childObj is int rawChild) ||
                    unchecked((uint)rawChild) == VSConstants.VSITEMID_NIL) {
                    return false;
                }

                childId = unchecked((uint)rawChild);
                return true;
            }

            public static bool TryGetNextSibling(IVsHierarchy hierarchy, uint itemId, out uint siblingId) {
                siblingId = VSConstants.VSITEMID_NIL;

                if (hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_NextSibling, out var nextObj) != VSConstants.S_OK ||
                    !(nextObj is int rawNext) ||
                    unchecked((uint)rawNext) == VSConstants.VSITEMID_NIL) {
                    return false;
                }

                siblingId = unchecked((uint)rawNext);
                return true;
            }
        } // class Walker



        public static List<HierarchyItem> CollectItemsRecursive(
            IVsHierarchy hierarchy,
            uint itemId,
            Func<HierarchyItem, bool> predicate
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<HierarchyItem>();
            VsHierarchy.CollectItemsRecursiveInternal(hierarchy, itemId, predicate, result);
            return result;
        }


        public static void LogSolutionHierarchy() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_LOADEDINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];

                string projectName = Utils.EnvDteUtils.GetDteProjectUniqueNameFromVsHierarchy(hierarchy);
                Helpers.Diagnostic.Logger.LogDebug($"[Hierarchy] {projectName} (VSITEMID_ROOT)");

                VsHierarchy.LogSolutionHierarchyRecursive(hierarchy, VSConstants.VSITEMID_ROOT, 0);
            }
        }


        //
        // Internal logic
        //
        private static void CollectItemsRecursiveInternal(
            IVsHierarchy hierarchy,
            uint itemId,
            Func<HierarchyItem, bool> predicate,
            List<HierarchyItem> result
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            var name = nameObj as string;

            hierarchy.GetCanonicalName(itemId, out var canonicalName);
            var hierarchyItem = new HierarchyItem(hierarchy, itemId, name, canonicalName);

            if (predicate(hierarchyItem)) {
                result.Add(hierarchyItem);
            }

            foreach (var childId in Walker.GetChildren(hierarchy, itemId)) {
                VsHierarchy.CollectItemsRecursiveInternal(hierarchy, childId, predicate, result);
            }
        }

        private static void LogSolutionHierarchyRecursive(IVsHierarchy hierarchy, uint itemId, int indent) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (itemId != VSConstants.VSITEMID_ROOT) {
                hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string ?? "(null)";

                hierarchy.GetCanonicalName(itemId, out var canonicalName);

                string indentStr = new string(' ', indent * 2);
                Helpers.Diagnostic.Logger.LogDebug($"[{itemId}]{indentStr}{name} ({canonicalName})");
            }

            foreach (var childId in Walker.GetChildren(hierarchy, itemId)) {
                VsHierarchy.LogSolutionHierarchyRecursive(hierarchy, childId, indent + 1);
            }
        }
    }
}