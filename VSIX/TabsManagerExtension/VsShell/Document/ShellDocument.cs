using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Document {
    public class ShellDocument {
        public EnvDTE.Document Document { get; private set; }

        public ShellDocument(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Document = document;
        }


        public string GetDocumentProjectName() {
            try {
                var projectItem = this.Document.ProjectItem?.ContainingProject;
                return projectItem?.Name ?? "Без проекта";
            }
            catch {
                return "Без проекта";
            }
        }

      

        public bool IsDocumentInPreviewTab() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsUiShell = PackageServices.VsUIShell;
            if (vsUiShell == null)
                return false;

            vsUiShell.GetDocumentWindowEnum(out IEnumWindowFrames windowFramesEnum);
            IVsWindowFrame[] frameArray = new IVsWindowFrame[1];
            uint fetched;

            while (windowFramesEnum.Next(1, frameArray, out fetched) == VSConstants.S_OK && fetched == 1) {
                IVsWindowFrame frame = frameArray[0];
                if (frame == null)
                    continue;

                // Получаем путь к документу
                if (ErrorHandler.Succeeded(frame.GetProperty((int)__VSFPROPID.VSFPROPID_pszMkDocument, out object docPathObj)) &&
                    docPathObj is string docPath &&
                    string.Equals(docPath, this.Document.FullName, StringComparison.OrdinalIgnoreCase)) {
                    // Проверяем, является ли окно временным (предварительный просмотр)
                    if (ErrorHandler.Succeeded(frame.GetProperty((int)__VSFPROPID5.VSFPROPID_IsProvisional, out object isProvisionalObj)) &&
                        isProvisionalObj is bool isProvisional) {
                        return isProvisional;
                    }
                }
            }

            return false;
        }


        public void OpenDocumentAsPinned() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsUIShellOpenDocument = PackageServices.VsUIShellOpenDocument;
            if (vsUIShellOpenDocument == null) {
                return;
            }

            Guid logicalView = VSConstants.LOGVIEWID_Primary;
            IVsUIHierarchy hierarchy;
            uint itemId;
            IVsWindowFrame windowFrame;
            Microsoft.VisualStudio.OLE.Interop.IServiceProvider serviceProvider;

            // Повторное открытие документа
            int hr = vsUIShellOpenDocument.OpenDocumentViaProject(
                this.Document.FullName,
                ref logicalView,
                out serviceProvider,
                out hierarchy,
                out itemId,
                out windowFrame);

            if (ErrorHandler.Succeeded(hr) && windowFrame != null) {
                windowFrame.Show();
            }
        }


        //
        // Internal logic
        //
        private bool ProjectContainsDocumentInProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                    if (this.ProjectItemContainsDocument(item)) {
                        return true;
                    }
                }
            }
            catch {
                // Игнорируем ошибки проверки
            }

            return false;
        }


        // Метод проверки документа внутри ProjectItem (включая вложенные)
        private bool ProjectItemContainsDocument(EnvDTE.ProjectItem item) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                if (item.FileCount > 0) {
                    for (short i = 1; i <= item.FileCount; i++) {
                        string filePath = item.FileNames[i];
                        if (string.Equals(filePath, this.Document.FullName, StringComparison.OrdinalIgnoreCase)) {
                            return true;
                        }
                    }
                }

                // Проверяем вложенные элементы (вложенные папки, ссылки)
                if (item.ProjectItems?.Count > 0) {
                    foreach (EnvDTE.ProjectItem subItem in item.ProjectItems) {
                        if (this.ProjectItemContainsDocument(subItem)) {
                            return true;
                        }
                    }
                }
            }
            catch {
                // Игнорируем ошибки
            }

            return false;
        }
    }
}