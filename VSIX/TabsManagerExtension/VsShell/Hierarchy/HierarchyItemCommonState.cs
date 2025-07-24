using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using EnvDTE;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public abstract class HierarchyItemCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<HierarchyItemCommonState> {
        public uint ItemId => this.CommonState.ItemId;
        public string Name => this.CommonState.Name;
        public string CanonicalName => this.CommonState.CanonicalName;
        public string FilePath => this.CommonState.FilePath;

        protected HierarchyItemCommonStateViewModel(HierarchyItemCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(HierarchyItemCommonState.ItemId):
                    base.OnPropertyChanged(nameof(this.ItemId));
                    break;

                case nameof(HierarchyItemCommonState.Name):
                    base.OnPropertyChanged(nameof(this.Name));
                    break;

                case nameof(HierarchyItemCommonState.CanonicalName):
                    base.OnPropertyChanged(nameof(this.CanonicalName));
                    break;

                case nameof(HierarchyItemCommonState.FilePath):
                    base.OnPropertyChanged(nameof(this.FilePath));
                    break;
            }
        }
    }



    public partial class HierarchyItemCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        // TODO: generate
        public static class PropertyInfo {
            public static readonly HelpersV4.Collections.CommonProperty<HierarchyItemCommonState, IHierarchy> Hierarchy =
                new(typeof(HierarchyItemCommonState).GetProperty(nameof(HierarchyItemCommonState.Hierarchy)));

            public static readonly HelpersV4.Collections.CommonProperty<HierarchyItemCommonState, uint> ItemId =
                new(typeof(HierarchyItemCommonState).GetProperty(nameof(HierarchyItemCommonState.ItemId)));

            public static readonly HelpersV4.Collections.CommonProperty<HierarchyItemCommonState, string> Name =
                new(typeof(HierarchyItemCommonState).GetProperty(nameof(HierarchyItemCommonState.Name)));

            public static readonly HelpersV4.Collections.CommonProperty<HierarchyItemCommonState, string> CanonicalName =
                new(typeof(HierarchyItemCommonState).GetProperty(nameof(HierarchyItemCommonState.CanonicalName)));

            public static readonly HelpersV4.Collections.CommonProperty<HierarchyItemCommonState, string> FilePath =
                new(typeof(HierarchyItemCommonState).GetProperty(nameof(HierarchyItemCommonState.FilePath)));
        }


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

        protected bool _disposed = false;

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
            if (_disposed) {
                return;
            }

            _hierarchy = null;
            _itemId = VSConstants.VSITEMID_NIL;
            _name = "<disposed>";
            _canonicalName = "<disposed>";
            _filePath = "<disposed>";

            _disposed = true;
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
}