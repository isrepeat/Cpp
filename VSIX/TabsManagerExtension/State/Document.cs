using EnvDTE;
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
    public class ShellProject {
        public EnvDTE.Project Project { get; private set; }
        private EnvDTE80.DTE2 _dte;

        public ShellProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Project = project;
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }
    }

    public class ShellDocument {
        public EnvDTE.Document Document { get; private set; }
        private EnvDTE80.DTE2 _dte;

        public ShellDocument(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            
            this.Document = document;
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
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

        public List<TabItemProject> GetDocumentProjects() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var projects = new List<EnvDTE.Project>();

            try {
                // Проверяем основной проект
                var primaryProject = this.Document.ProjectItem?.ContainingProject;
                if (primaryProject != null) {
                    projects.Add(primaryProject);
                }

                // Проверяем дополнительные проекты через Shared Project (если есть)
                var solution = _dte.Solution;
                foreach (EnvDTE.Project project in solution.Projects) {
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

            return projects.Select(p => new TabItemProject(new ShellProject(p))).ToList();
        }

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

    public class ShellWindow {
        public EnvDTE.Window Window { get; private set; }
        private EnvDTE80.DTE2 _dte;

        public ShellWindow(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Window = window;
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }

        public bool IsTabWindow() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (this.Window == null) {
                return false;
            }

            // Во вкладках редактора окна имеют Linkable == false, tool windows — true.
            return !this.Window.Linkable;
        }
    }













    public abstract class TabItemBase : Helpers.ObservableObject {

        private string _name;
        public string Name {
            get => _name;
            set {
                if (_name != value) {
                    _name = value;
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
    }

    public interface IActivatableTab {
        void Activate();
    }



    public class TabItemProject : TabItemBase {
        public ShellProject ShellProject { get; private set; }
        public TabItemProject(ShellProject shellProject) {
            base.Name = shellProject.Project.Name;
            base.FullName = shellProject.Project.FullName;
            this.ShellProject = shellProject;
        }
    }


    public class TabItemDocument : TabItemBase, IActivatableTab {
        public ShellDocument ShellDocument { get; private set; }


        private ObservableCollection<DocumentProjectReferenceInfo> _projectReferenceList = new ObservableCollection<DocumentProjectReferenceInfo>();
        public ObservableCollection<DocumentProjectReferenceInfo> ProjectReferenceList {
            get => _projectReferenceList;
            set {
                _projectReferenceList = value;
                OnPropertyChanged();
            }
        }

        public TabItemDocument(ShellDocument shellDocument) {
            base.Name = shellDocument.Document.Name;
            base.FullName = shellDocument.Document.FullName;
            this.ShellDocument = shellDocument;

            this.UpdateProjectReferenceList();
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.ShellDocument.Document.Activate();
        }
        public void UpdateProjectReferenceList() {
            this.ProjectReferenceList.Clear();

            var projects = this.ShellDocument.GetDocumentProjects()
                .Select(p => new DocumentProjectReferenceInfo(
                    tabItemProject: p,
                    tabItemDocument: this
                ));

            foreach (var project in projects) {
                this.ProjectReferenceList.Add(project);
            }
        }
    }


    public class TabItemWindow : TabItemBase, IActivatableTab {
        public EnvDTE.Window Window { get; private set; }

        public TabItemWindow(EnvDTE.Window window) {
            base.Name = window.Caption;
            base.FullName = window.Caption;
            this.Window = window;
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.Window.Activate();
        }
    }







    public class DocumentProjectReferenceInfo : Helpers.ObservableObject {
        public TabItemProject TabItemProject { get; private set; }
        public TabItemDocument TabItemDocument { get; private set; }

        public DocumentProjectReferenceInfo(TabItemProject tabItemProject, TabItemDocument tabItemDocument) {
            this.TabItemProject = tabItemProject;
            this.TabItemDocument = tabItemDocument;
        }
    }


    public class TabItemGroup : Helpers.ObservableObject {
        public string Name { get; set; }
        public ObservableCollection<TabItemBase> Items { get; set; } = new ObservableCollection<TabItemBase>();
    }
}