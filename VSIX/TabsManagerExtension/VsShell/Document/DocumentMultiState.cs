using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using System.Threading.Tasks;


namespace TabsManagerExtension.VsShell.Document {
    public abstract class DocumentMultiStateElementBase :
        HelpersV4.Collections.MultiStateContainer<
            _Details.DocumentCommonState,
            Document,
            InvalidatedDocument> {

        protected DocumentMultiStateElementBase(_Details.DocumentCommonState commonState)
            : base(commonState) {
        }

        protected DocumentMultiStateElementBase(
            _Details.DocumentCommonState commonState,
            Func<_Details.DocumentCommonState, Document> factoryA,
            Func<_Details.DocumentCommonState, InvalidatedDocument> factoryB)
            : base(commonState, factoryA, factoryB) {
        }
    }



    public class DocumentMultiStateElement : DocumentMultiStateElementBase {
        public DocumentMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(new _Details.DocumentCommonState(projectNode, hierarchyItemEntry)) {
        }
    }


    public class SharedItemMultiStateElement : DocumentMultiStateElementBase {
        public SharedItemMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(
                new _Details.DocumentCommonState(projectNode, hierarchyItemEntry),
                commonState => new SharedItem(commonState),
                commonState => new InvalidatedDocument(commonState)
                ) {
        }
    }


    public class ExternalIncludeMultiStateElement : DocumentMultiStateElementBase {
        public ExternalIncludeMultiStateElement(
            VsShell.Project.ProjectNode projectNode,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(
                new _Details.DocumentCommonState(projectNode, hierarchyItemEntry),
                commonState => new ExternalInclude(commonState),
                commonState => new InvalidatedDocument(commonState)
                ) {
        }
    }



    public partial class Document :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private bool _isOppenedWithProjectContext = false;

        public Document(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is InvalidatedDocument) {
                throw new InvalidOperationException("Switching state from InvalidatedDocument is not supported");
            }
            this.CommonState.HierarchyItemEntry.MultiState.SwitchTo<Hierarchy.HierarchyItem>();
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                this.CommonState.HierarchyItemEntry.MultiState.SwitchTo<Hierarchy.InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return $"Document({base.CommonState.ToStringCore()})";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);
        }

        protected void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (this.CommonState.IsDisposed) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            if (!this.ProjectNode.IsLoaded) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            if (this.CommonState.HierarchyItemEntry.MultiState.Current is Hierarchy.InvalidatedHierarchyItem) {
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
                .GetTransitiveFilesIncludersByIncludePath(this.HierarchyItemEntry.BaseViewModel.FilePath);

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
                !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentNode.BaseViewModel.HierarchyItemEntry.BaseViewModel.FilePath);

            int hr = VSConstants.S_OK;

            // Открываем файл в контексте проекта.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                this.ProjectNode.ProjectHierarchy.VsHierarchy,
                this.HierarchyItemEntry.BaseViewModel.ItemId);
            ErrorHandler.ThrowOnFailure(hr);

            // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
            // иначе IntelliSense не подхватит контекст.
            if (contextSwitchDocumentNode != null) {
                hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                    contextSwitchDocumentNode.BaseViewModel.ProjectNode.ProjectHierarchy.VsHierarchy,
                    contextSwitchDocumentNode.BaseViewModel.HierarchyItemEntry.BaseViewModel.ItemId);
                ErrorHandler.ThrowOnFailure(hr);
            }

            // Закрываем временный файл переключения контекста.
            if (needCloseContextSwitchDocumentNode) {
                var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d =>
                        string.Equals(
                            d.FullName,
                            contextSwitchDocumentNode.BaseViewModel.HierarchyItemEntry.BaseViewModel.FilePath,
                            StringComparison.OrdinalIgnoreCase)
                        );

                doc?.Close(EnvDTE.vsSaveChanges.vsSaveChangesNo);
            }

            this.IsOppenedWithProjectContext = true;

            // Возвращаем активным предыдущий документ.
            VsixThreadHelper.RunOnUiThread(async () => {
                await Task.Delay(20);
                activeDocumentBefore?.Activate();
            });
        }
    }



    //
    // SharedItem
    //
    public sealed class SharedItem : Document {
        public SharedItem(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public new void OpenWithProjectContext() {
            if (!this.ProjectNode.IsLoaded) {
                Utils.VsHierarchyUtils.ReloadProject(this.ProjectNode.ProjectGuid);
            }

            // Получаем все sharedItems проекты, которые знают про этот файл.
            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var sharedItemsProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(this.HierarchyItemEntry.BaseViewModel.FilePath);

            var externalIncludesProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByDocumentPath(this.HierarchyItemEntry.BaseViewModel.FilePath);

            // Собираем все проекты на выгрузку, кроме текущего 'this.ProjectNode'
            // и оригинального 'IsSharedProject'.
            var sharedItemsProjectGuidsToUnload = sharedItemsProjectNodes
                .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode) && !p.IsSharedProject)
                .Select(p => p.ProjectGuid)
                .ToList();

            var externalIncludesProjectGuidsToUnload = externalIncludesProjectNodes
                .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode))
                .Select(p => p.ProjectGuid)
                .ToList();

            foreach (var projectGuid in sharedItemsProjectGuidsToUnload) {
                Utils.VsHierarchyUtils.UnloadProject(projectGuid);
            }

            // Выгружаем все остальные связанные проекты (т.е. те которые содержат данный файл как External Dependencies)
            // только лишь когда на целевой проект ещще не было переключений контекста или когда
            // имеются другие sharedItems проект которые еще не выгружены. Это нужно чтобы
            // контекст целевого проекта установился наверняка.
            bool shouldReloadExternalIncludesProjects =
                sharedItemsProjectGuidsToUnload.Count > 0 ||
                base.IsOppenedWithProjectContext == false;

            if (shouldReloadExternalIncludesProjects) {
                foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.UnloadProject(projectGuid);
                }
            }

            base.OpenWithProjectContext();

            if (shouldReloadExternalIncludesProjects) {
                foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.ReloadProject(projectGuid);
                }
            }
        }

        public override string ToString() {
            return $"SharedItem({base.CommonState.ToStringCore()})";
        }
    }



    //
    // ExternalInclude
    //
    public sealed class ExternalInclude : Document {
        public ExternalInclude(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public new void OpenWithProjectContext() {
            base.OpenWithProjectContext();
        }

        public override string ToString() {
            return $"ExternalInclude({base.CommonState.ToStringCore()})";
        }
    }




    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedDocument(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument> {this.CommonState.ToString()}";
        }
    }
}