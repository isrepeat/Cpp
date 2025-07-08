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

    public sealed class ProjectLoadStateChangedEventArgs : EventArgs {
        public IVsHierarchy? StubHierarchy { get; }
        public IVsHierarchy? RealHierarchy { get; }
        public EnvDTE.Project? DteProject { get; }

        public ProjectLoadStateChangedEventArgs(
            IVsHierarchy? stubHierarchy,
            IVsHierarchy? realHierarchy,
            EnvDTE.Project? dteProject
            ) {
            this.StubHierarchy = stubHierarchy;
            this.RealHierarchy = realHierarchy;
            this.DteProject = dteProject;
        }
    }
}