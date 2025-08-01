using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    namespace _Details {
        public partial class HierarchyItemCommonState :
            Helpers.MultiState.StaticCommonStateBase,
            IDisposable {

            public IVsHierarchy VsHierarchy { get; private set; }

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private uint _itemId;

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _name;

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _canonicalName;

            // Lazy
            private string _filePath;
            public string FilePath {
                get {
                    if (_filePath == null) {
                        var hierarchyItemName = this.CanonicalName ?? this.Name ?? string.Empty;
                        var normalizedPath = System.IO.Path
                            .GetFullPath(hierarchyItemName)
                            .TrimEnd(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar);

                        _filePath = normalizedPath;
                    }
                    return _filePath;
                }
            }

            [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet, NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private bool _isDisposed;

            public HierarchyItemCommonState(
                IVsHierarchy vsHierarchy,
                uint itemId
                ) {
                ThreadHelper.ThrowIfNotOnUIThread();

                this.VsHierarchy = vsHierarchy;
                this.ItemId = itemId;

                vsHierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                this.Name = nameObj as string;

                vsHierarchy.GetCanonicalName(itemId, out var canonicalName);
                this.CanonicalName = canonicalName;
            }


            public void Dispose() {
                if (this.IsDisposed) {
                    return;
                }

                this.VsHierarchy = null;
                this.ItemId = VSConstants.VSITEMID_NIL;
                this.Name = "<disposed>";
                this.CanonicalName = "<disposed>";

                _filePath = "<disposed>";
                base.OnCommonStatePropertyChanged(nameof(this.FilePath));

                this.IsDisposed = true;
            }


            public override bool Equals(object? obj) {
                if (obj is HierarchyItemCommonState other) {
                    return StringComparer.OrdinalIgnoreCase.Equals(this.CanonicalName, other.CanonicalName);
                }
                return false;
            }

            public override int GetHashCode() {
                int hash = this.CanonicalName != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.CanonicalName)
                    : 0;
                
                return hash;
            }

            public string ToStringCore() {
                return $"ItemId={this.ItemId}, CanonicalName='{this.CanonicalName}'";
            }
        }
    } // namesace _Details



    public abstract class HierarchyItemCommonStateViewModel :
        Helpers.MultiState.CommonStateViewModelBase<_Details.HierarchyItemCommonState> {
        public uint ItemId => base.CommonState.ItemId;
        public string Name => base.CommonState.Name;
        public string CanonicalName => base.CommonState.CanonicalName;
        public string FilePath => base.CommonState.FilePath;
        public bool IsDisposed => base.CommonState.IsDisposed;

        protected HierarchyItemCommonStateViewModel(_Details.HierarchyItemCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(_Details.HierarchyItemCommonState.ItemId):
                    base.OnPropertyChanged(nameof(this.ItemId));
                    break;

                case nameof(_Details.HierarchyItemCommonState.Name):
                    base.OnPropertyChanged(nameof(this.Name));
                    break;

                case nameof(_Details.HierarchyItemCommonState.CanonicalName):
                    base.OnPropertyChanged(nameof(this.CanonicalName));
                    break;

                case nameof(_Details.HierarchyItemCommonState.FilePath):
                    base.OnPropertyChanged(nameof(this.FilePath));
                    break;

                case nameof(_Details.HierarchyItemCommonState.IsDisposed):
                    base.OnPropertyChanged(nameof(this.IsDisposed));
                    break;
            }
        }
    }
}