using Microsoft.VisualStudio.Shell;
using System;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public interface IVsHierarchy {
        Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }
    }

    public interface IVsRealHierarchy : IVsHierarchy {
    }

    public interface IVsStubHierarchy : IVsHierarchy {
    }


    public static class VsHierarchyFactory {
        public static IVsHierarchy CreateHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return VsHierarchyFactory.IsRealHierarchy(hierarchy)
                ? new VsHierarchyFactory.VsRealHierarchy(hierarchy)
                : new VsHierarchyFactory.VsStubHierarchy(hierarchy);
        }

        private static bool IsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return hierarchy is Microsoft.VisualStudio.Shell.Interop.IVsProject;
        }


        private sealed class VsRealHierarchy : IVsRealHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }

            public VsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.VsHierarchy = hierarchy;
            }
        }

        private sealed class VsStubHierarchy : IVsStubHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }

            public VsStubHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.VsHierarchy = hierarchy;
            }
        }
    }
}