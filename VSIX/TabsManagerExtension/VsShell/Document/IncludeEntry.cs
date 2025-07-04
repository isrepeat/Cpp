using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Document {
    public sealed class ExternalInclude {
        public VsShell.Project.ProjectNode ProjectNode { get; }
        public string FilePath { get; }
        public uint ItemId { get; }

        public ExternalInclude(VsShell.Project.ProjectNode projectNode, string filePath, uint itemId) {
            this.ProjectNode = projectNode;
            this.FilePath = filePath;
            this.ItemId = itemId;
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
            string? contextSwitchFile = this.ProjectNode.FirstCppRelatedFile;
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


        public override string ToString() {
            return $"ExternalInclude(FilePath='{this.FilePath}', Project='{this.ProjectNode.Project.UniqueName}', ItemId={this.ItemId})";
        }
    }



    public class IncludeEntry {
        public string RawInclude { get; }
        public string NormalizedName { get; }

        public IncludeEntry(string rawInclude) {
            this.RawInclude = rawInclude;
            this.NormalizedName = Path.GetFileName(rawInclude);
        }

        public override bool Equals(object? obj) {
            return obj is IncludeEntry other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.RawInclude, other.RawInclude);
        }

        public override int GetHashCode() {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(this.RawInclude);
        }

        public override string ToString() => this.RawInclude;
    }


    public class ResolvedIncludeEntry {
        public IncludeEntry IncludeEntry { get; }
        public string? ResolvedPath { get; }

        public ResolvedIncludeEntry(IncludeEntry includeEntry, string? resolvedPath) {
            this.IncludeEntry = includeEntry;
            this.ResolvedPath = resolvedPath;
        }

        public override bool Equals(object? obj) {
            return obj is ResolvedIncludeEntry other &&
                   this.IncludeEntry.Equals(other.IncludeEntry) &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.ResolvedPath, other.ResolvedPath);
        }

        public override int GetHashCode() {
            int h1 = this.IncludeEntry.GetHashCode();
            int h2 = this.ResolvedPath is null ? 0 : StringComparer.OrdinalIgnoreCase.GetHashCode(this.ResolvedPath);
            return (h1 * 397) ^ h2;
        }

        public override string ToString() => $"{this.IncludeEntry.RawInclude} → {this.ResolvedPath ?? "unresolved"}";
    }
}