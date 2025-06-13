using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using EnvDTE;


namespace TabsManagerExtension.State.Document {
    public class ShellProject {
        public EnvDTE.Project Project { get; private set; }
        private EnvDTE80.DTE2 _dte;

        public ShellProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Project = project;
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }


        /// <summary>
        /// Возвращает список директорий из MSBuild-свойства (например, "PublicIncludeDirectories", "AdditionalLibraryDirectories").
        /// 
        /// ⚠️ Значение извлекается только для активной конфигурации (например, "Debug|x64") —
        /// именно так, как это делает Visual Studio при сборке проекта.
        /// </summary>
        public List<string> GetMsBuildProjectPropertyList(string propertyName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<string>();
            string? rawValue = null;

            var hierarchy = GetVsHierarchy(this.Project);
            if (hierarchy is not IVsBuildPropertyStorage storage) {
                return result;
            }

            int hr = storage.GetPropertyValue(propertyName, null, (uint)_PersistStorageType.PST_PROJECT_FILE, out rawValue);
            if (!ErrorHandler.Succeeded(hr) || string.IsNullOrWhiteSpace(rawValue)) {
                return result;
            }

            foreach (var dir in rawValue.Split(';')) {
                string trimmed = dir.Trim();
                if (string.IsNullOrEmpty(trimmed)) {
                    continue;
                }

                string expanded = ExpandMsBuildVariables(trimmed, this.Project);

                if (!expanded.Contains("%")) {
                    try {
                        result.Add(Path.GetFullPath(expanded));
                    }
                    catch {
                        // skip invalid
                    }
                }
            }

            return result.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        }

        public List<string> GetAdditionalIncludeDirectories() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("GetProjectIncludeDirectories()");
            Helpers.Diagnostic.Logger.LogDebug($"  [Project.Name] = {this.Project.Name}");

            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<string>();

            if (this.Project.Object is Microsoft.VisualStudio.VCProjectEngine.VCProject vcProject) {
                var configs = vcProject.Configurations as Microsoft.VisualStudio.VCProjectEngine.IVCCollection;
                if (configs == null) {
                    return result;
                }

                Helpers.Diagnostic.Logger.LogDebug($"  [configs.Count] = {configs.Count}");
                
                foreach (Microsoft.VisualStudio.VCProjectEngine.VCConfiguration config in configs) {
                    Helpers.Diagnostic.Logger.LogDebug($"    [config.Name] = {config.Name}");
                    var tools = config.Tools as Microsoft.VisualStudio.VCProjectEngine.IVCCollection;
                    if (tools == null) {
                        continue;
                    }
                    Helpers.Diagnostic.Logger.LogDebug($"    [tools.Count] = {tools.Count}");

                    foreach (object tool in tools) {
                        if (tool is Microsoft.VisualStudio.VCProjectEngine.VCCLCompilerTool cl) {
                            Helpers.Diagnostic.Logger.LogDebug($"      [cl.Name] = {cl.toolName}");

                            var dirs = cl.AdditionalIncludeDirectories?.Split(';') ?? Array.Empty<string>();

                            foreach (var dir in dirs) {
                                Helpers.Diagnostic.Logger.LogDebug($"        [dir] = {dir}");

                                string trimmed = dir.Trim();
                                if (!string.IsNullOrEmpty(trimmed)) {
                                    string expanded = ExpandMsBuildVariables(trimmed, this.Project);

                                    if (!expanded.Contains("%")) {
                                        try {
                                            result.Add(Path.GetFullPath(expanded));
                                        }
                                        catch {
                                            // ignore invalid paths
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        }



        private string ExpandMsBuildVariables(string pathWithVars, EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            string expanded = pathWithVars;

            if (string.IsNullOrEmpty(pathWithVars)) {
                return expanded;
            }

            var hierarchy = GetVsHierarchy(project);
            if (hierarchy is not IVsBuildPropertyStorage propertyStorage) {
                return expanded;
            }

            // Попробуем найти все $(...) и заменить по очереди
            var matches = System.Text.RegularExpressions.Regex.Matches(pathWithVars, @"\$\(([^)]+)\)");
            foreach (System.Text.RegularExpressions.Match match in matches) {
                string fullVar = match.Groups[0].Value; // $(ProjectDir)
                string name = match.Groups[1].Value;    // ProjectDir

                int hr = VSConstants.S_OK;
                string value = string.Empty;

                if (name == "MSBuildThisFileDirectory") {
                    value = Path.GetDirectoryName(project.FullName);
                }
                else {
                    hr = propertyStorage.GetPropertyValue(name, null, (uint)_PersistStorageType.PST_PROJECT_FILE, out value);
                }

                if (ErrorHandler.Succeeded(hr) && !string.IsNullOrWhiteSpace(value)) {
                    expanded = System.Text.RegularExpressions.Regex.Replace(
                                expanded,
                                System.Text.RegularExpressions.Regex.Escape(fullVar),
                                value,
                                System.Text.RegularExpressions.RegexOptions.IgnoreCase);
                }
            }

            return expanded;
        }

        private IVsHierarchy? GetVsHierarchy(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var solution = Package.GetGlobalService(typeof(SVsSolution)) as IVsSolution;
            if (solution == null) {
                return null;
            }

            int hr = solution.GetProjectOfUniqueName(project.UniqueName, out IVsHierarchy hierarchy);
            return ErrorHandler.Succeeded(hr) ? hierarchy : null;
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


        public List<TabItemProject> GetDocumentProjectsIncludingIndirect() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var projects = new List<EnvDTE.Project>();
            var alreadyAdded = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            // ① Основной проект + shared project
            foreach (var p in this.GetDocumentProjects()) {
                if (alreadyAdded.Add(p.ShellProject.Project.UniqueName)) {
                    projects.Add(p.ShellProject.Project);
                }
            }

            // ② Подключения по пути (включения .h в других проектах)
            var rdt = (IVsRunningDocumentTable)Package.GetGlobalService(typeof(SVsRunningDocumentTable));
            var solution = (IVsSolution)Package.GetGlobalService(typeof(SVsSolution));

            if (rdt != null && solution != null) {
                rdt.GetRunningDocumentsEnum(out IEnumRunningDocuments enumDocs);
                uint[] pCookie = new uint[1];
                uint fetched;

                while (enumDocs.Next(1, pCookie, out fetched) == VSConstants.S_OK && fetched == 1) {
                    rdt.GetDocumentInfo(
                        pCookie[0],
                        out _,
                        out _,
                        out _,
                        out string docPath,
                        out var pHierarchy,
                        out var pItemId,
                        out var docData);

                    if (!string.Equals(docPath, this.Document.FullName, StringComparison.OrdinalIgnoreCase)) {
                        continue;
                    }

                    if (pHierarchy != null) {
                        if (ErrorHandler.Succeeded(pHierarchy.GetProperty(VSConstants.VSITEMID_ROOT, (int)__VSHPROPID.VSHPROPID_ExtObject, out var extObject))) {
                            if (extObject is EnvDTE.Project project) {
                                if (alreadyAdded.Add(project.UniqueName)) {
                                    projects.Add(project);
                                }
                            }
                        }
                    }
                }
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
            return IsTabWindow(this.Window);
        }
        public static bool IsTabWindow(EnvDTE.Window window) {
            if (window == null) {
                return false;
            }

            // Во вкладках редактора окна имеют Linkable == false, tool windows — true.
            return !window.Linkable;
        }


        public string GetWindowId() {
            return GetWindowId(this.Window);
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
    }





    public abstract class TabItemBase : Helpers.SelectableItemBase {

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

        private bool _isPreviewTab = false;
        public bool IsPreviewTab {
            get => _isPreviewTab;
            set {
                if (_isPreviewTab != value) {
                    _isPreviewTab = value;
                    OnPropertyChanged();
                }
            }
        }

        private bool _isPinnedTab = false;
        public bool IsPinnedTab {
            get => _isPinnedTab;
            set {
                if (_isPinnedTab != value) {
                    _isPinnedTab = value;
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
        //public List<string> PublicIncludeDirectories { get; private set; }
        //public List<string> AdditionalIncludeDirectories { get; private set; }

        public TabItemProject(ShellProject shellProject) {
            base.Caption = shellProject.Project.Name;
            base.FullName = shellProject.Project.FullName;
            this.ShellProject = shellProject;
            //this.PublicIncludeDirectories = this.ShellProject.GetMsBuildProjectPropertyList("PublicIncludeDirectories");
            //this.AdditionalIncludeDirectories = this.ShellProject.GetAdditionalIncludeDirectories();
            //if (this.PublicIncludeDirectories.Count > 0) {
            //    int xx = 9;
            //}
            //if (this.AdditionalIncludeDirectories.Count > 0) {
            //    int xx = 9;
            //}
        }

        public TabItemProject(EnvDTE.Project project)
            : this(new ShellProject(project)) {
        }

        public override bool Equals(object? obj) {
           return obj is TabItemProject other &&
                StringComparer.OrdinalIgnoreCase.Equals(this.ShellProject.Project?.UniqueName, other.ShellProject.Project?.UniqueName);
        }

        public override int GetHashCode() {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(this.ShellProject.Project?.UniqueName ?? string.Empty);
        }

        public override string ToString() => base.FullName;
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
        //public void UpdateProjectReferenceList() {
        //    this.ProjectReferenceList.Clear();

        //    var projects = this.ShellDocument.GetDocumentProjects()
        //        .Select(p => new DocumentProjectReferenceInfo(
        //            tabItemProject: p,
        //            tabItemDocument: this
        //        ));

        //    foreach (var project in projects) {
        //        this.ProjectReferenceList.Add(project);
        //    }
        //}

        public void UpdateProjectReferenceList() {
            this.ProjectReferenceList.Clear();

            var projects = this.ShellDocument.GetDocumentProjectsIncludingIndirect()
                .Select(p => new DocumentProjectReferenceInfo(p, this));

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



    public abstract class TabItemsGroupBase : Helpers.ObservableObject, Helpers.ISelectableGroup<TabItemBase> {
        public string GroupName { get; }

        public Helpers.SortedObservableCollection<TabItemBase> Items { get; }

        public Helpers.IMetadata Metadata { get; } = new Helpers.FlaggableMetadata();

        protected TabItemsGroupBase(string groupName) {
            this.GroupName = groupName;

            var defaultTabItemBaseComparer = Comparer<TabItemBase>.Create((a, b) =>
                string.Compare(a.Caption, b.Caption, StringComparison.OrdinalIgnoreCase));

            this.Items = new Helpers.SortedObservableCollection<TabItemBase>(defaultTabItemBaseComparer);
            this.Items.CollectionChanged += (s, e) => {
                OnPropertyChanged(nameof(this.GroupName));
            };
        }
    }


    public class TabItemsPreviewGroup : TabItemsGroupBase {
        public TabItemsPreviewGroup() : base("__Preview__") {
            this.Metadata.SetFlag("IsPreviewGroup", true);
        }
    }

    public class TabItemsPinnedGroup : TabItemsGroupBase {
        public TabItemsPinnedGroup(string groupName) : base(groupName) {
            this.Metadata.SetFlag("IsPinnedGroup", true);
        }
    }

    public class TabItemsDefaultGroup : TabItemsGroupBase {
        public TabItemsDefaultGroup(string groupName) : base(groupName) {
        }
    }

    public class SeparatorTabItemsGroup : TabItemsGroupBase {
        public string Key { get; }
        public SeparatorTabItemsGroup(string key) : base(string.Empty) {
            this.Key = key;
            this.Metadata.SetFlag("IsSeparator", true);
        }
    }
}