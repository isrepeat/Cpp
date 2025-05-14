using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TabsManagerExtension {
    public class DocumentInfo : INotifyPropertyChanged {
        private string _displayName;
        private string _fullName;

        public string DisplayName {
            get => _displayName;
            set {
                if (_displayName != value) {
                    _displayName = value;
                    OnPropertyChanged();
                }
            }
        }

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

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }


    public class ProjectGroup : INotifyPropertyChanged {
        public string Name { get; set; }
        public ObservableCollection<DocumentInfo> Items { get; set; } = new ObservableCollection<DocumentInfo>();

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }


    public partial class TabsManagerToolWindowControl : UserControl {
        private DTE2 _dte;
        private DocumentEvents _documentEvents;
        private SolutionEvents _solutionEvents;
        private FileSystemWatcher _fileWatcher;

        private DispatcherTimer _saveStateCheckTimer;
        private DispatcherTimer _previewStateCheckTimer;
        public ObservableCollection<DocumentInfo> PreviewDocuments { get; set; } = new ObservableCollection<DocumentInfo>();
        public ObservableCollection<ProjectGroup> GroupedDocuments { get; set; } = new ObservableCollection<ProjectGroup>();


        public TabsManagerToolWindowControl() {
            InitializeComponent();
            InitializeDTE();
            InitializeFileWatcher();
            InitializeTimers();
            DataContext = this;
            LoadOpenDocuments();
        }

        private void InitializeDTE() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (DTE2)Package.GetGlobalService(typeof(DTE));

            _documentEvents = _dte.Events.DocumentEvents;
            _documentEvents.DocumentOpened += DocumentOpenedHandler;
            _documentEvents.DocumentSaved += DocumentSavedHandler;
            _documentEvents.DocumentClosing += DocumentClosingHandler;

            _solutionEvents = _dte.Events.SolutionEvents;
            _solutionEvents.BeforeClosing += OnSolutionClosing;
        }

        private void InitializeFileWatcher() {
            string solutionDir = GetSolutionDirectory();
            if (string.IsNullOrEmpty(solutionDir)) return;

            _fileWatcher = new FileSystemWatcher {
                Path = solutionDir,
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.Size,
                IncludeSubdirectories = true,
                Filter = "*.*"
            };

            _fileWatcher.Changed += OnFileChanged;
            _fileWatcher.Renamed += OnFileRenamed;
            _fileWatcher.Deleted += OnFileDeleted;
            _fileWatcher.EnableRaisingEvents = true;
        }

        private void InitializeTimers() {
            _saveStateCheckTimer = new DispatcherTimer();
            _saveStateCheckTimer.Interval = TimeSpan.FromMilliseconds(200);
            _saveStateCheckTimer.Tick += SaveStateCheckTimerHandler;
            _saveStateCheckTimer.Start();
        }

        private void SaveStateCheckTimerHandler(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Создаем копию для безопасного перебора
            var groupsCopy = GroupedDocuments.ToList();

            foreach (var group in groupsCopy) {
                var itemsCopy = group.Items.ToList();

                foreach (var docInfo in itemsCopy) {
                    var document = _dte.Documents.Cast<Document>().FirstOrDefault(d => d.FullName == docInfo.FullName);
                    if (document != null) {
                        if (document.Saved) {
                            docInfo.DisplayName = docInfo.DisplayName.TrimEnd('*');
                        }
                        else {
                            if (!docInfo.DisplayName.EndsWith("*")) {
                                docInfo.DisplayName += "*";
                            }
                        }
                    }
                }
            }


            foreach (var docInfo in PreviewDocuments.ToList()) {
                if (!IsDocumentInPreviewTab(docInfo.FullName)) {
                    // Перемещаем документ из предварительного просмотра в основную группу
                    MoveDocumentToGroup(docInfo);
                    PreviewDocuments.Remove(docInfo);
                }
            }
        }



        private void DocumentOpenedHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, находится ли документ в режиме предварительного просмотра
            if (IsDocumentInPreviewTab(document.FullName)) {
                AddDocumentToPreview(document);
            }
            else {
                AddDocumentToGroup(document);
            }
        }

        private void DocumentSavedHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            SaveStateCheckTimerHandler(null, null);
        }

        private void DocumentClosingHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            RemoveDocumentFromPreview(document.FullName);
            RemoveDocumentFromGroup(document.FullName);
        }

        private void OnSolutionClosing() {
            GroupedDocuments.Clear();
            _fileWatcher?.Dispose();
        }

        // Обработчик изменения файла (без TMP-файлов)
        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            if (IsTemporaryFile(e.FullPath)) return; // Игнорируем временные файлы

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                UpdateDocumentUI(e.FullPath);
            });
        }

        // Обработчик переименования файла (без TMP-файлов)
        private void OnFileRenamed(object sender, RenamedEventArgs e) {
            if (IsTemporaryFile(e.FullPath) || IsTemporaryFile(e.OldFullPath)) return;

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                UpdateDocumentUI(e.OldFullPath, e.FullPath);
            });
        }

        // Обработчик удаления файла (без TMP-файлов)
        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            if (IsTemporaryFile(e.FullPath)) return;

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                RemoveDocumentFromGroup(e.FullPath);
            });
        }




        private void LoadOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();
            GroupedDocuments.Clear();
            foreach (Document doc in _dte.Documents) {
                AddDocumentToGroup(doc);
            }
        }


        private void AddDocumentToPreview(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, нет ли уже этого документа в Preview
            var existing = PreviewDocuments.FirstOrDefault(d => d.FullName == document.FullName);
            if (existing == null) {
                PreviewDocuments.Clear(); // Только один документ в режиме предварительного просмотра
                PreviewDocuments.Add(new DocumentInfo {
                    DisplayName = document.Name,
                    FullName = document.FullName,
                    ProjectName = GetDocumentProjectName(document)
                });
            }
        }

        private void RemoveDocumentFromPreview(string fullName) {
            var doc = PreviewDocuments.FirstOrDefault(d => d.FullName == fullName);
            if (doc != null) {
                PreviewDocuments.Remove(doc);
            }
        }

        private void MoveDocumentToGroup(DocumentInfo docInfo) {
            string projectName = docInfo.ProjectName;

            // Ищем группу
            var group = GroupedDocuments.FirstOrDefault(g => g.Name == projectName);
            if (group == null) {
                group = new ProjectGroup { Name = projectName };
                GroupedDocuments.Add(group);
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == docInfo.FullName)) {
                group.Items.Add(docInfo);
            }
        }

        private void AddDocumentToGroup(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            string projectName = GetDocumentProjectName(document);

            // Ищем группу
            var group = GroupedDocuments.FirstOrDefault(g => g.Name == projectName);
            if (group == null) {
                group = new ProjectGroup { Name = projectName };
                GroupedDocuments.Add(group);
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == document.FullName)) {
                group.Items.Add(new DocumentInfo {
                    DisplayName = document.Name,
                    FullName = document.FullName,
                    ProjectName = projectName
                });
            }
        }

        private void RemoveDocumentFromGroup(string fullName) {
            ThreadHelper.ThrowIfNotOnUIThread(); 
            
            foreach (var group in GroupedDocuments.ToList()) {
                var docToRemove = group.Items.FirstOrDefault(d => d.FullName == fullName);
                if (docToRemove != null) {
                    group.Items.Remove(docToRemove);
                    if (group.Items.Count == 0) {
                        GroupedDocuments.Remove(group);
                    }
                    break;
                }
            }
        }

        

        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in GroupedDocuments) {
                var docInfo = group.Items.FirstOrDefault(d => d.FullName == oldPath);
                if (docInfo != null) {
                    if (newPath == null) {
                        // Обновляем только имя (в случае изменения)
                        docInfo.DisplayName = Path.GetFileName(oldPath);
                    }
                    else {
                        // Обновляем имя и полный путь (в случае переименования)
                        docInfo.FullName = newPath;
                        docInfo.DisplayName = Path.GetFileName(newPath);
                    }
                    return;
                }
            }
        }



        private string GetSolutionDirectory() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var solution = _dte.Solution;
            return string.IsNullOrEmpty(solution.FullName) ? null : Path.GetDirectoryName(solution.FullName);
        }

        private string GetDocumentProjectName(Document document) {
            try {
                var projectItem = document.ProjectItem?.ContainingProject;
                return projectItem?.Name ?? "Без проекта";
            }
            catch {
                return "Без проекта";
            }
        }

        // Проверка на временный файл (TMP)
        private bool IsTemporaryFile(string fullPath) {
            string extension = Path.GetExtension(fullPath);
            return extension.Equals(".TMP", StringComparison.OrdinalIgnoreCase) ||
                   fullPath.Contains("~") && fullPath.Contains(".TMP");
        }


        private bool IsDocumentInPreviewTab(string documentFullPath) {
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
                    string.Equals(docPath, documentFullPath, StringComparison.OrdinalIgnoreCase)) {
                    // Проверяем, является ли окно временным (предварительный просмотр)
                    if (ErrorHandler.Succeeded(frame.GetProperty((int)__VSFPROPID5.VSFPROPID_IsProvisional, out object isProvisionalObj)) &&
                        isProvisionalObj is bool isProvisional) {
                        return isProvisional;
                    }
                }
            }

            return false;
        }


        // Метод для получения всех открытых окон документов
        private IVsWindowFrame[] GetAllDocumentFrames(IVsUIShell shell) {
            ThreadHelper.ThrowIfNotOnUIThread();
            shell.GetDocumentWindowEnum(out IEnumWindowFrames windowFramesEnum);
            List<IVsWindowFrame> frames = new List<IVsWindowFrame>();
            IVsWindowFrame[] frameArray = new IVsWindowFrame[1];

            while (windowFramesEnum.Next(1, frameArray, out uint fetched) == VSConstants.S_OK && fetched == 1) {
                frames.Add(frameArray[0]);
            }

            return frames.ToArray();
        }

        private void OpenDocumentAsPinned(string documentFullPath) {
            ThreadHelper.ThrowIfNotOnUIThread();

            IVsUIShellOpenDocument openDoc = Package.GetGlobalService(typeof(SVsUIShellOpenDocument)) as IVsUIShellOpenDocument;
            if (openDoc == null)
                return;

            Guid logicalView = VSConstants.LOGVIEWID_Primary;
            IVsUIHierarchy hierarchy;
            uint itemId;
            IVsWindowFrame windowFrame;
            Microsoft.VisualStudio.OLE.Interop.IServiceProvider serviceProvider;

            // Повторное открытие документа
            int hr = openDoc.OpenDocumentViaProject(
                documentFullPath,
                ref logicalView,
                out serviceProvider,
                out hierarchy,
                out itemId,
                out windowFrame);

            if (ErrorHandler.Succeeded(hr) && windowFrame != null) {
                windowFrame.Show();
            }
        }




        private void PinDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                // Проверяем, находится ли документ в режиме предварительного просмотра
                var previewDoc = PreviewDocuments.FirstOrDefault(d => d.FullName == fullName);
                if (previewDoc != null) {
                    // Открываем документ повторно в режиме Opened (закрепленный)
                    OpenDocumentAsPinned(fullName);
                    // Перемещаем документ в основную группу
                    MoveDocumentToGroup(previewDoc);
                    PreviewDocuments.Remove(previewDoc);
                }
            }
        }


        private void DocumentList_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is ListBox listBox && listBox.SelectedItem is DocumentInfo docInfo) {
                var document = _dte.Documents.Cast<Document>()
                                .FirstOrDefault(d => string.Equals(d.FullName, docInfo.FullName, StringComparison.OrdinalIgnoreCase));

                if (document != null) {
                    document.Activate();
                }

                listBox.SelectedItem = null;
            }
        }

        private void CloseDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                var document = _dte.Documents.Cast<Document>().FirstOrDefault(doc => doc.FullName == fullName);
                if (document != null) {
                    document.Close();
                }
            }
        }
    }
}