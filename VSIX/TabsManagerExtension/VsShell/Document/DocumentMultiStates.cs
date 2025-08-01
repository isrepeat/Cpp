using System;
using System.Linq;
using System.ComponentModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Helpers.Attributes;
using System.Threading.Tasks;
using System.Collections.Generic;
using TabsManagerExtension.VsShell.Hierarchy;


namespace TabsManagerExtension.VsShell.Document {
    public abstract class DocumentMultiStateElementBase :
        Helpers.MultiState.MultiStateContainer<
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
            Project.ProjectCommonStateViewModel projectBaseViewModel,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(new _Details.DocumentCommonState(projectBaseViewModel, hierarchyItemEntry)) {
        }
    }


    public class SharedItemMultiStateElement : DocumentMultiStateElementBase {
        public SharedItemMultiStateElement(
            Project.ProjectCommonStateViewModel projectBaseViewModel,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(
                new _Details.DocumentCommonState(projectBaseViewModel, hierarchyItemEntry),
                commonState => new SharedItem(commonState),
                commonState => new InvalidatedDocument(commonState)
                ) {
        }
    }


    public class ExternalIncludeMultiStateElement : DocumentMultiStateElementBase {
        public ExternalIncludeMultiStateElement(
            Project.ProjectCommonStateViewModel projectBaseViewModel,
            Hierarchy.HierarchyItemEntry hierarchyItemEntry
            ) : base(
                new _Details.DocumentCommonState(projectBaseViewModel, hierarchyItemEntry),
                commonState => new ExternalInclude(commonState),
                commonState => new InvalidatedDocument(commonState)
                ) {
        }
    }



    public partial class Document :
        DocumentCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
        private bool _isOppenedWithProjectContext = false;

        public Document(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is Helpers.MultiState.UnknownMultiStateElement) {
                Helpers.ThrowableAssert.Require(base.CommonState.HierarchyItemEntry.IsRealHierarchy);
            }
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<Document> ({base.CommonState.ToStringCore()})";
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            base.OnCommonStatePropertyChanged(sender, e);
        }

        protected void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.ThrowableAssert.Require(!base.CommonState.IsDisposed);
            Helpers.ThrowableAssert.Require(base.CommonState.HierarchyItemEntry.MultiState.Current is Hierarchy.RealHierarchyItem);
            Helpers.ThrowableAssert.Require(base.ProjectBaseViewModel is Project.LoadedProject);

            var ownProject = base.ProjectBaseViewModel as Project.LoadedProject;

            // Сохраняем активный документ до всех действий.
            var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

            // Попытка найти первый cpp/h файл проекта,
            // чтобы открыть его и "переключить" контекст редактора на нужный проект.
            // Это нужно для того, чтобы при открытии внешнего include файла
            // Visual Studio знала, что контекстом открытия является именно этот проект.
            var includeDependencyAnalyzer = VsShell.Solution.Services.IncludeDependencyAnalyzerService.Instance;
            var allTransitiveIncludingFiles = includeDependencyAnalyzer
                .GetTransitiveFilesIncludersByIncludePath(base.HierarchyItemEntry.BaseViewModel.FilePath);

            var currentProjectTransitiveIncludingFiles = allTransitiveIncludingFiles
                .Where(sf => sf.LoadedProject.Equals(ownProject))
                .ToList();

            // Нужно открывать именно .cpp файл который реально включает наш include,
            // чтоб сработала смена контекста.
            var contextSwitchSourceFile = currentProjectTransitiveIncludingFiles
                .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp");

            if (contextSwitchSourceFile == null) {
                return;
            }

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var contextSwitchDocument = solutionHierarchyAnalyzer.SourcesRepresentationsTable
                .GetDocumentByProjectAndDocumentPath(base.ProjectBaseViewModel, contextSwitchSourceFile.FilePath);

            var contextSwitchDocumentProject = contextSwitchDocument?.BaseViewModel.ProjectBaseViewModel as Project.LoadedProject;
            if (Helpers.Assert.Require(contextSwitchDocumentProject != null).Failed) {
                return;
            }

            var contextSwitchDocumentHierarchyItem = contextSwitchDocument?.BaseViewModel.HierarchyItemEntry.BaseViewModel as Hierarchy.RealHierarchyItem;
            if (contextSwitchDocumentHierarchyItem == null) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            bool needCloseContextSwitchDocumentNode =
                !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentHierarchyItem.FilePath);

            int hr = VSConstants.S_OK;

            // Открываем файл в контексте проекта.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                ownProject.ProjectHierarchy.VsRealHierarchy,
                base.HierarchyItemEntry.BaseViewModel.ItemId);
            ErrorHandler.ThrowOnFailure(hr);

            // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
            // иначе IntelliSense не подхватит контекст.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                contextSwitchDocumentProject.ProjectHierarchy.VsRealHierarchy,
                contextSwitchDocumentHierarchyItem.ItemId);
            ErrorHandler.ThrowOnFailure(hr);
            

            // Закрываем временный файл переключения контекста.
            if (needCloseContextSwitchDocumentNode) {
                var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d =>
                        string.Equals(
                            d.FullName,
                            contextSwitchDocumentHierarchyItem.FilePath,
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
            // Получаем все проекты, которые знают про этот файл.
            // NOTE: Поскольку мы не используем текущий проект нам не нужно дожидаться пока
            //       у него подгрузяться externalIncluede чтоб использовать solutionHierarchyAnalyzer.
            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var sharedItemProjects = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(base.HierarchyItemEntry.BaseViewModel.FilePath);

            var externalIncludeProjects = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
                .GetProjectsByDocumentPath(base.HierarchyItemEntry.BaseViewModel.FilePath);

            // Собираем все проекты на выгрузку, кроме текущего 'base.ProjectBaseViewModel'
            // и оригинального 'IsSharedProject'.
            var sharedItemProjectGuidsToUnload = sharedItemProjects
                .Where(p => p.IsLoaded && !p.BaseViewModel.Equals(base.ProjectBaseViewModel) && !p.BaseViewModel.IsSharedProject)
                .Select(p => p.BaseViewModel.ProjectGuid)
                .ToList();

            var externalIncludeProjectGuidsToUnload = externalIncludeProjects
                .Where(p => p.IsLoaded && !p.BaseViewModel.Equals(base.ProjectBaseViewModel))
                .Select(p => p.BaseViewModel.ProjectGuid)
                .ToList();

            foreach (var projectGuid in sharedItemProjectGuidsToUnload) {
                Utils.VsHierarchyUtils.UnloadProject(projectGuid);
            }

            // Выгружаем все остальные связанные проекты (т.е. те которые содержат данный файл как External Dependencies)
            // только лишь когда на целевой проект ещще не было переключений контекста или когда
            // имеются другие sharedItems проект которые еще не выгружены. Это нужно чтобы
            // контекст целевого проекта установился наверняка.
            bool shouldReloadExternalIncludesProjects =
                sharedItemProjectGuidsToUnload.Count > 0 ||
                base.IsOppenedWithProjectContext == false;

            if (shouldReloadExternalIncludesProjects) {
                foreach (var projectGuid in externalIncludeProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.UnloadProject(projectGuid);
                }
            }

            base.OpenWithProjectContext();

            if (shouldReloadExternalIncludesProjects) {
                foreach (var projectGuid in externalIncludeProjectGuidsToUnload) {
                    Utils.VsHierarchyUtils.ReloadProject(projectGuid);
                }
            }
        }

        public override string ToString() {
            return $"<SharedItem> ({base.CommonState.ToStringCore()})";
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
            return $"<ExternalInclude> ({base.CommonState.ToStringCore()})";
        }
    }




    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        Helpers.MultiState.IMultiStateElement {

        private Document? _invalidatedPreviousDocument = null;
        private string _invalidatedPreviousDocumentFilePath;

        public InvalidatedDocument(_Details.DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
            if (e.PreviousState is Document document) {
                _invalidatedPreviousDocument = document;
                _invalidatedPreviousDocumentFilePath = document.HierarchyItemEntry.BaseViewModel.FilePath;
            }

            if (e.UpdatePackageObj is Project.ProjectCommonStateViewModel projectBaseViewModel) {
                base.CommonState.ProjectBaseViewModel = projectBaseViewModel;
            }

            base.CommonState.HierarchyItemEntry.MultiState.SwitchTo<Hierarchy.InvalidatedHierarchyItem>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
            Helpers.ThrowableAssert.Unexpected("Switching from InvalidatedDocument is not supported");
        }

        public void OpenWithProjectContext() {
            if (base.ProjectBaseViewModel is Project.UnloadedProject) {
                if (_invalidatedPreviousDocument is SharedItem) {
                    base.ProjectBaseViewModel.SharedItemsChanged.Add(
                        Helpers.Events.Action.Options.UnsubscribeAfterInvoked,
                        this.OnFreshedSharedItemsLoaded);
                }

                Utils.VsHierarchyUtils.ReloadProject(base.ProjectBaseViewModel.ProjectGuid);
                // NOTE: After reload this document will be disposed.
            }
        }

        public override string ToString() {
            return $"<InvalidatedDocument> ({base.CommonState.ToStringCore()})";
        }

        private void OnFreshedSharedItemsLoaded(IReadOnlyList<SharedItemEntry> freshSharedItemEntries) {
            Helpers.ThrowableAssert.Require(freshSharedItemEntries.All(d => d.MultiState.Current is SharedItem));
            Helpers.ThrowableAssert.Require(base.IsDisposed);

            // WORKAROUND:
            // Выполняем в следующей итерации очереди, т.к. OpenWithProjectContext использует
            // IncludeDependencyAnalyzerService, который в свою очередь подписан на OnProjectLoaded,
            // т.е. нам сначала необходимо дождаться пока обновиться IncludeDependencyAnalyzerService
            // для перезагруженного проекта, и только потом выполнять рутину по переключению контекста.
            VsixThreadHelper.RunOnUiThread(() => {
                var associatedFreshSharedItem = freshSharedItemEntries
                    .Select(sharedItemEntry => sharedItemEntry.MultiState.As<SharedItem>())
                    .FirstOrDefault(sharedItem => string.Equals(
                        sharedItem.HierarchyItemEntry.BaseViewModel.FilePath,
                        _invalidatedPreviousDocumentFilePath,
                        StringComparison.OrdinalIgnoreCase
                        ));

                if (associatedFreshSharedItem != null) {
                    associatedFreshSharedItem.OpenWithProjectContext();
                }
            });
        }
    }
}