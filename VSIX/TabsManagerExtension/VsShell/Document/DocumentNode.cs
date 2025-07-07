using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using TabsManagerExtension.VsShell.Utils;
using static TabsManagerExtension.VsShell.Utils.VsHierarchy;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentNode {
        public uint ItemId { get; }
        public string FilePath { get; }
        public VsShell.Project.ProjectNode ProjectNode { get; }

        public DocumentNode(uint itemId, string filePath, VsShell.Project.ProjectNode projectNode) {
            this.ItemId = itemId;
            this.FilePath = filePath;
            this.ProjectNode = projectNode;
        }

        public override bool Equals(object? obj) {
            if (obj is not DocumentNode other) {
                return false;
            }

            return
                this.ProjectNode.ProjectGuid == other.ProjectNode.ProjectGuid &&
                StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath);
        }

        public override int GetHashCode() {
            unchecked {
                int hash = 17;

                hash = hash * 31 + this.ProjectNode.ProjectGuid.GetHashCode();
                hash = hash * 31 + (this.FilePath != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath)
                    : 0);

                return hash;
            }
        }


        protected string ToStringCore() {
            return $"FilePath='{this.FilePath}', Project='{this.ProjectNode.UniqueName}', ItemId={this.ItemId}";
        }

        public override string ToString() {
            return $"DocumentNode({this.ToStringCore()})";
        }
    }



    public sealed class SharedItemNode : DocumentNode {
        public VsShell.Project.ProjectNode SharedProjectNode { get; private set; }
        
        private IVsHierarchy? _sharedAssetsProjectHierarchy;
        public SharedItemNode(
            uint itemId,
            string filePath,
            VsShell.Project.ProjectNode projectNode,
            VsShell.Project.ProjectNode sharedProjectNode)
            : base(itemId, filePath, projectNode) {

            this.SharedProjectNode = sharedProjectNode;
        }


        public void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

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
            if (this.ProjectNode.Sources.Count == 0) {
                System.Diagnostics.Debugger.Break();
            }
            string contextSwitchFile = this.ProjectNode.Sources.FirstOrDefault()?.FilePath;

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
            if (this.ProjectNode.VsHierarchy is IVsUIHierarchy uiHierarchy) {
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
                    Helpers.Diagnostic.Logger.LogDebug($"[IncludeGraph] Opened '{this.FilePath}' in project '{this.ProjectNode.UniqueName}'");
                }
                else {
                    Helpers.Diagnostic.Logger.LogError($"[IncludeGraph] Failed to open '{this.FilePath}' in project '{this.ProjectNode.UniqueName}', hr=0x{hr:X8}");
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
        public ExternalInclude(uint itemId, string filePath, VsShell.Project.ProjectNode projectNode)
            : base(itemId, filePath, projectNode) {
        }

        public void OpenWithProjectContext() {
            ThreadHelper.ThrowIfNotOnUIThread();

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
                .Where(sf => sf.ProjectNode.Equals(this.ProjectNode))
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
            if (this.ProjectNode.VsHierarchy is IVsUIHierarchy uiHierarchy) {
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
                    Helpers.Diagnostic.Logger.LogDebug($"[IncludeGraph] Opened '{this.FilePath}' in project '{this.ProjectNode.UniqueName}'");
                }
                else {
                    Helpers.Diagnostic.Logger.LogError($"[IncludeGraph] Failed to open '{this.FilePath}' in project '{this.ProjectNode.UniqueName}', hr=0x{hr:X8}");
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