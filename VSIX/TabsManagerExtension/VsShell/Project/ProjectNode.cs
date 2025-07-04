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
        public string? FirstCppRelatedFile { get; }

        
        private readonly List<VsShell.Document.ExternalInclude> _externalIncludes = new();
        public IReadOnlyList<VsShell.Document.ExternalInclude> ExternalIncludes => _externalIncludes;


        public ProjectNode(
            EnvDTE.Project dteProject,
            IVsHierarchy hierarchy) : base(dteProject) {

            this.VsHierarchy = hierarchy;
            this.FirstCppRelatedFile = this.FindFirstCppOrHeaderFile();
        }


        public void UpdateExternalIncludes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var childId in Utils.VsHierarchyWalker.GetChildren(this.VsHierarchy, VSConstants.VSITEMID_ROOT)) {
                this.VsHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                // только для GUID-папок (External Dependencies) запускаем рекурсивную обработку
                if (this.IsGuidName(name)) {
                    this.CollectExternalIncludesRecursive(childId);
                }
            }
        }

        private void CollectExternalIncludesRecursive(uint itemId) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.VsHierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            var name = nameObj as string;

            this.VsHierarchy.GetCanonicalName(itemId, out var canonicalName);

            // внутри виртуальной папки ищем файлы
            if (this.IsExternalIncludeFile(name)) {
                var fileKey = Path.GetFullPath(canonicalName ?? name)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                    .ToLowerInvariant();

                var externalInclude = new VsShell.Document.ExternalInclude(this, fileKey, itemId);
                this._externalIncludes.Add(externalInclude);
            }

            // продолжаем рекурсию для всех дочерних элементов
            foreach (var childId in Utils.VsHierarchyWalker.GetChildren(this.VsHierarchy, itemId)) {
                this.CollectExternalIncludesRecursive(childId);
            }
        }



        private string? FindFirstCppOrHeaderFile() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return this.FindFirstCppOrHeaderFileRecursive(this.VsHierarchy, VSConstants.VSITEMID_ROOT);
        }

        private string? FindFirstCppOrHeaderFileRecursive(IVsHierarchy hierarchy, uint itemId) {
            ThreadHelper.ThrowIfNotOnUIThread();

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            var name = nameObj as string;

            hierarchy.GetCanonicalName(itemId, out var canonicalName);

            if (this.IsCppProjectRelatedFile(canonicalName)) {
                return canonicalName;
            }

            // Не проверяем guid элементы такие как например "External Dependendies",
            // потому что может найтись тот же файл который мы захотим открыть через ExternalInclude.Open.
            if (!this.IsGuidName(name)) {
                foreach (var childId in Utils.VsHierarchyWalker.GetChildren(hierarchy, itemId)) {
                    var found = this.FindFirstCppOrHeaderFileRecursive(hierarchy, childId);
                    if (found != null) {
                        return found;
                    }
                }
            }

            return null;
        }


        private bool IsGuidName(string? name) {
            return !string.IsNullOrEmpty(name) && name.StartsWith("{") && name.EndsWith("}");
        }

        private bool IsExternalIncludeFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase));
        }

        private bool IsCppProjectRelatedFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}