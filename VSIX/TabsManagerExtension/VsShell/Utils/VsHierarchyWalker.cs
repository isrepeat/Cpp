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

            if (TryGetFirstChild(hierarchy, parentId, out var childId)) {
                do {
                    result.Add(childId);
                }
                while (TryGetNextSibling(hierarchy, childId, out childId));
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
    }
}