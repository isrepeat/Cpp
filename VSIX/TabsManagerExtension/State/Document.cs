using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;


namespace TabsManagerExtension.State.Document {

    public abstract class TabItemBase : Helpers.Collections.SelectableItemBase {

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



    public class DocumentProjectReferencesInfo : Helpers.ObservableObject {
        private ObservableCollection<RefEntry> _references = new();
        public ObservableCollection<RefEntry> References => _references;


        public bool _hasUnloadedProjects = false;
        public bool HasUnloadedProjects {
            get => _hasUnloadedProjects;
            private set {
                if (_hasUnloadedProjects != value) {
                    _hasUnloadedProjects = value;
                    OnPropertyChanged();
                }
            }
        }


        public DocumentProjectReferencesInfo() {
            _references.CollectionChanged += this.OnCollectionChanged;
        }


        private void OnCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e) {
            if (e.Action == NotifyCollectionChangedAction.Reset) {
                foreach (var oldItem in _references) {
                    oldItem.DocumentNode.ProjectNode.PropertyChanged -= this.OnProjectNodePropertyChanged;
                }
            }

            if (e.OldItems != null) {
                foreach (RefEntry oldItem in e.OldItems) {
                    oldItem.DocumentNode.ProjectNode.PropertyChanged -= this.OnProjectNodePropertyChanged;
                }
            }
            if (e.NewItems != null) {
                foreach (RefEntry newItem in e.NewItems) {
                    newItem.DocumentNode.ProjectNode.PropertyChanged += this.OnProjectNodePropertyChanged;
                }
            }

            this.UpdateHasUnloadedProjectsProperty();
        }


        private void OnProjectNodePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName == nameof(VsShell.Project.ProjectNode.IsLoaded)) {
                this.UpdateHasUnloadedProjectsProperty();
            }
        }

        private void UpdateHasUnloadedProjectsProperty() {
            bool hasUnloadedProjects = _references.Any(refEntry => !refEntry.DocumentNode.ProjectNode.IsLoaded);
            this.HasUnloadedProjects = hasUnloadedProjects;
            Helpers.Diagnostic.Logger.LogDebug($"[UpdateHasUnloadedProjectsProperty] this.HasUnloadedProjects = {this.HasUnloadedProjects}");
        }


        public class RefEntry : Helpers.ObservableObject {
            public VsShell.Document.DocumentNode DocumentNode { get; private set; }

            public RefEntry(VsShell.Document.DocumentNode documentNode) {
                this.DocumentNode = documentNode;
            }
        }
    }



    public class TabItemDocument : TabItemBase, IActivatableTab {
        public VsShell.Document.ShellDocument ShellDocument { get; private set; }
        public DocumentProjectReferencesInfo DocumentProjectReferencesInfo { get; } = new();
        public VsShell.Project.ProjectNode ProjectNodeContext { get; set; }


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
            this.DocumentProjectReferencesInfo.References.Clear();

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

            // Получаем все проекты, которые знают про этот файл.
            var externalIncludesSolutionProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByDocumentPath(this.FullName);

            var sharedItemsSolutionProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(this.FullName);

            var allSolutionProjectNodes = externalIncludesSolutionProjectNodes
                .Concat(sharedItemsSolutionProjectNodes)
                .ToList();


            if (allSolutionProjectNodes.Count < 2) {
                return; // Игнорируем только лишь ссылки на собсвтенные проекты.
            }

            var documentNodes = new List<VsShell.Document.DocumentNode>();

            foreach (var projectNode in allSolutionProjectNodes) {
                var externalInclude = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                    .GetDocumentByProjectAndDocumentPath(projectNode, this.FullName);
                
                if (externalInclude != null) {
                    documentNodes.Add(externalInclude);
                }
            }


            foreach (var projectNode in allSolutionProjectNodes) {
                var sharedItemNode = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                    .GetDocumentByProjectAndDocumentPath(projectNode, this.FullName);

                if (sharedItemNode != null) {
                    documentNodes.Add(sharedItemNode);
                }
            }

            documentNodes = documentNodes
                .OrderBy(d => d.ProjectNode.UniqueName, StringComparer.OrdinalIgnoreCase)
                .ToList();

            foreach (var documentNode in documentNodes) {
                this.DocumentProjectReferencesInfo.References
                    .Add(new DocumentProjectReferencesInfo.RefEntry(documentNode));
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





    public abstract class TabItemsGroupBase : Helpers.ObservableObject, Helpers.Collections.ISelectableGroup<TabItemBase> {
        public string GroupName { get; }

        public Helpers.Collections.SortedObservableCollection<TabItemBase> Items { get; }

        public Helpers.IMetadata Metadata { get; } = new Helpers.FlaggableMetadata();

        protected TabItemsGroupBase(string groupName) {
            this.GroupName = groupName;

            var defaultTabItemBaseComparer = Comparer<TabItemBase>.Create((a, b) =>
                string.Compare(a.Caption, b.Caption, StringComparison.OrdinalIgnoreCase));

            this.Items = new Helpers.Collections.SortedObservableCollection<TabItemBase>(defaultTabItemBaseComparer);
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