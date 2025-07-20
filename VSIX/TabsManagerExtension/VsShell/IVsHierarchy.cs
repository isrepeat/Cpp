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



    public partial class HierarchyItem :
        IDisposable {

        public IHierarchy Hierarchy { get; }

        [InvalidatableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private uint _itemId;

        [InvalidatableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private string _name;

        [InvalidatableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private string _canonicalName;

        [InvalidatableLazyProperty(AccessMarker.Get, nameof(this.InitializeFilePathField))]
        private string _filePath;


        private bool _isInvalidated = false;
        public bool IsInvalidated => _isInvalidated;


        private bool _isDisposed = false;
        public bool IsDisposed => _isDisposed;

        public HierarchyItem(
            Microsoft.VisualStudio.Shell.Interop.IVsHierarchy hierarchy,
            uint itemId
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Hierarchy = VsHierarchyFactory.CreateHierarchy(hierarchy);
            this.ItemId = itemId;

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            this.Name = nameObj as string;

            hierarchy.GetCanonicalName(itemId, out var canonicalName);
            this.CanonicalName = canonicalName;
        }


        public void Invalidate() {
            if (_isDisposed) {
                return;
            }
            if (_isInvalidated) {
                return;
            }

            this.InvalidateProperties();

            _isInvalidated = true;
        }


        public void Dispose() {
            if (_isDisposed) {
                return;
            }

            this.Invalidate();

            foreach (var disposable in Helpers.Reflection.GetPropertiesOf<IDisposable>(this)) {
                disposable.Dispose();
            }

            _isDisposed = true;
        }


        public override bool Equals(object? obj) {
            return obj is HierarchyItem other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.CanonicalNameCached, other.CanonicalNameCached);
        }

        public override int GetHashCode() {
            return this.CanonicalNameCached != null
                ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.CanonicalNameCached)
                : 0;
        }

        public override string ToString() {
            return $"HierarchyItem(ItemId={this.ItemId}, Name='{this.Name}', CanonicalName='{this.CanonicalName}')";
        }


        private string InitializeFilePathField() {
            var hierarchyItemName = this.CanonicalName ?? this.Name ?? string.Empty;
            var normalizedPath = System.IO.Path
                .GetFullPath(hierarchyItemName)
                .TrimEnd(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar);
            
            return normalizedPath;
        }
    }
}