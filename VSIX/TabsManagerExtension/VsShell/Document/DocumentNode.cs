using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using TabsManagerExtension.VsShell.Utils;


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

            var loadedSharedProjectNode = this.SharedSolutionProjectNode.ProjectNodeObj as VsShell.Project.LoadedProjectNode;
            if (loadedSharedProjectNode == null) {
                System.Diagnostics.Debugger.Break();
                return;
            }


            // Сохраняем активный документ до всех действий
            var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

            // Проверяем, совпадает ли уже активный документ с нашим файлом
            bool wasAlreadyActive =
                activeDocumentBefore != null &&
                string.Equals(activeDocumentBefore.FullName, this.FilePath, StringComparison.OrdinalIgnoreCase);


            // Попытка найти первый cpp/h файл проекта,
            // чтобы открыть его и "переключить" контекст редактора на нужный проект.
            // Это нужно для того, чтобы при открытии внешнего include файла
            // Visual Studio знала, что контекстом открытия является именно этот проект.
            if (loadedProjectNode.Sources.Count == 0) {
                System.Diagnostics.Debugger.Break();
            }
            string contextSwitchFile = loadedProjectNode.Sources.FirstOrDefault()?.FilePath;

            bool needCloseContextSwitchFile = false;
            if (!string.IsNullOrEmpty(contextSwitchFile)) {
                bool alreadyOpen = Utils.EnvDteUtils.IsDocumentOpen(contextSwitchFile);
                if (!alreadyOpen) {
                    PackageServices.Dte2.ItemOperations.OpenFile(contextSwitchFile);
                    needCloseContextSwitchFile = true;
                }
            }

            // Используем ExecCommand для эмуляции DoubleClick в Solution Explorer,
            // чтобы Visual Studio открыла файл так, как если бы пользователь дважды кликнул
            // по нему именно в контексте этого проекта в папке External Dependencies.
            if (this.SolutionProjectNode.ProjectHierarchy.Hierarchy is IVsUIHierarchy uiHierarchy) {
                Guid cmdGroup = VSConstants.CMDSETID.UIHierarchyWindowCommandSet_guid;
                const uint cmdId = (uint)VSConstants.VsUIHierarchyWindowCmdIds.UIHWCMDID_DoubleClick;

                int hr = uiHierarchy.ExecCommand(
                    this.ItemId,
                    ref cmdGroup,
                    cmdId,
                    0,
                    IntPtr.Zero,
                    IntPtr.Zero);

                if (ErrorHandler.Succeeded(hr)) {
                    Helpers.Diagnostic.Logger.LogDebug($"[SharedItemNode] Opened '{this.FilePath}' in project '{this.SolutionProjectNode.UniqueName}'");
                }
                else {
                    Helpers.Diagnostic.Logger.LogError($"[SharedItemNode] Failed to open '{this.FilePath}' in project '{this.SolutionProjectNode.UniqueName}', hr=0x{hr:X8}");
                    ErrorHandler.ThrowOnFailure(hr);
                }
            }

            // Закрываем временный файл переключения контекста
            if (needCloseContextSwitchFile) {
                var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d => string.Equals(d.FullName, contextSwitchFile, StringComparison.OrdinalIgnoreCase));

                doc?.Close(EnvDTE.vsSaveChanges.vsSaveChangesNo);
            }

            // Если наш файл изначально НЕ был активным, возвращаем активным предыдущий документ
            if (!wasAlreadyActive && activeDocumentBefore != null) {
                activeDocumentBefore.Activate();
            }
        }

        //public void OpenWithProjectContext() {
        //    ThreadHelper.ThrowIfNotOnUIThread();

        //    //var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

        //    var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;

        //    // Получаем все проекты которые знают про этот файл
        //    var externalIncludesProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
        //        .GetProjectsByDocumentPath(this.FilePath);

        //    var sharedItemsProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
        //        .GetProjectsByDocumentPath(this.FilePath);

        //    var allProjectNodes = externalIncludesProjectNodes
        //        .Concat(sharedItemsProjectNodes)
        //        .ToList();

        //    var projectGuidsToUnload = new List<Guid>();

        //    foreach (var projectNode in allProjectNodes) {
        //        // Игнорируем из выгрузки:
        //        // - оригинальный Shared.vcxitems проект, которому принадлежит путь this.FilePath.
        //        // - целевой проект this.ProjectNode, в контексте которого и лежит этот SharedItemNode.
        //        bool exlcludedFromUnload = projectNode.Equals(this.SharedProjectNode) || projectNode.Equals(this.ProjectNode);

        //        if (!exlcludedFromUnload) {
        //            projectGuidsToUnload.Add(projectNode.ProjectGuid);
        //        }
        //    }

        //    using (solutionHierarchyAnalyzer.AccumulateProjectEventsScoped()) {
        //        foreach (var projectGuid in projectGuidsToUnload) {
        //            VsHierarchy.UnloadProject(projectGuid);
        //        }
        //    }

        //    // Открываем файл в контексте оставшегося единственного проекта
        //    PackageServices.Dte2.ItemOperations.OpenFile(this.FilePath);

        //    //// Загружаем обратно выгруженные проекты
        //    //using (solutionHierarchyAnalyzer.AccumulateProjectEventsScoped()) {
        //    //    foreach (var projectGuid in projectGuidsToUnload) {
        //    //        VsHierarchy.ReloadProject(projectGuid);
        //    //    }
        //    //}


        //    //bool wasAlreadyActive =
        //    //    activeDocumentBefore != null &&
        //    //    string.Equals(activeDocumentBefore.FullName, this.FilePath, StringComparison.OrdinalIgnoreCase);

        //    //if (!wasAlreadyActive && activeDocumentBefore != null) {
        //    //    activeDocumentBefore.Activate();
        //    //}

        //}


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

            // Сохраняем активный документ до всех действий
            var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

            // Проверяем, совпадает ли уже активный документ с нашим файлом
            bool wasAlreadyActive =
                activeDocumentBefore != null &&
                string.Equals(activeDocumentBefore.FullName, this.FilePath, StringComparison.OrdinalIgnoreCase);


            // Попытка найти первый cpp/h файл проекта,
            // чтобы открыть его и "переключить" контекст редактора на нужный проект.
            // Это нужно для того, чтобы при открытии внешнего include файла
            // Visual Studio знала, что контекстом открытия является именно этот проект.

            //if (this.ProjectNode.Sources.Count == 0) {
            //    System.Diagnostics.Debugger.Break();
            //}
            //string contextSwitchFile = this.ProjectNode.Sources.FirstOrDefault()?.FilePath;

            var includeDependencyAnalyzer = VsShell.Solution.Services.IncludeDependencyAnalyzerService.Instance;
            var allTransitiveIncludingFiles = includeDependencyAnalyzer
                .GetTransitiveFilesIncludersByIncludePath(this.FilePath);

            var currentProjectTransitiveIncludingFiles = allTransitiveIncludingFiles
                .Where(sf => sf.SolutionProjectNode.Equals(this.SolutionProjectNode))
                .ToList();

            string contextSwitchFile = currentProjectTransitiveIncludingFiles
                .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp")
                ?.FilePath;

            bool needCloseContextSwitchFile = false;
            if (!string.IsNullOrEmpty(contextSwitchFile)) {
                bool alreadyOpen = Utils.EnvDteUtils.IsDocumentOpen(contextSwitchFile);
                if (!alreadyOpen) {
                    PackageServices.Dte2.ItemOperations.OpenFile(contextSwitchFile);
                    needCloseContextSwitchFile = true;
                }
            }

            // Используем ExecCommand для эмуляции DoubleClick в Solution Explorer,
            // чтобы Visual Studio открыла файл так, как если бы пользователь дважды кликнул
            // по нему именно в контексте этого проекта в папке External Dependencies.
            if (this.SolutionProjectNode.ProjectHierarchy.Hierarchy is IVsUIHierarchy uiHierarchy) {
                Guid cmdGroup = VSConstants.CMDSETID.UIHierarchyWindowCommandSet_guid;
                const uint cmdId = (uint)VSConstants.VsUIHierarchyWindowCmdIds.UIHWCMDID_DoubleClick;

                int hr = uiHierarchy.ExecCommand(
                    this.ItemId,
                    ref cmdGroup,
                    cmdId,
                    0,
                    IntPtr.Zero,
                    IntPtr.Zero);

                if (ErrorHandler.Succeeded(hr)) {
                    Helpers.Diagnostic.Logger.LogDebug($"[ExternalInclude] Opened '{this.FilePath}' in project '{this.SolutionProjectNode.UniqueName}'");
                }
                else {
                    Helpers.Diagnostic.Logger.LogError($"[ExternalInclude] Failed to open '{this.FilePath}' in project '{this.SolutionProjectNode.UniqueName}', hr=0x{hr:X8}");
                    ErrorHandler.ThrowOnFailure(hr);
                }
            }

            // Закрываем временный файл переключения контекста
            if (needCloseContextSwitchFile) {
                var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d => string.Equals(d.FullName, contextSwitchFile, StringComparison.OrdinalIgnoreCase));

                doc?.Close(EnvDTE.vsSaveChanges.vsSaveChangesNo);
            }

            // Если наш файл изначально НЕ был активным, возвращаем активным предыдущий документ
            if (!wasAlreadyActive && activeDocumentBefore != null) {
                activeDocumentBefore.Activate();
            }
        }

        public override string ToString() {
            return $"ExternalInclude({base.ToStringCore()})";
        }
    }
}