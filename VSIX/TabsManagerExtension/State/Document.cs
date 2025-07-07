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

        public override string ToString() {
            return $"TabItemBase(FullName='{this.FullName}')";
        }
    }

    public interface IActivatableTab {
        void Activate();
    }




    public class DocumentProjectReferenceInfo : Helpers.ObservableObject {
        public TabItemDocument TabItemDocument { get; private set; }
        public VsShell.Project.ProjectNode ProjectNode { get; private set; }

        public DocumentProjectReferenceInfo(
            TabItemDocument tabItemDocument,
            VsShell.Project.ProjectNode projectNode
            ) {
            this.TabItemDocument = tabItemDocument;
            this.ProjectNode = projectNode;
        }
    }


    public class TabItemDocument : TabItemBase, IActivatableTab {
        public VsShell.Document.ShellDocument ShellDocument { get; private set; }
        public VsShell.Project.ProjectNode ProjectNodeContext { get; set; }


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

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            solutionHierarchyAnalyzer.AnalyzeExternalIncludes();
            
            var externalIncludesProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByDocumentPath(this.FullName);

            var sharedItemsProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(this.FullName);

            var allProjectNodes = externalIncludesProjectNodes;
            //var allProjectNodes = externalIncludesProjectNodes
            //    .Concat(sharedItemsProjectNodes)
            //    .ToList();

            Helpers.Diagnostic.Logger.LogDebug($"[allProjectNodes]:");
            foreach (var projectNode in allProjectNodes) {
                Helpers.Diagnostic.Logger.LogDebug($"- {projectNode}");
            }

            if (allProjectNodes.Count < 2) {
                return; // Игнорируем только лишь ссылки на собсвтенные проекты.
            }

            var documentProjectReferences = allProjectNodes
                .Select(projectNode => new DocumentProjectReferenceInfo(this, projectNode));

            foreach (var documentProjectReference in documentProjectReferences) {
                this.ProjectReferenceList.Add(documentProjectReference);
            }
        }


        public override string ToString() {
            return $"TabItemDocument(FullName='{this.FullName}', ProjectCtx='{this.ProjectNodeContext}')";
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