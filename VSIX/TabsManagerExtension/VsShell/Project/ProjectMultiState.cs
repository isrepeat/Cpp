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


namespace TabsManagerExtension.VsShell.Project {
    public class ProjectMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            ProjectCommonState,
            LoadedProject,
            UnloadedProject> {
        public ProjectMultiStateElement(
            ProjectNode projectNode
            ) : base(new ProjectCommonState(projectNode)) {
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


        public LoadedProject(ProjectCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(this.ProjectNode.ProjectHierarchy.VsHierarchy);
            this.ShellProject = new ShellProject(dteProject);


            foreach (var externalInclude in this.CommonState.ExternalIncludes) {
                externalInclude.SwitchTo<Document.ExternalInclude>();
            }
            foreach (var sharedItem in this.CommonState.SharedItems) {
                sharedItem.SwitchTo<Document.SharedItem>();
            }
            foreach (var source in this.CommonState.Sources) {
                source.SwitchTo<Document.Document>();
            }

            var currentExternalIncludesHashItems = this.ExternalIncludes
                .OfType<Document.Document>()
                .Select(d => d.HierarchyItemObj)
                .OfType<Hierarchy.InvalidatedHierarchyItem>()
                .Select(h => h.)


            _projectHierarchyTracker = new ProjectHierarchyTracker(
                this.ProjectNode.ProjectHierarchy.VsHierarchy,
                this.ExternalIncludes.OfType<Document.InvalidatedDocument>().Select(d => d.HierarchyItemObj).ToHashSet(),
                _sharedItems.Select(d => d.HierarchyItem).ToHashSet(),
                _sources.Select(d => d.HierarchyItem).ToHashSet()
                );

            _projectHierarchyTracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;
            _projectHierarchyTracker.SharedItemsChanged += this.OnSharedItemsChanged;
            _projectHierarchyTracker.SourcesChanged += this.OnSourcesChanged;

            _projectHierarchyTracker.ExternalDependenciesChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SharedItemsChanged.InvokeForLastHandlerIfTriggered();
            _projectHierarchyTracker.SourcesChanged.InvokeForLastHandlerIfTriggered();


            _ = Task.Run(async () => {
                await Task.Delay(1000);
                this.CommonState.Sources.RemoveAndDispose(null);
                this.CommonState.Sources.Add(new Document.DocumentMultiStateElement(this.ProjectNode, null));
            });
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            foreach (var externalInclude in this.CommonState.ExternalIncludes) {
                externalInclude.SwitchTo<Document.InvalidatedDocument>();
            }
            foreach (var sharedItem in this.CommonState.SharedItems) {
                sharedItem.SwitchTo<Document.InvalidatedDocument>();
            }
            foreach (var source in this.CommonState.Sources) {
                source.SwitchTo<Document.InvalidatedDocument>();
            }
        }

        public override string ToString() {
            return $"LoadedProject({this.ProjectNode.UniqueName})";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);
        }

        protected void OpenWithProjectContext() {
            // ...
        }
    }




    public class UnloadedProject :
        ProjectCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public UnloadedProject(ProjectCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
            base.Dispose();
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