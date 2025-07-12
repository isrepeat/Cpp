using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Telemetry;
using System.Security.RightsManagement;
using Newtonsoft.Json.Linq;


namespace TabsManagerExtension.VsShell.Project {
    public sealed class LoadedProjectNode :
        Helpers.ObservableObject,
        Helpers.Collections.IMultiStateElement,
        IDisposable {

        public ProjectNode ProjectNode { get; }


        private ShellProject? _shellProject = null;
        public ShellProject ShellProject { 
            get {
                if (_shellProject == null) {
                    System.Diagnostics.Debugger.Break();
                }
                return _shellProject;
            }
            private set {
                if (_shellProject != value) {
                    _shellProject = value;
                }
            }
        }


        private readonly List<VsShell.Document.DocumentNode> _sources = new();
        public IReadOnlyList<VsShell.Document.DocumentNode> Sources => _sources;


        private readonly List<VsShell.Document.SharedItemNode> _sharedItems = new();
        public IReadOnlyList<VsShell.Document.SharedItemNode> SharedItems => _sharedItems;


        private readonly List<VsShell.Document.ExternalInclude> _externalIncludes = new();
        public IReadOnlyList<VsShell.Document.ExternalInclude> ExternalIncludes => _externalIncludes;

        public bool IsIncludeSharedItems => _sharedItems.Count > 0;

        private ProjectExternalDependenciesTracker _projectExternalDependenciesTracker;

        public LoadedProjectNode(ProjectNode projectNode) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.ProjectNode = projectNode;
        }

        //
        // IDisposable
        //
        public void Dispose() {
            this.OnStateDisabled(null);
        }


        //
        // IMultiStateElement
        //
        public void OnStateEnabled(Helpers.Collections.IMultiStateElement previousState) {
            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            _projectExternalDependenciesTracker = new ProjectExternalDependenciesTracker(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            _projectExternalDependenciesTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;

            TabsManagerExtension.Services.TimeManagerService.Instance.Subscribe(Enums.TimerType._1s, this.OnRefreshExternalDependencies);
        }

        public void OnStateDisabled(Helpers.Collections.IMultiStateElement nextState) {
            TabsManagerExtension.Services.TimeManagerService.Instance.Unsubscribe(Enums.TimerType._1s, this.OnRefreshExternalDependencies);

            _projectExternalDependenciesTracker.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
            _projectExternalDependenciesTracker = null;
            this.ShellProject = null;
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        // NODE: Musts be called after SolutionHierarchyAnalyzerService build all projectNodes.
        public void UpdateDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var projectHierarchy = this.ProjectNode.ProjectHierarchy;
            var hierarchyItems = new List<Utils.VsHierarchyUtils.HierarchyItem>();

            foreach (var childId in Utils.VsHierarchyUtils.Walker.GetChildren(projectHierarchy.VsHierarchy, VSConstants.VSITEMID_ROOT)) {
                projectHierarchy.VsHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                if (!this.IsGuidName(name)) {
                    var resultItems = Utils.VsHierarchyUtils.CollectItemsRecursive(
                        projectHierarchy.VsHierarchy,
                        childId,
                        hierarchyItem => this.IsHeaderOrCppFile(hierarchyItem.CanonicalName));

                    hierarchyItems.AddRange(resultItems);
                }
            }

            var newSources = new List<VsShell.Document.DocumentNode>();
            var newSharedItems = new List<VsShell.Document.SharedItemNode>();

            foreach (var hierarchyItem in hierarchyItems) {
                bool isSharedItem = false;

                projectHierarchy.VsHierarchy.GetProperty(
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

                    projectHierarchy.VsHierarchy.GetProperty(
                        hierarchyItem.ItemId,
                        (int)__VSHPROPID7.VSHPROPID_SharedProjectHierarchy,
                        out var sharedProjectHierarchyObj);

                    if (sharedProjectHierarchyObj is IVsHierarchy hierarchy) {
                        sharedProjectHierarchy = hierarchy;
                    }

                    var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
                    var sharedSolutionProjectNode = solutionHierarchyAnalyzer.SolutionProjects
                        .FirstOrDefault(p => Equals(p.ProjectHierarchy.VsHierarchy, sharedProjectHierarchy));

                    if (sharedSolutionProjectNode == null) {
                        System.Diagnostics.Debugger.Break();
                    }

                    var newDocumentNodeCandidate = new VsShell.Document.SharedItemNode(
                            hierarchyItem.ItemId,
                            normalizedPath,
                            this.ProjectNode,
                            sharedSolutionProjectNode);

                    newSharedItems.Add(newDocumentNodeCandidate);
                }
                else {
                    var newDocumentNodeCandidate = new VsShell.Document.DocumentNode(
                            hierarchyItem.ItemId,
                            normalizedPath,
                            this.ProjectNode);

                    newSources.Add(newDocumentNodeCandidate);
                }
            }

            //
            // === Diff strategy ===
            //
            // Обновляем старый ItemId (который мог измениться после перезагрузки проекта),
            // чтобы корректно сравнивать с newDocumentNodeCandidate.
            foreach (var documentNode in _sources) {
                documentNode.TryRefreshItemId();
            }
            foreach (var sharedItemNode in _sharedItems) {
                sharedItemNode.TryRefreshItemId();
            }


            var sourcesToRemove = _sources.Except(newSources).ToList();
            var sourcesToAdd = newSources.Except(_sources).ToList();

            foreach (var item in sourcesToRemove) {
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateDocuments] Remove documentNode: {item}");
                item.Invalidated();
                _sources.Remove(item);
            }

            foreach (var item in sourcesToAdd) {
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateDocuments] Add documentNode: {item}");
                _sources.Add(item);
            }


            var sharedItemsToRemove = _sharedItems.Except(newSharedItems).ToList();
            var sharedItemsToAdd = newSharedItems.Except(_sharedItems).ToList();

            foreach (var item in sharedItemsToRemove) {
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateDocuments] Remove sharedItemNode: {item}");
                item.Invalidated();
                _sharedItems.Remove(item);
            }

            foreach (var item in sharedItemsToAdd) {
                Helpers.Diagnostic.Logger.LogDebug($"[UpdateDocuments] Add sharedItemNode: {item}");
                _sharedItems.Add(item);
            }
        }


        public override string ToString() {
            return $"LoadedProjectNode({this.ProjectNode.UniqueName})";
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnRefreshExternalDependencies() {
            _projectExternalDependenciesTracker.Refresh();
        }


        private void OnExternalDependenciesChanged(_EventArgs.ProjectExternalDependenciesChangedEventArgs e) {
            var externalIncludesToAdd = new List<VsShell.Document.ExternalInclude>();

            foreach (var hierarchyItem in e.Added) {
                var hierarchyItemName = hierarchyItem.CanonicalName ?? hierarchyItem.Name ?? string.Empty;
                var normalizedPath = Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                var newDocumentNodeCandidate = new VsShell.Document.ExternalInclude(
                    hierarchyItem.ItemId,
                    normalizedPath,
                    this.ProjectNode
                    );

                externalIncludesToAdd.Add(newDocumentNodeCandidate);
            }


            var externalIncludesToRemove = new List<VsShell.Document.ExternalInclude>();

            foreach (var hierarchyItem in e.Removed) {
                var hierarchyItemName = hierarchyItem.CanonicalName ?? hierarchyItem.Name ?? string.Empty;
                var normalizedPath = Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                var existExternalInclude = _externalIncludes
                    .FirstOrDefault(item => string.Equals(item.FilePath, normalizedPath, StringComparison.OrdinalIgnoreCase));

                externalIncludesToRemove.Add(existExternalInclude);
            }


            foreach (var item in externalIncludesToRemove) {
                Helpers.Diagnostic.Logger.LogDebug($"[OnExternalDependenciesChanged] Remove externalInclude: {item}");
                item.Invalidated();
                _externalIncludes.Remove(item);
            }

            foreach (var item in externalIncludesToAdd) {
                Helpers.Diagnostic.Logger.LogDebug($"[OnExternalDependenciesChanged] Add externalInclude: {item}");
                _externalIncludes.Add(item);
            }
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private bool IsGuidName(string? name) {
            return !string.IsNullOrEmpty(name) && name.StartsWith("{") && name.EndsWith("}");
        }

        private bool IsHeaderOrCppFile(string? name) {
            return !string.IsNullOrEmpty(name) &&
                (name.EndsWith(".h", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
                 name.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));
        }
    }
}