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
    public static class VsHierarchyWalker {
        public static List<uint> GetChildren(IVsHierarchy hierarchy, uint parentId) {
            var result = new List<uint>();

            if (VsHierarchyWalker.TryGetFirstChild(hierarchy, parentId, out var childId)) {
                do {
                    result.Add(childId);
                }
                while (VsHierarchyWalker.TryGetNextSibling(hierarchy, childId, out childId));
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

                VsHierarchyWalker.LogSolutionHierarchyRecursive(hierarchy, VSConstants.VSITEMID_ROOT, 0);
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

            foreach (var childId in Utils.VsHierarchyWalker.GetChildren(hierarchy, itemId)) {
                VsHierarchyWalker.LogSolutionHierarchyRecursive(hierarchy, childId, indent + 1);
            }
        }
    }
}