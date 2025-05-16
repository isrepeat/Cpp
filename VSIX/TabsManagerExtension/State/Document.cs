using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TabsManagerExtension {
    public class ShellDocument {
        public EnvDTE.Document Document { get; private set; }
        private DTE2 _dte;

        public ShellDocument(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            
            this.Document = document;
            _dte = (DTE2)Package.GetGlobalService(typeof(DTE));
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

        public List<Project> GetDocumentProjects() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var projects = new List<Project>();

            try {
                // Проверяем основной проект
                var primaryProject = this.Document.ProjectItem?.ContainingProject;
                if (primaryProject != null) {
                    projects.Add(primaryProject);
                }

                // Проверяем дополнительные проекты через Shared Project (если есть)
                var solution = _dte.Solution;
                foreach (Project project in solution.Projects) {
                    if (this.ProjectContainsDocumentInProject(project)) {
                        if (!projects.Contains(project)) {
                            projects.Add(project);
                        }
                    }
                }
            }
            catch (Exception ex) {
                System.Diagnostics.Debug.WriteLine($"[ERROR] GetDocumentProjects: {ex.Message}");
            }

            return projects;
        }

        private bool ProjectContainsDocumentInProject(Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                foreach (ProjectItem item in project.ProjectItems) {
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
        private bool ProjectItemContainsDocument(ProjectItem item) {
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
                    foreach (ProjectItem subItem in item.ProjectItems) {
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


        public bool IsDocumentInPreviewTab() {
            ThreadHelper.ThrowIfNotOnUIThread();

            IVsUIShell shell = Package.GetGlobalService(typeof(SVsUIShell)) as IVsUIShell;
            if (shell == null)
                return false;

            shell.GetDocumentWindowEnum(out IEnumWindowFrames windowFramesEnum);
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
    }


    public class DocumentInfo : Helpers.ObservableObject {

        private string _displayName;
        public string DisplayName {
            get => _displayName;
            set {
                if (_displayName != value) {
                    _displayName = value;
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

        public string ProjectName { get; set; }

        public ShellDocument ShellDocument { get; private set; }
        public DocumentInfo(ShellDocument shellDocument) {
            this.ShellDocument = shellDocument;

            this.DisplayName = shellDocument.Document.Name;
            this.FullName = shellDocument.Document.FullName;
            this.ProjectName = shellDocument.GetDocumentProjectName();
        }
    }


    public class ProjectGroup : Helpers.ObservableObject {
        public string Name { get; set; }
        public ObservableCollection<DocumentInfo> Items { get; set; } = new ObservableCollection<DocumentInfo>();
    }
}