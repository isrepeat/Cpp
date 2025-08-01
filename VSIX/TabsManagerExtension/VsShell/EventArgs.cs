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
        public Hierarchy.HierarchyItemEntry? OldHierarchy { get; }
        public Hierarchy.HierarchyItemEntry NewHierarchy { get; }

        public ProjectHierarchyChangedEventArgs(
            Hierarchy.HierarchyItemEntry? oldHierarchy,
            Hierarchy.HierarchyItemEntry newHierarchy
            ) {
            this.OldHierarchy = oldHierarchy;
            this.NewHierarchy = newHierarchy;
        }
    }



    public sealed class ProjectHierarchyItemsChangedEventArgs : EventArgs {
        public IReadOnlyList<Hierarchy.HierarchyItemEntry> Added { get; }
        public IReadOnlyList<Hierarchy.HierarchyItemEntry> Removed { get; }

        public ProjectHierarchyItemsChangedEventArgs(
            IReadOnlyList<Hierarchy.HierarchyItemEntry> added,
            IReadOnlyList<Hierarchy.HierarchyItemEntry> removed
            ) {
            this.Added = added;
            this.Removed = removed;
        }
    }
}