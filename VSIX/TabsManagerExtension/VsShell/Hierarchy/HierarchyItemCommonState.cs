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
            HelpersV4.Collections.CommonStateBase,
            IDisposable {

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private IHierarchy _hierarchy;

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private uint _itemId;

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _name;

            [ObservableProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
            private string _canonicalName;


            private string _filePath;
            public string FilePath {
                get {
                    // Lazy
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
                Microsoft.VisualStudio.Shell.Interop.IVsHierarchy vsHierarchy,
                uint itemId
                ) {
                ThreadHelper.ThrowIfNotOnUIThread();

                this.Hierarchy = VsHierarchyFactory.CreateHierarchy(vsHierarchy);
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

                this.Hierarchy = null;
                this.ItemId = VSConstants.VSITEMID_NIL;
                this.Name = "<disposed>";
                this.CanonicalName = "<disposed>";

                _filePath = "<disposed>";
                base.OnCommonStatePropertyChanged(nameof(this.FilePath));

                this.IsDisposed = true;
            }


            public override bool Equals(object? obj) {
                return obj is HierarchyItemCommonState other &&
                       StringComparer.OrdinalIgnoreCase.Equals(this.CanonicalName, other.CanonicalName);
            }

            public override int GetHashCode() {
                return this.CanonicalName != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.CanonicalName)
                    : 0;
            }
        }
    } // namesace _Details



    public abstract class HierarchyItemCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<_Details.HierarchyItemCommonState> {
        public uint ItemId => this.CommonState.ItemId;
        public string Name => this.CommonState.Name;
        public string CanonicalName => this.CommonState.CanonicalName;
        public string FilePath => this.CommonState.FilePath;
        public bool IsDisposed => this.CommonState.IsDisposed;

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



    namespace _Details {
        // TODO: Add to CodeAnalyzer support generate generic classes.
        public abstract class HierarchyItemEntryBaseViewModel :
            Helpers.ObservableObject,
            IDisposable {
            public HierarchyItemCommonStateViewModel BaseViewModel => this.MultiStateBase.AsViewModel<HierarchyItemCommonStateViewModel>();
            protected HierarchyItemMultiStateElementBase MultiStateBase { get; }

            public HierarchyItemEntryBaseViewModel(HierarchyItemMultiStateElementBase multiStateBase) {
                this.MultiStateBase = multiStateBase;
                this.MultiStateBase.StateChanged += this.OnMultiStateChanged;
            }
            public void Dispose() {
                this.MultiStateBase.StateChanged -= this.OnMultiStateChanged;
                this.MultiStateBase.Dispose();
            }

            public override string ToString() {
                return this.MultiStateBase.ToString();
            }

            protected abstract void OnMultiStateChanged();
        }
    } // namesace _Details



    public abstract class HierarchyItemEntryBase : _Details.HierarchyItemEntryBaseViewModel {
        public bool IsInvalidated => this.MultiStateBase.Current is InvalidatedHierarchyItem;
        public HierarchyItemEntryBase(HierarchyItemMultiStateElementBase multiStateBase) : base(multiStateBase) {
        }
        protected override void OnMultiStateChanged() {
            base.OnPropertyChanged(nameof(this.IsInvalidated));
        }
    }


    public partial class HierarchyItemEntry : HierarchyItemEntryBase {
        [ObservableMultiStateProperty(NotifyMethod = "base.OnPropertyChanged")]
        private HierarchyItemMultiStateElement _multiState;

        public HierarchyItemEntry(HierarchyItemMultiStateElement multiState) : base(multiState) {
            this.MultiState = multiState;
            this.MultiState.SwitchTo<HierarchyItem>();
        }
    }
}