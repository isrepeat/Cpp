using Microsoft.VisualStudio.Shell;
using System;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public interface IVsHierarchy {
        Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }
    }

    public interface IVsRealHierarchy : IVsHierarchy {
    }

    public interface IVsStubHierarchy : IVsHierarchy {
    }


    public sealed class VsRealHierarchy : IVsRealHierarchy {
        public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }

        public VsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (hierarchy is not Microsoft.VisualStudio.Shell.Interop.IVsProject) {
                throw new ArgumentException("Provided hierarchy is not a loaded IVsProject hierarchy.", nameof(hierarchy));
            }
            this.Hierarchy = hierarchy;
        }
    }

    public sealed class VsStubHierarchy : IVsStubHierarchy {
        public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }

        public VsStubHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (hierarchy is Microsoft.VisualStudio.Shell.Interop.IVsProject) {
                throw new ArgumentException("Provided hierarchy is a loaded IVsProject, expected stub hierarchy.", nameof(hierarchy));
            }
            this.Hierarchy = hierarchy;
        }
    }
}