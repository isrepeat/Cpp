using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;


namespace TabsManagerExtension.State.Document {

    public abstract class TabItemBase : Helpers.SelectableItemBase {

        private string _caption;
        public string Caption {
            get => _caption;
            set {
                if (_caption != value) {
                    _caption = value;
                    OnPropertyChanged();
                }
            }
        }

        private string _fullName;
        public string FullName {
            get => _fullName;
            set {
                if (_fullName != value) {
                    _fullName = value;
                    OnPropertyChanged();
                }
            }
        }

        private bool _isPreviewTab = false;
        public bool IsPreviewTab {
            get => _isPreviewTab;
            set {
                if (_isPreviewTab != value) {
                    _isPreviewTab = value;
                    OnPropertyChanged();
                }
            }
        }

        private bool _isPinnedTab = false;
        public bool IsPinnedTab {
            get => _isPinnedTab;
            set {
                if (_isPinnedTab != value) {
                    _isPinnedTab = value;
                    OnPropertyChanged();
                }
            }
        }
    }

    public interface IActivatableTab {
        void Activate();
    }



    public class TabItemProject : TabItemBase {
        public VsShell.Project.ShellProject ShellProject { get; private set; }
        
        public TabItemProject(VsShell.Project.ShellProject shellProject) {
            base.Caption = shellProject.Project.Name;
            base.FullName = shellProject.Project.FullName;
            this.ShellProject = shellProject;
        }

        public TabItemProject(EnvDTE.Project project)
            : this(new VsShell.Project.ShellProject(project)) {
        }

        public override bool Equals(object? obj) {
           return obj is TabItemProject other &&
                StringComparer.OrdinalIgnoreCase.Equals(this.ShellProject.Project?.UniqueName, other.ShellProject.Project?.UniqueName);
        }

        public override int GetHashCode() {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(this.ShellProject.Project?.UniqueName ?? string.Empty);
        }

        public override string ToString() => base.FullName;
    }




    public class DocumentProjectReferenceInfo : Helpers.ObservableObject {
        public TabItemProject TabItemProject { get; private set; }
        public TabItemDocument TabItemDocument { get; private set; }

        public DocumentProjectReferenceInfo(
            TabItemProject tabItemProject,
            TabItemDocument tabItemDocument
            ) {
            this.TabItemProject = tabItemProject;
            this.TabItemDocument = tabItemDocument;
        }
    }


    public class TabItemDocument : TabItemBase, IActivatableTab {
        public VsShell.Document.ShellDocument ShellDocument { get; private set; }


        private ObservableCollection<DocumentProjectReferenceInfo> _projectReferenceList = new ObservableCollection<DocumentProjectReferenceInfo>();
        public ObservableCollection<DocumentProjectReferenceInfo> ProjectReferenceList {
            get => _projectReferenceList;
            set {
                _projectReferenceList = value;
                OnPropertyChanged();
            }
        }

        public TabItemDocument(VsShell.Document.ShellDocument shellDocument) {
            base.Caption = shellDocument.Document.Name;
            base.FullName = shellDocument.Document.FullName;
            this.ShellDocument = shellDocument;
        }

        public TabItemDocument(EnvDTE.Document document)
            : this(new VsShell.Document.ShellDocument(document)) {
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                this.ShellDocument.Document?.Activate();
            }
            catch (COMException ex) {
                Helpers.Diagnostic.Logger.LogWarning($"Failed to activate document '{this.Caption}': {ex.Message}");
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"Unexpected error activating document '{this.Caption}': {ex.Message}");
            }
        }


        public void UpdateProjectReferenceList() {
            this.ProjectReferenceList.Clear();

            var ext = System.IO.Path.GetExtension(this.FullName);
            switch (ext) {
                case ".h":
                case ".hpp":
                    break;

                default:
                    return;
            }

            var externalDependenciesAnalyzer = VsShell.Solution.Services.ExternalDependenciesAnalyzerService.Instance;
            externalDependenciesAnalyzer.Analyze();
            //if (!externalDependenciesAnalyzer.IsReady()) {
            //    return;
            //}

            var projectNodes = externalDependenciesAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByExternalIncludePath(this.FullName);

            var documentProjectReferences = projectNodes
                .Select(projectNode => new DocumentProjectReferenceInfo(
                    new TabItemProject(projectNode),
                    this)
                );

            foreach (var documentProjectReference in documentProjectReferences) {
                this.ProjectReferenceList.Add(documentProjectReference);
            }
        }
    }


    public class TabItemWindow : TabItemBase, IActivatableTab {
        public VsShell.Document.ShellWindow ShellWindow { get; private set; }

        public string WindowId { get; private set; }

        public TabItemWindow(VsShell.Document.ShellWindow shellWindow) {
            base.Caption = shellWindow.Window.Caption;
            base.FullName = shellWindow.Window.Caption;
            this.ShellWindow = shellWindow;
            this.WindowId = shellWindow.GetWindowId();
        }

        public TabItemWindow(EnvDTE.Window window)
            : this(new VsShell.Document.ShellWindow(window)) {
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                this.ShellWindow.Window?.Activate();
            }
            catch (COMException ex) {
                Helpers.Diagnostic.Logger.LogWarning($"Failed to activate window '{this.Caption}': {ex.Message}");
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"Unexpected error activating window '{this.Caption}': {ex.Message}");
            }
        }
    }





    public abstract class TabItemsGroupBase : Helpers.ObservableObject, Helpers.ISelectableGroup<TabItemBase> {
        public string GroupName { get; }

        public Helpers.SortedObservableCollection<TabItemBase> Items { get; }

        public Helpers.IMetadata Metadata { get; } = new Helpers.FlaggableMetadata();

        protected TabItemsGroupBase(string groupName) {
            this.GroupName = groupName;

            var defaultTabItemBaseComparer = Comparer<TabItemBase>.Create((a, b) =>
                string.Compare(a.Caption, b.Caption, StringComparison.OrdinalIgnoreCase));

            this.Items = new Helpers.SortedObservableCollection<TabItemBase>(defaultTabItemBaseComparer);
            this.Items.CollectionChanged += (s, e) => {
                OnPropertyChanged(nameof(this.GroupName));
            };
        }
    }


    public class TabItemsPreviewGroup : TabItemsGroupBase {
        public TabItemsPreviewGroup() : base("__Preview__") {
            this.Metadata.SetFlag("IsPreviewGroup", true);
        }
    }

    public class TabItemsPinnedGroup : TabItemsGroupBase {
        public TabItemsPinnedGroup(string groupName) : base(groupName) {
            this.Metadata.SetFlag("IsPinnedGroup", true);
        }
    }

    public class TabItemsDefaultGroup : TabItemsGroupBase {
        public TabItemsDefaultGroup(string groupName) : base(groupName) {
        }
    }

    public class SeparatorTabItemsGroup : TabItemsGroupBase {
        public string Key { get; }
        public SeparatorTabItemsGroup(string key) : base(string.Empty) {
            this.Key = key;
            this.Metadata.SetFlag("IsSeparator", true);
        }
    }
}