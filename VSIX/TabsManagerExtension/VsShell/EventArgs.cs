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


namespace TabsManagerExtension.VsShell._EventArgs {
    public sealed class DocumentNavigationEventArgs : EventArgs {
        public string? PreviousDocumentFullName { get; }
        public string CurrentDocumentFullName { get; }

        public DocumentNavigationEventArgs(string? previousDocumentFullName, string currentDocumentFullName) {
            this.PreviousDocumentFullName = previousDocumentFullName;
            this.CurrentDocumentFullName = currentDocumentFullName;
        }
    }

    public sealed class ProjectHierarchyChangedEventArgs : EventArgs {
        public VsShell.Hierarchy.IVsHierarchy? OldHierarchy { get; }
        public VsShell.Hierarchy.IVsHierarchy NewHierarchy { get; }

        public ProjectHierarchyChangedEventArgs(
            VsShell.Hierarchy.IVsHierarchy? oldHierarchy,
            VsShell.Hierarchy.IVsHierarchy newHierarchy
            ) {
            this.OldHierarchy = oldHierarchy;
            this.NewHierarchy = newHierarchy;
        }

        public bool TryGetRealHierarchy(out VsShell.Hierarchy.IVsRealHierarchy realHierarchy) {
            if (this.NewHierarchy is VsShell.Hierarchy.IVsRealHierarchy realNewHierarchy) {
                realHierarchy = realNewHierarchy;
                return true;
            }
            else if (this.OldHierarchy is VsShell.Hierarchy.IVsRealHierarchy realOldHierarchy) {
                realHierarchy = realOldHierarchy;
                return true;
            }

            realHierarchy = null;
            return false;
        }
    }


    public sealed class ProjectHierarchyItemsChangedEventArgs : EventArgs {
        public IVsHierarchy ProjectHierarchy { get; }
        public IReadOnlyList<Utils.VsHierarchyUtils.HierarchyItem> Added { get; }
        public IReadOnlyList<Utils.VsHierarchyUtils.HierarchyItem> Removed { get; }

        public ProjectHierarchyItemsChangedEventArgs(
            IVsHierarchy projectHierarchy,
            IReadOnlyList<Utils.VsHierarchyUtils.HierarchyItem> added,
            IReadOnlyList<Utils.VsHierarchyUtils.HierarchyItem> removed
            ) {
            this.ProjectHierarchy = projectHierarchy;
            this.Added = added;
            this.Removed = removed;
        }
    }
}