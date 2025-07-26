using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Package;
using TabsManagerExtension.VsShell.Document;
using Microsoft.VisualStudio.VSHelp;


namespace TabsManagerExtension.VsShell.Project {
    public abstract class ProjectMultiStateElementBase :
        HelpersV4.Collections.MultiStateContainer<
            _Details.ProjectCommonState,
            LoadedProject,
            UnloadedProject> {

        protected ProjectMultiStateElementBase(_Details.ProjectCommonState commonState)
            : base(commonState) {
        }

        protected ProjectMultiStateElementBase(
            _Details.ProjectCommonState commonState,
            Func<_Details.ProjectCommonState, LoadedProject> factoryA,
            Func<_Details.ProjectCommonState, UnloadedProject> factoryB)
            : base(commonState, factoryA, factoryB) {
        }
    }


    public class ProjectMultiStateElement : ProjectMultiStateElementBase {
        public ProjectMultiStateElement(
            ProjectNode projectNode
            ) : base(new _Details.ProjectCommonState(projectNode)) {
        }
    }


    public class LoadedProject :
        ProjectCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        private ShellProject? _shellProject = null;
        public ShellProject ShellProject {
            get {
                return _shellProject;
            }
            set {
                if (_shellProject != value) {
                    _shellProject = value;
                }
            }
        }

        private ProjectHierarchyTracker _projectHierarchyTracker;

        public LoadedProject(_Details.ProjectCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            var currentExternalIncludesHashSet = this.ExternalIncludes
                .Select(d => d.BaseViewModel.HierarchyItemEntry)
                .ToHashSet();

            var currentSharedItemsHashSet = this.SharedItems
                .Select(d => d.BaseViewModel.HierarchyItemEntry)
                .ToHashSet();

            var currentSourcesHashSet = this.Sources
                .Select(d => d.BaseViewModel.HierarchyItemEntry)
                .ToHashSet();


            _projectHierarchyTracker = new ProjectHierarchyTracker(
                this.ProjectNode.ProjectHierarchy.VsHierarchy,
                currentExternalIncludesHashSet,
                currentSharedItemsHashSet,
                currentSourcesHashSet
                );

            _projectHierarchyTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;
            _projectHierarchyTracker.SharedItemsChanged += this.OnSharedItemsChanged;
            _projectHierarchyTracker.SourcesChanged += this.OnSourcesChanged;

            _projectHierarchyTracker.ExternalDependenciesChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SharedItemsChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SourcesChanged.InvokeForLastHandlerIfTriggered();
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            foreach (var externalInclude in this.CommonState.ExternalIncludes) {
                externalInclude.MultiState.SwitchTo<Document.InvalidatedDocument>();
            }
            foreach (var sharedItem in this.CommonState.SharedItems) {
                sharedItem.MultiState.SwitchTo<Document.InvalidatedDocument>();
            }
            foreach (var source in this.CommonState.Sources) {
                source.MultiState.SwitchTo<Document.InvalidatedDocument>();
            }
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public override string ToString() {
            return $"LoadedProject({this.ProjectNode.UniqueName})";
        }

        protected void OpenWithProjectContext() {
            // ...
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);
        }

        private void OnExternalDependenciesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(this.ExternalIncludes, e, this.CreateExternalInclude);
        }

        private void OnSharedItemsChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(this.SharedItems, e, this.CreateSharedItem);
        }

        private void OnSourcesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(this.Sources, e, this.CreateSource);
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void UpdateProjectHierarchyItems<TDocumentEntry>(
           Helpers.Collections.DisposableList<TDocumentEntry> currentDocuments,
           _EventArgs.ProjectHierarchyItemsChangedEventArgs e,
           Func<Hierarchy.HierarchyItemEntry, TDocumentEntry> createNewDocumenEntryFactory
            )
            where TDocumentEntry : VsShell.Document.DocumentEntryBase {

            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var hierarchyItemEntry in e.Added) {
                var existDocument = this.FindDocumentByHierarchyItem(hierarchyItemEntry, currentDocuments);
                if (existDocument != null) {
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Remove exist: {existDocument} [{this.ProjectNode.UniqueName}]");
                    currentDocuments.RemoveAndDispose(existDocument);
                }

                var newDocumentEntry = createNewDocumenEntryFactory(hierarchyItemEntry);

                Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Add: {newDocumentEntry} [{this.ProjectNode.UniqueName}]");
                currentDocuments.Add(newDocumentEntry);
            }

            foreach (var hierarchyItemEntry in e.Removed) {
                var existDocumentEntry = this.FindDocumentByHierarchyItem(hierarchyItemEntry, currentDocuments);

                Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Remove: {existDocumentEntry} [{this.ProjectNode.UniqueName}]");
                currentDocuments.RemoveAndDispose(existDocumentEntry);
            }
        }


        private TDocumentEntry? FindDocumentByHierarchyItem<TDocumentEntry>(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry,
            Helpers.Collections.DisposableList<TDocumentEntry> currentDocuments)
            where TDocumentEntry : VsShell.Document.DocumentEntryBase {

            var match = currentDocuments
                .FirstOrDefault(d => string.Equals(
                    d.BaseViewModel.HierarchyItemEntry.BaseViewModel.FilePath,
                    hierarchyItemEntry.BaseViewModel.FilePath,
                    StringComparison.OrdinalIgnoreCase
                    ));

            return match;
        }


        private VsShell.Document.ExternalIncludeEntry CreateExternalInclude(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) {
            return new VsShell.Document.ExternalIncludeEntry(
                new VsShell.Document.ExternalIncludeMultiStateElement(
                    this.ProjectNode,
                    hierarchyItemEntry
                    ));
        }


        private VsShell.Document.SharedItemEntry CreateSharedItem(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) {
            return new VsShell.Document.SharedItemEntry(
                new VsShell.Document.SharedItemMultiStateElement(
                    this.ProjectNode,
                    hierarchyItemEntry
                    ));
        }


        private VsShell.Document.DocumentEntry CreateSource(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) {
            return new VsShell.Document.DocumentEntry(
                new VsShell.Document.DocumentMultiStateElement(
                    this.ProjectNode,
                    hierarchyItemEntry
                    ));
        }
    }




    public class UnloadedProject :
        ProjectCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public UnloadedProject(_Details.ProjectCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<UnloadedProject>";
        }
    }
}