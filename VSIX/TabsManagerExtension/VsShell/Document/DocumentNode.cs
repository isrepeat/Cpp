using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentNode :
        Helpers.ObservableObject,
        IDisposable {

        public uint ItemId { get; private set; }
        public string FilePath { get; }
        public VsShell.Project.ProjectNode ProjectNode { get; }


        private bool _isEnabled = true;
        public bool IsEnabled {
            get => _isEnabled;
            set {
                if (_isEnabled != value) {
                    _isEnabled = value;
                    OnPropertyChanged();
                }
            }
        }


        private bool _disposed = false;
        public bool IsDisposed {
            get => _disposed;
        }


        public DocumentNode(uint itemId, string filePath, VsShell.Project.ProjectNode projectNode) {
            this.ItemId = itemId;
            this.FilePath = filePath;
            this.ProjectNode = projectNode;
        }


        public void Dispose() {
            if (_disposed) {
                return;
            }

            this.ItemId = VSConstants.VSITEMID_NIL;
            this.IsEnabled = false;

            OnPropertyChanged(nameof(this.IsDisposed));
            _disposed = true;
        }

        public void Update(Utils.VsHierarchyUtils.HierarchyItem hierarchyItem) {
            this.ItemId = hierarchyItem.ItemId;
        }

        public void TryRefreshItemId() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (this.IsDisposed) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            //Helpers.Diagnostic.Logger.LogDebug($"TryRefreshItemId for '{this.FilePath}'");

            var hierarchy = this.ProjectNode.ProjectHierarchy.VsHierarchy;
            if (hierarchy != null) {
                var hr = hierarchy.ParseCanonicalName(this.FilePath, out var newItemId);

                if (ErrorHandler.Succeeded(hr) && newItemId != VSConstants.VSITEMID_NIL) {
                    if (newItemId != this.ItemId) {
                        Helpers.Diagnostic.Logger.LogDebug($"ItemId changed from {this.ItemId} to {newItemId}'");
                        this.ItemId = newItemId;
                    }
                }
                else {
                    // можно добавить дополнительный поиск через Walk, если ParseCanonicalName не нашел
                }
            }
        }


        public override bool Equals(object? obj) {
            if (obj is not DocumentNode other) {
                return false;
            }

            return
                this.ItemId == other.ItemId &&
                this.ProjectNode.ProjectGuid == other.ProjectNode.ProjectGuid &&
                StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath);
        }

        public override int GetHashCode() {
            unchecked {
                int hash = 17;

                hash = hash * 31 + this.ItemId.GetHashCode();
                hash = hash * 31 + this.ProjectNode.ProjectGuid.GetHashCode();
                hash = hash * 31 + (this.FilePath != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath)
                    : 0);

                return hash;
            }
        }



        protected void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (this.IsDisposed) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            if (!this.ProjectNode.IsLoaded) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            // Сохраняем активный документ до всех действий.
            var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

            // Попытка найти первый cpp/h файл проекта,
            // чтобы открыть его и "переключить" контекст редактора на нужный проект.
            // Это нужно для того, чтобы при открытии внешнего include файла
            // Visual Studio знала, что контекстом открытия является именно этот проект.
            var includeDependencyAnalyzer = VsShell.Solution.Services.IncludeDependencyAnalyzerService.Instance;
            var allTransitiveIncludingFiles = includeDependencyAnalyzer
                .GetTransitiveFilesIncludersByIncludePath(this.FilePath);

            var currentProjectTransitiveIncludingFiles = allTransitiveIncludingFiles
                .Where(sf => sf.ProjectNode.Equals(this.ProjectNode))
                .ToList();

            // Нужно открывать именно .cpp файл который реально включает наш include,
            // чтоб сработала смена контекста.
            var contextSwitchFile = currentProjectTransitiveIncludingFiles
                .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp")
                ?.FilePath;

            if (contextSwitchFile == null) {
                return;
            }

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var contextSwitchDocumentNode = solutionHierarchyAnalyzer.SourcesRepresentationsTable
                .GetDocumentByProjectAndDocumentPath(this.ProjectNode, contextSwitchFile);

            if (contextSwitchDocumentNode == null) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            bool needCloseContextSwitchDocumentNode =
                !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentNode.FilePath);

            int hr = VSConstants.S_OK;

            // Открываем файл в контексте проекта.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                this.ProjectNode.ProjectHierarchy.VsHierarchy,
                this.ItemId);
            ErrorHandler.ThrowOnFailure(hr);

            // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
            // иначе IntelliSense не подхватит контекст.
            if (contextSwitchDocumentNode != null) {

                Utils.VsHierarchyUtils.LogSolutionHierarchy();

                hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                    contextSwitchDocumentNode.ProjectNode.ProjectHierarchy.VsHierarchy,
                    contextSwitchDocumentNode.ItemId);
                ErrorHandler.ThrowOnFailure(hr);
            }

            // Закрываем временный файл переключения контекста.
            if (needCloseContextSwitchDocumentNode) {
                var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d => string.Equals(d.FullName, contextSwitchDocumentNode.FilePath, StringComparison.OrdinalIgnoreCase));

                doc?.Close(EnvDTE.vsSaveChanges.vsSaveChangesNo);
            }

            // Возвращаем активным предыдущий документ.
            VsixThreadHelper.RunOnUiThread(async () => {
                await Task.Delay(20);
                activeDocumentBefore?.Activate();
            });
        }


        protected string ToStringCore() {
            return $"FilePath='{this.FilePath}', Project='{this.ProjectNode.UniqueName}', ItemId={this.ItemId}";
        }

        public override string ToString() {
            return $"DocumentNode({this.ToStringCore()})";
        }
    }



    public sealed class SharedItemNode : DocumentNode {
        private IVsHierarchy? _sharedProjectHierarchy;

        public SharedItemNode(
            uint itemId,
            string filePath,
            VsShell.Project.ProjectNode projectNode,
            IVsHierarchy? sharedProjectHierarchy)
            : base(itemId, filePath, projectNode) {

            _sharedProjectHierarchy = sharedProjectHierarchy;

            //var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            //var sharedProjectNode = solutionHierarchyAnalyzer.SolutionProjects
            //    .FirstOrDefault(p => Equals(p.ProjectHierarchy.VsHierarchy, sharedProjectHierarchy));
        }


        public new void OpenWithProjectContext() {
            if (!this.ProjectNode.IsLoaded) {
                Utils.VsHierarchyUtils.ReloadProject(this.ProjectNode.ProjectGuid);
            }

            // Получаем все sharedItems проекты, которые знают про этот файл.
            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var sharedItemsSolutionProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(this.FilePath);

            var externalIncludesSolutionProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByDocumentPath(this.FilePath);

            // Собираем все проекты на выгрузку, кроме текущего 'this.ProjectNode'
            // и оригинального 'IsSharedProject'.
            var sharedItemsProjectGuidsToUnload = sharedItemsSolutionProjectNodes
                .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode) && !p.IsSharedProject)
                .Select(p => p.ProjectGuid)
                .ToList();

            var externalIncludesProjectGuidsToUnload = externalIncludesSolutionProjectNodes
                .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode))
                .Select(p => p.ProjectGuid)
                .ToList();


            foreach (var projectGuid in sharedItemsProjectGuidsToUnload) {
                Utils.VsHierarchyUtils.UnloadProject(projectGuid);
            }

            // Выгружаем все остальные связанные проекты только лишь когда имеются 
            // другие невыгруженные sharedItems проекты. Это нужно чтобы контекст
            // целевого проекта установился наверняка.
            //if (sharedItemsProjectGuidsToUnload.Count > 0) {
                foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.UnloadProject(projectGuid);
                }
            //}

            base.OpenWithProjectContext();

            //if (sharedItemsProjectGuidsToUnload.Count > 0) {
                foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.ReloadProject(projectGuid);
                }
            //}
        }


        public override string ToString() {
            return $"SharedItemNode({base.ToStringCore()})";
        }
    }



    public sealed class ExternalInclude : DocumentNode {
        public ExternalInclude(
            uint itemId,
            string filePath,
            VsShell.Project.ProjectNode projectNode)
            : base(itemId, filePath, projectNode) {
        }


        public new void OpenWithProjectContext() {
            base.OpenWithProjectContext();
        }


        public override string ToString() {
            return $"ExternalInclude({base.ToStringCore()})";
        }
    }
}