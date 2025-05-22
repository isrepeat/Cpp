using EnvDTE;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.InteropServices;
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

            return projects.Select(p => new TabItemProject(p)).ToList();
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


        public void OpenDocumentAsPinned() {
            ThreadHelper.ThrowIfNotOnUIThread();

            IVsUIShellOpenDocument openDoc = Package.GetGlobalService(typeof(SVsUIShellOpenDocument)) as IVsUIShellOpenDocument;
            if (openDoc == null) {
                return;
            }

            Guid logicalView = VSConstants.LOGVIEWID_Primary;
            IVsUIHierarchy hierarchy;
            uint itemId;
            IVsWindowFrame windowFrame;
            Microsoft.VisualStudio.OLE.Interop.IServiceProvider serviceProvider;

            // Повторное открытие документа
            int hr = openDoc.OpenDocumentViaProject(
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

        public static string GetWindowId(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                // У каждого Tool Window — свой уникальный ObjectKind,
                // потому что они создаются как отдельные компоненты, зарегистрированные в системе Visual Studio.
                return window.ObjectKind;
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"GetWindowId(ObjectKind) failed: {ex.Message}");
                return string.Empty;
            }
        }
        public string GetWindowId() {
            return GetWindowId(this.Window);
        }
    }







    public abstract class TabItemBase : Helpers.ObservableObject {

        private string _caption;
        public string Caption {
            get => _caption;
            set {
                if (_caption != value) {
                    _caption = value;
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
            base.Caption = shellProject.Project.Name;
            base.FullName = shellProject.Project.FullName;
            this.ShellProject = shellProject;
        }

        public TabItemProject(EnvDTE.Project project)
            : this(new ShellProject(project)) {
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
            base.Caption = shellDocument.Document.Name;
            base.FullName = shellDocument.Document.FullName;
            this.ShellDocument = shellDocument;
        }

        public TabItemDocument(EnvDTE.Document document)
            : this(new ShellDocument(document)) {
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                this.ShellDocument.Document?.Activate();
            }
            catch (COMException ex) {
                Helpers.Diagnostic.Logger.LogWarning($"Failed to activate document '{this.Caption}': {ex.Message}");
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"Unexpected error activating document '{this.Caption}': {ex.Message}");
            }
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
        public ShellWindow ShellWindow { get; private set; }

        public string WindowId { get; private set; }

        public TabItemWindow(ShellWindow shellWindow) {
            base.Caption = shellWindow.Window.Caption;
            base.FullName = shellWindow.Window.Caption;
            this.ShellWindow = shellWindow;
            this.WindowId = shellWindow.GetWindowId();
        }

        public TabItemWindow(EnvDTE.Window window)
            : this(new ShellWindow(window)) {
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                this.ShellWindow.Window?.Activate();
            }
            catch (COMException ex) {
                Helpers.Diagnostic.Logger.LogWarning($"Failed to activate window '{this.Caption}': {ex.Message}");
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"Unexpected error activating window '{this.Caption}': {ex.Message}");
            }
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


    public class TabItemGroup : Helpers.ISelectableGroup<TabItemBase> {
        public string GroupName { get; set; }
        public ObservableCollection<TabItemBase> TabItems { get; set; } = new();
        public ObservableCollection<TabItemBase> SelectedItems { get; set; } = new();
    }
}