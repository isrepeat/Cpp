using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;
using EnvDTE;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentNode {
        public uint ItemId { get; }
        public string FilePath { get; }
        public VsShell.Project.SolutionProjectNode SolutionProjectNode { get; }

        public DocumentNode(uint itemId, string filePath, VsShell.Project.SolutionProjectNode solutionProjectNode) {
            this.ItemId = itemId;
            this.FilePath = filePath;
            this.SolutionProjectNode = solutionProjectNode;
        }

        public override bool Equals(object? obj) {
            if (obj is not DocumentNode other) {
                return false;
            }

            return
                this.SolutionProjectNode.ProjectGuid == other.SolutionProjectNode.ProjectGuid &&
                StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath);
        }

        public override int GetHashCode() {
            unchecked {
                int hash = 17;

                hash = hash * 31 + this.SolutionProjectNode.ProjectGuid.GetHashCode();
                hash = hash * 31 + (this.FilePath != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath)
                    : 0);

                return hash;
            }
        }


        protected string ToStringCore() {
            return $"FilePath='{this.FilePath}', Project='{this.SolutionProjectNode.UniqueName}', ItemId={this.ItemId}";
        }

        public override string ToString() {
            return $"DocumentNode({this.ToStringCore()})";
        }
    }



    public sealed class SharedItemNode : DocumentNode {
        public VsShell.Project.SolutionProjectNode SharedSolutionProjectNode { get; private set; }
        
        public SharedItemNode(
            uint itemId,
            string filePath,
            VsShell.Project.SolutionProjectNode solutionProjectNode,
            VsShell.Project.SolutionProjectNode sharedSolutionProjectNode)
            : base(itemId, filePath, solutionProjectNode) {

            this.SharedSolutionProjectNode = sharedSolutionProjectNode;
        }


        public void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var loadedProjectNode = this.SolutionProjectNode.ProjectNodeObj as VsShell.Project.LoadedProjectNode;
            if (loadedProjectNode == null) {
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
                .Where(sf => sf.SolutionProjectNode.Equals(this.SolutionProjectNode))
                .ToList();

            // Нужно открывать именно .cpp файл который реально включает наш include,
            // чтоб сработала смена контекста.
            string contextSwitchFile = currentProjectTransitiveIncludingFiles
                .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp")
                ?.FilePath;

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var contextSwitchDocumentNode = solutionHierarchyAnalyzer.SourcesRepresentationsTable
                .GetDocumentByProjectAndDocumentPath(this.SolutionProjectNode, contextSwitchFile);

            if (contextSwitchDocumentNode == null) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            bool needCloseContextSwitchDocumentNode =
                !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentNode.FilePath);

            int hr = VSConstants.S_OK;

            // Открываем файл в контексте проекта.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                this.SolutionProjectNode.ProjectHierarchy.VsHierarchy,
                this.ItemId);
            ErrorHandler.ThrowOnFailure(hr);

            // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
            // иначе IntelliSense не подхватит контекст.
            if (contextSwitchDocumentNode != null) {
                hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                    contextSwitchDocumentNode.SolutionProjectNode.ProjectHierarchy.VsHierarchy,
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


            // Выгрузка связанных проектов.
            // Получаем все sharedItems проекты, которые знают про этот файл.
            var sharedItemsSolutionProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
                .GetProjectsByDocumentPath(this.FilePath);

            var projectGuidsToUnload = sharedItemsSolutionProjectNodes
                .Where(spn => !spn.IsSharedProject && !spn.Equals(this.SolutionProjectNode))
                .Select(spn => spn.ProjectGuid)
                .ToList();

            foreach (var projectGuid in projectGuidsToUnload) {
                Utils.VsHierarchyUtils.UnloadProject(projectGuid);
            }
        }


        public override string ToString() {
            return $"SharedItemNode({base.ToStringCore()})";
        }
    }



    public sealed class ExternalInclude : DocumentNode {
        public ExternalInclude(
            uint itemId,
            string filePath,
            VsShell.Project.SolutionProjectNode solutionProjectNode)
            : base(itemId, filePath, solutionProjectNode) {
        }


        public void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var loadedProjectNode = this.SolutionProjectNode.ProjectNodeObj as VsShell.Project.LoadedProjectNode;
            if (loadedProjectNode == null) {
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
                .Where(sf => sf.SolutionProjectNode.Equals(this.SolutionProjectNode))
                .ToList();

            // Нужно открывать именно .cpp файл который реально включает наш include,
            // чтоб сработала смена контекста.
            string contextSwitchFile = currentProjectTransitiveIncludingFiles
                .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp")
                ?.FilePath;

            var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
            var contextSwitchDocumentNode = solutionHierarchyAnalyzer.SourcesRepresentationsTable
                .GetDocumentByProjectAndDocumentPath(this.SolutionProjectNode, contextSwitchFile);

            if (contextSwitchDocumentNode == null) {
                System.Diagnostics.Debugger.Break();
                return;
            }

            bool needCloseContextSwitchDocumentNode = 
                !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentNode.FilePath);

            int hr = VSConstants.S_OK;

            // Открываем файл в контексте проекта.
            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                this.SolutionProjectNode.ProjectHierarchy.VsHierarchy,
                this.ItemId);
            ErrorHandler.ThrowOnFailure(hr);

            // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
            // иначе IntelliSense не подхватит контекст.
            if (contextSwitchDocumentNode != null) {
                hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
                    contextSwitchDocumentNode.SolutionProjectNode.ProjectHierarchy.VsHierarchy,
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


        public override string ToString() {
            return $"ExternalInclude({base.ToStringCore()})";
        }
    }
}