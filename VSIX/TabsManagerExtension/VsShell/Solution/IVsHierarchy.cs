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


    public sealed class VsHierarchy : IVsHierarchy {
        public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }

        //private VsHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
        //    Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
        //    this.Hierarchy = hierarchy;
        //}

        public static IVsHierarchy CreateHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return VsHierarchy.IsRealHierarchy(hierarchy)
                ? new VsHierarchy.VsRealHierarchy(hierarchy)
                : new VsHierarchy.VsStubHierarchy(hierarchy);
        }

        public static bool IsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return hierarchy is Microsoft.VisualStudio.Shell.Interop.IVsProject;
        }


        private sealed class VsRealHierarchy : IVsRealHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }

            public VsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.Hierarchy = hierarchy;
            }
        }

        private sealed class VsStubHierarchy : IVsStubHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy Hierarchy { get; }

            public VsStubHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.Hierarchy = hierarchy;
            }
        }
    }
}