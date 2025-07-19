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
        public VsShell.Hierarchy.IHierarchy? OldHierarchy { get; }
        public VsShell.Hierarchy.IHierarchy NewHierarchy { get; }

        public ProjectHierarchyChangedEventArgs(
            VsShell.Hierarchy.IHierarchy? oldHierarchy,
            VsShell.Hierarchy.IHierarchy newHierarchy
            ) {
            this.OldHierarchy = oldHierarchy;
            this.NewHierarchy = newHierarchy;
        }

        public bool TryGetRealHierarchy(out VsShell.Hierarchy.IRealHierarchy realHierarchy) {
            if (this.NewHierarchy is VsShell.Hierarchy.IRealHierarchy realNewHierarchy) {
                realHierarchy = realNewHierarchy;
                return true;
            }
            else if (this.OldHierarchy is VsShell.Hierarchy.IRealHierarchy realOldHierarchy) {
                realHierarchy = realOldHierarchy;
                return true;
            }

            realHierarchy = null;
            return false;
        }
    }


    public sealed class ProjectHierarchyItemsChangedEventArgs : EventArgs {
        public IVsHierarchy ProjectHierarchy { get; }
        public IReadOnlyList<Hierarchy.HierarchyItem> Added { get; }
        public IReadOnlyList<Hierarchy.HierarchyItem> Removed { get; }

        public ProjectHierarchyItemsChangedEventArgs(
            IVsHierarchy projectHierarchy,
            IReadOnlyList<Hierarchy.HierarchyItem> added,
            IReadOnlyList<Hierarchy.HierarchyItem> removed
            ) {
            this.ProjectHierarchy = projectHierarchy;
            this.Added = added;
            this.Removed = removed;
        }
    }
}