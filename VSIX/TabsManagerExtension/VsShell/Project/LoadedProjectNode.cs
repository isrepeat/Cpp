using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class LoadedProjectNode : ShellProject {
        public SolutionProjectNode SolutionProjectNode { get; }


        private readonly List<VsShell.Document.DocumentNode> _sources = new();
        public IReadOnlyList<VsShell.Document.DocumentNode> Sources => _sources;


        private readonly List<VsShell.Document.SharedItemNode> _sharedItems = new();
        public IReadOnlyList<VsShell.Document.SharedItemNode> SharedItems => _sharedItems;


        private readonly List<VsShell.Document.ExternalInclude> _externalIncludes = new();
        public IReadOnlyList<VsShell.Document.ExternalInclude> ExternalIncludes => _externalIncludes;

        public bool IsIncludeSharedItems => _sharedItems.Count > 0;


        public LoadedProjectNode(SolutionProjectNode solutionProjectNode)
            : base(Utils.EnvDteUtils.GetDteProjectFromHierarchy(solutionProjectNode.VsHierarchy)) {

            ThreadHelper.ThrowIfNotOnUIThread();

            this.SolutionProjectNode = solutionProjectNode;
        }


        // NODE: Musts be called after SolutionHierarchyAnalyzerService build all projectNodes.
        public void UpdateDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var projectHierarchy = this.SolutionProjectNode.VsHierarchy;
            var hierarchyItems = new List<Utils.VsHierarchy.HierarchyItem>();

            foreach (var childId in Utils.VsHierarchy.Walker.GetChildren(projectHierarchy, VSConstants.VSITEMID_ROOT)) {
                projectHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                // Игнорируем вложенные элементы для виртуальных GUID-папок (External Dependencies и проч.)
                if (!this.IsGuidName(name)) {
                    var resultItems = Utils.VsHierarchy.CollectItemsRecursive(
                        projectHierarchy,
                        childId,
                        hierarchyItem => this.IsHeaderOrCppFile(hierarchyItem.CanonicalName));

                    hierarchyItems.AddRange(resultItems);
                }
            }

            _sources.Clear();
            _sharedItems.Clear();

            foreach (var hierarchyItem in hierarchyItems) {
                bool isSharedItem = false;

                projectHierarchy.GetProperty(
                    hierarchyItem.ItemId,
                    (int)__VSHPROPID7.VSHPROPID_IsSharedItem,
                    out var isSharedItemObj);

                if (isSharedItemObj is bool boolVal) {
                    isSharedItem = boolVal;
                }

                var hierarchyItemName = hierarchyItem.CanonicalName ?? hierarchyItem.Name ?? string.Empty;
                var normalizedPath = Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                if (isSharedItem) {
                    IVsHierarchy? sharedProjectHierarchy = null;

                    projectHierarchy.GetProperty(
                        hierarchyItem.ItemId,
                        (int)__VSHPROPID7.VSHPROPID_SharedProjectHierarchy,
                        out var sharedProjectHierarchyObj);

                    if (sharedProjectHierarchyObj is IVsHierarchy hierarchy) {
                        sharedProjectHierarchy = hierarchy;
                    }

                    var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
                    var sharedSolutionProjectNode = solutionHierarchyAnalyzer.SolutionProjects
                        .FirstOrDefault(p => Equals(p.VsHierarchy, sharedProjectHierarchy));

                    if (sharedSolutionProjectNode == null) {
                        System.Diagnostics.Debugger.Break();
                    }

                    _sharedItems.Add(new VsShell.Document.SharedItemNode(
                        hierarchyItem.ItemId,
                        normalizedPath,
                        this.SolutionProjectNode,
                        sharedSolutionProjectNode));
                }
                else {
                    _sources.Add(new VsShell.Document.DocumentNode(
                        hierarchyItem.ItemId,
                        normalizedPath,
                        this.SolutionProjectNode));
                }
            }
        }


        public void UpdateExternalIncludes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var projectHierarchy = this.SolutionProjectNode.VsHierarchy;
            var hierarchyItems = new List<Utils.VsHierarchy.HierarchyItem>();

            foreach (var childId in Utils.VsHierarchy.Walker.GetChildren(projectHierarchy, VSConstants.VSITEMID_ROOT)) {
                projectHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                // только для GUID-папок (External Dependencies) запускаем рекурсивную обработку
                if (this.IsGuidName(name)) {
                    var resultItems = Utils.VsHierarchy.CollectItemsRecursive(
                        projectHierarchy,
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

                _externalIncludes.Add(new VsShell.Document.ExternalInclude(
                    hierarchyItem.ItemId,
                    normalizedPath,
                    this.SolutionProjectNode
                    ));
            }
        }

        public override string ToString() {
            return $"ProjectNode({this.SolutionProjectNode.UniqueName})";
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

    }
}