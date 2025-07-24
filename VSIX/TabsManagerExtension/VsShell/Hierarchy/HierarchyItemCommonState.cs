using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Hierarchy {
    public partial class HierarchyItemCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {
        
        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private IHierarchy _hierarchy;

        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private uint _itemId;

        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
        private string _name;

        [ObservableProperty(NotifyMethod = "base.OnSharedStatePropertyChanged")]
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


        public HierarchyItemCommonState(
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

        public void Dispose() {
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

        public override string ToString() {
            return $"HierarchyItem(ItemId={this.ItemId}, Name='{this.Name}', CanonicalName='{this.CanonicalName}')";
        }
    }


    
    public abstract class HierarchyItemCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<HierarchyItemCommonState> {
        protected HierarchyItemCommonStateViewModel(HierarchyItemCommonState commonState)
            : base(commonState) {
        }

        protected override void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            // ...
        }
    }
}