using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using TabsManagerExtension.VsShell.Project;
using Microsoft.VisualStudio.OLE.Interop;


namespace TabsManagerExtension.VsShell.Document {
    public class DocumentNode {
        public VsShell.Project.ProjectNode ProjectNode { get; }
        public string FilePath { get; }
        public uint ItemId { get; }

        public DocumentNode(VsShell.Project.ProjectNode projectNode, string filePath, uint itemId) {
            this.ProjectNode = projectNode;
            this.FilePath = filePath;
            this.ItemId = itemId;
        }

        public override bool Equals(object? obj) {
            if (obj is not DocumentNode other) {
                return false;
            }

            return
                StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                StringComparer.OrdinalIgnoreCase.Equals(this.ProjectNode.Project.UniqueName, other.ProjectNode.Project.UniqueName);
        }

        public override int GetHashCode() {
            unchecked {
                int hash = 17;

                hash = hash * 31 + (this.ProjectNode.Project.UniqueName != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.ProjectNode.Project.UniqueName)
                    : 0);

                hash = hash * 31 + (this.FilePath != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath)
                    : 0);

                return hash;
            }
        }

        public override string ToString() {
            return $"DocumentNode(FilePath='{this.FilePath}', Project='{this.ProjectNode.Project.UniqueName}', ItemId={this.ItemId})";
        }
    }



    public sealed class ExternalInclude : DocumentNode {
        public ExternalInclude(VsShell.Project.ProjectNode projectNode, string filePath, uint itemId)
            : base(projectNode, filePath, itemId) {
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
            if (this.ProjectNode.DocumentNodes.Count == 0) {
                this.ProjectNode.UpdateDocumentNodes();
            }
            string contextSwitchFile = this.ProjectNode.DocumentNodes.FirstOrDefault()?.FilePath;

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
                    Helpers.Diagnostic.Logger.LogDebug(
                        $"[IncludeGraph] Opened '{this.FilePath}' in project '{this.ProjectNode.Project.UniqueName}'");
                }
                else {
                    Helpers.Diagnostic.Logger.LogError(
                        $"[IncludeGraph] Failed to open '{this.FilePath}' in project '{this.ProjectNode.Project.UniqueName}', hr=0x{hr:X8}");
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
    }
}