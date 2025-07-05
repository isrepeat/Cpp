using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class ProjectNode : ShellProject {
        public IVsHierarchy VsHierarchy { get; }

        private readonly List<VsShell.Document.DocumentNode> _documentNodes = new();
        public IReadOnlyList<VsShell.Document.DocumentNode> DocumentNodes => _documentNodes;


        private readonly List<VsShell.Document.ExternalInclude> _externalIncludes = new();
        public IReadOnlyList<VsShell.Document.ExternalInclude> ExternalIncludes => _externalIncludes;


        public ProjectNode(EnvDTE.Project dteProject, IVsHierarchy hierarchy) : base(dteProject) {
            this.VsHierarchy = hierarchy;
        }


        public void UpdateDocumentNodes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var hierarchyItems = new List<Utils.VsHierarchy.HierarchyItem>();

            foreach (var childId in Utils.VsHierarchy.Walker.GetChildren(this.VsHierarchy, VSConstants.VSITEMID_ROOT)) {
                this.VsHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                // Игнорируем вложенные элементы для виртуальных GUID-папок (External Dependencies и проч.)
                if (!this.IsGuidName(name)) {
                    var resultItems = Utils.VsHierarchy.CollectItemsRecursive(
                        this.VsHierarchy,
                        childId,
                        hierarchyItem => this.IsHeaderOrCppFile(hierarchyItem.CanonicalName));

                    hierarchyItems.AddRange(resultItems);
                }
            }

            _documentNodes.Clear();
            foreach (var hierarchyItem in hierarchyItems) {
                var hierarchyItemName = hierarchyItem.CanonicalName ?? hierarchyItem.Name ?? string.Empty;
                var normalizedPath = Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                _documentNodes.Add(new VsShell.Document.DocumentNode(this, normalizedPath, hierarchyItem.ItemId));
            }
        }


        public void UpdateExternalIncludes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var hierarchyItems = new List<Utils.VsHierarchy.HierarchyItem>();

            foreach (var childId in Utils.VsHierarchy.Walker.GetChildren(this.VsHierarchy, VSConstants.VSITEMID_ROOT)) {
                this.VsHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                // только для GUID-папок (External Dependencies) запускаем рекурсивную обработку
                if (this.IsGuidName(name)) {
                    var resultItems = Utils.VsHierarchy.CollectItemsRecursive(
                        this.VsHierarchy,
                        childId,
                        hierarchyItem => this.IsExternalIncludeFile(hierarchyItem.CanonicalName));

                    hierarchyItems.AddRange(resultItems);
                }
            }

            _externalIncludes.Clear();
            foreach (var hierarchyItem in hierarchyItems) {
                var hierarchyItemName = hierarchyItem.CanonicalName ?? hierarchyItem.Name ?? string.Empty;
                var normalizedPath = Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                _externalIncludes.Add(new VsShell.Document.ExternalInclude(this, normalizedPath, hierarchyItem.ItemId));
            }
        }


        private bool IsGuidName(string? name) {
            return !string.IsNullOrEmpty(name) && name.StartsWith("{") && name.EndsWith("}");
        }
        
        private bool IsHeaderOrCppFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }

        private bool IsExternalIncludeFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase));
        }



        public override bool Equals(object? obj) {
            if (obj is not ProjectNode other) {
                return false;
            }

            return StringComparer.OrdinalIgnoreCase.Equals(this.Project.UniqueName, other.Project.UniqueName);
        }

        public override int GetHashCode() {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(this.Project.UniqueName ?? string.Empty);
        }

        public override string ToString() {
            return $"ProjectNode({this.Project.UniqueName})";
        }
    }
}