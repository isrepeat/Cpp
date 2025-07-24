using System;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Helpers.Properties;
using System.Linq;
using Newtonsoft.Json.Linq;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using Helpers.Attributes.Markers.Access;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public interface IHierarchy {
        Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }
        public void Invalidate();
    }

    public interface IRealHierarchy : IHierarchy {
    }

    public interface IStubHierarchy : IHierarchy {
    }


    public static class VsHierarchyFactory {
        public static IHierarchy CreateHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return VsHierarchyFactory.IsRealHierarchy(hierarchy)
                ? new VsHierarchyFactory.VsRealHierarchy(hierarchy)
                : new VsHierarchyFactory.VsStubHierarchy(hierarchy);
        }

        private static bool IsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
            Microsoft.VisualStudio.Shell.ThreadHelper.ThrowIfNotOnUIThread();
            return hierarchy is Microsoft.VisualStudio.Shell.Interop.IVsProject;
        }


        private sealed class VsRealHierarchy : IRealHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }
            public bool IsInvalidated { get; private set; }

            public VsRealHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.VsHierarchy = hierarchy;
                this.IsInvalidated = false;
            }

            public void Invalidate() {
                this.IsInvalidated = false;
            }
        }

        private sealed class VsStubHierarchy : IStubHierarchy {
            public Microsoft.VisualStudio.Shell.Interop.IVsHierarchy VsHierarchy { get; }
            public bool IsInvalidated { get; private set; }

            public VsStubHierarchy(Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy) {
                this.VsHierarchy = hierarchy;
                this.IsInvalidated = false;
            }

            public void Invalidate() {
                this.IsInvalidated = false;
            }
        }
    }
}