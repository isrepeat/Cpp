using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Project {
    public abstract class ProjectMultiStateElementBase :
        Helpers.MultiState.MultiStateContainer<
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
            VsShell.Hierarchy.HierarchyItemEntry projectHierarchy
            ) : base(new _Details.ProjectCommonState(projectHierarchy)) {
        }
    }


    public class LoadedProject :
        ProjectCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public VsShell.Hierarchy.RealHierarchyItem ProjectHierarchy =>
            base.CommonState.ProjectHierarchy.MultiState.As<Hierarchy.RealHierarchyItem>();

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


        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.UpdatePackageObj is Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
                Helpers.ThrowableAssert.Require(hierarchyItemEntry.IsRealHierarchy);
                base.CommonState.ProjectHierarchy = hierarchyItemEntry;
            }

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectHierarchy.VsRealHierarchy);
            this.ShellProject = new ShellProject(dteProject);

            base.CommonState.ExternalIncludes.ClearAndDispose();
            base.CommonState.SharedItems.ClearAndDispose();
            base.CommonState.Sources.ClearAndDispose();

            //if (this.ProjectNode.UniqueName == "Editor\\Editor.vcxproj") {
            if (base.UniqueName == "Engine\\Engine.vcxproj") {
                int xx = 9;
            }

            _projectHierarchyTracker = new ProjectHierarchyTracker(this.ProjectHierarchy.VsRealHierarchy);

            _projectHierarchyTracker.ExternalDependenciesChanged.Add(this.OnExternalDependenciesChanged);
            _projectHierarchyTracker.SharedItemsChanged.Add(this.OnSharedItemsChanged);
            _projectHierarchyTracker.SourcesChanged.Add(this.OnSourcesChanged);

            _projectHierarchyTracker.ExternalDependenciesChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SharedItemsChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SourcesChanged.InvokeForLastHandlerIfTriggered();
        }


        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
            _projectHierarchyTracker.Dispose();
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public override string ToString() {
            return $"LoadedProject({base.UniqueName})";
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
            this.UpdateProjectHierarchyItems(base.CommonState.ExternalIncludes, e, this.CreateExternalInclude);
            base.CommonState.ExternalIncludesChanged.Invoke(base.CommonState.ExternalIncludes);
        }

        private void OnSharedItemsChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(base.CommonState.SharedItems, e, this.CreateSharedItem);
            base.CommonState.SharedItemsChanged.Invoke(base.CommonState.SharedItems);
        }

        private void OnSourcesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.UpdateProjectHierarchyItems(base.CommonState.Sources, e, this.CreateSource);
            base.CommonState.SourcesChanged.Invoke(base.CommonState.Sources);
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

            //if (this.UniqueName == "Editor\\Editor.vcxproj") {
            if (this.UniqueName == "Engine\\Engine.vcxproj") {
                int xx = 9;
            }

            foreach (var hierarchyItemEntry in e.Added) {
                var existDocumentEntry = this.FindDocumentByHierarchyItem(hierarchyItemEntry, currentDocuments);
                if (existDocumentEntry != null) {
                    Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Remove exist: {existDocumentEntry} [{base.UniqueName}]");
                    currentDocuments.RemoveAndDispose(existDocumentEntry);
                }

                var newDocumentEntry = createNewDocumenEntryFactory(hierarchyItemEntry);

                Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Add: {newDocumentEntry} [{base.UniqueName}]");
                currentDocuments.Add(newDocumentEntry);
            }

            foreach (var hierarchyItemEntry in e.Removed) {
                var existDocumentEntry = this.FindDocumentByHierarchyItem(hierarchyItemEntry, currentDocuments);

                Helpers.Diagnostic.Logger.LogDebug($"[UpdateProjectHierarchyItems] Remove: {existDocumentEntry} [{base.UniqueName}]");
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
            return VsShell.Document.ExternalIncludeEntry.CreateWithState<VsShell.Document.Document>(
                new VsShell.Document.ExternalIncludeMultiStateElement(
                    this,
                    hierarchyItemEntry
                    ));
        }


        private VsShell.Document.SharedItemEntry CreateSharedItem(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) {
            return VsShell.Document.SharedItemEntry.CreateWithState<VsShell.Document.Document>(
                new VsShell.Document.SharedItemMultiStateElement(
                    this,
                    hierarchyItemEntry
                    ));
        }


        private VsShell.Document.DocumentEntry CreateSource(
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) {
            return VsShell.Document.DocumentEntry.CreateWithState<VsShell.Document.Document>(
                new VsShell.Document.DocumentMultiStateElement(
                    this,
                    hierarchyItemEntry
                    ));
        }
    }




    public class UnloadedProject :
        ProjectCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        public UnloadedProject(_Details.ProjectCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.UpdatePackageObj is Hierarchy.HierarchyItemEntry hierarchyItemEntry) {
                Helpers.ThrowableAssert.Require(hierarchyItemEntry.IsStubHierarchy);
                base.CommonState.ProjectHierarchy = hierarchyItemEntry;
            }

            foreach (var externalInclude in base.CommonState.ExternalIncludes) {
                externalInclude.MultiState.SwitchTo<Document.InvalidatedDocument>(this);
            }
            foreach (var sharedItem in base.CommonState.SharedItems) {
                sharedItem.MultiState.SwitchTo<Document.InvalidatedDocument>(this);
            }
            foreach (var source in base.CommonState.Sources) {
                source.MultiState.SwitchTo<Document.InvalidatedDocument>(this);
            }
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<UnloadedProject>";
        }
    }
}