using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace TabsManagerExtension {
    public partial class TabsManagerToolWindowControl : UserControl {
        private DTE2 _dte;
        private DocumentEvents _documentEvents;
        private SolutionEvents _solutionEvents;
        private FileSystemWatcher _fileWatcher;

        private DispatcherTimer _saveStateCheckTimer;
        private DispatcherTimer _previewStateCheckTimer;
        public ObservableCollection<DocumentInfo> PreviewDocuments { get; set; } = new ObservableCollection<DocumentInfo>();
        public ObservableCollection<ProjectGroup> GroupedDocuments { get; set; } = new ObservableCollection<ProjectGroup>();
        private CollectionViewSource GroupedDocumentsViewSource { get; set; }

        private ShellDocument _activeShellDocument;
        private double _currentScaleFactor = 1.0;


        public TabsManagerToolWindowControl() {
            InitializeComponent();
            InitializeDTE();
            InitializeFileWatcher();
            InitializeTimers();
            InitializeDocumentsViewSource();

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
            if (string.IsNullOrEmpty(solutionDir)) {
                return;
            }

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

        public void InitializeDocumentsViewSource() {
            // Создаем представление с сортировкой
            GroupedDocumentsViewSource = new CollectionViewSource { Source = GroupedDocuments };

            // Сортировка групп по имени (по алфавиту)
            GroupedDocumentsViewSource.SortDescriptions.Add(new SortDescription("Name", ListSortDirection.Ascending));

            // Применяем сортировку внутри групп через метод
            GroupedDocumentsViewSource.View.CollectionChanged += (s, e) => SortDocumentsInGroups();
        }


        //
        // Event handlers
        //
        private void DocumentOpenedHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var shellDocument = new ShellDocument(document);

            // Проверяем, находится ли документ в режиме предварительного просмотра
            if (shellDocument.IsDocumentInPreviewTab()) {
                this.AddDocumentToPreview(shellDocument);
            }
            else {
                this.AddDocumentToGroup(shellDocument);
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
            if (IsTemporaryFile(e.FullPath)) {
                return; // Игнорируем временные файлы
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                UpdateDocumentUI(e.FullPath);
            });
        }

        // Обработчик переименования файла (без TMP-файлов)
        private void OnFileRenamed(object sender, RenamedEventArgs e) {
            if (IsTemporaryFile(e.FullPath) || IsTemporaryFile(e.OldFullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                UpdateDocumentUI(e.OldFullPath, e.FullPath);
            });
        }

        // Обработчик удаления файла (без TMP-файлов)
        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            if (IsTemporaryFile(e.FullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                RemoveDocumentFromGroup(e.FullPath);
            });
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
                if (!docInfo.ShellDocument.IsDocumentInPreviewTab()) {
                    // Перемещаем документ из предварительного просмотра в основную группу
                    this.MoveDocumentToGroup(docInfo);
                    this.PreviewDocuments.Remove(docInfo);
                }
            }
        }



        //
        // UI click handlers
        //
        private void ShowProjectsFlyout_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is Button button && button.Tag is string fullName) {
                var document = _dte.Documents.Cast<Document>().FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
                if (document == null) {
                    return;
                }

                var shellDocument = new ShellDocument(document);
                var projects = shellDocument.GetDocumentProjects();

                // Создаем ContextMenu вместо Popup
                var contextMenu = new ContextMenu();

                foreach (var project in projects) {
                    var projectMenuItem = new MenuItem {
                        Header = project.Name,
                        CommandParameter = new Tuple<string, string>(fullName, project.Name)
                    };
                    projectMenuItem.Click += ProjectMenuItem_Click;
                    contextMenu.Items.Add(projectMenuItem);
                }

                // Отображаем ContextMenu
                contextMenu.IsOpen = true;
                contextMenu.PlacementTarget = button;
                contextMenu.Placement = System.Windows.Controls.Primitives.PlacementMode.Right;
            }
        }

        private void ProjectMenuItem_Click(object sender, RoutedEventArgs e) {
            if (sender is MenuItem menuItem && menuItem.CommandParameter is Tuple<string, string> parameters) {
                string fullName = parameters.Item1;
                string projectName = parameters.Item2;

                MoveDocumentToProjectGroup(fullName, projectName);
            }
        }

        private void PinDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                // Проверяем, находится ли документ в режиме предварительного просмотра
                var previewDoc = PreviewDocuments.FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
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
                    var shellDocument = new ShellDocument(document);

                    // Активируем документ
                    shellDocument.Document.Activate();
                    _activeShellDocument = shellDocument;

                    // Сбрасываем выделение во всех группах кроме активного
                    this.DeselectNonActiveDocuments();

                    // Логируем основные поля документа
                    Helpers.Diagnostic.Logger.LogDebug("=== Document Selected ===");
                    Helpers.Diagnostic.Logger.LogDebug($"Name: {shellDocument.Document.Name}");
                    Helpers.Diagnostic.Logger.LogDebug($"FullName: {shellDocument.Document.FullName}");
                    Helpers.Diagnostic.Logger.LogDebug($"Path: {Path.GetDirectoryName(shellDocument.Document.FullName)}");
                    Helpers.Diagnostic.Logger.LogDebug($"ProjectName (Primary): {docInfo.ProjectName}");

                    // Проверяем принадлежность к нескольким проектам (Shared Project)
                    var projects = shellDocument.GetDocumentProjects();
                    Helpers.Diagnostic.Logger.LogDebug($"Projects ({projects.Count}):");
                    foreach (var project in projects) {
                        Helpers.Diagnostic.Logger.LogDebug($" - {project.Name}");
                    }
                }
            }
        }

        private void CloseDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                var document = _dte.Documents.Cast<Document>().FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
                if (document != null) {
                    document.Close();
                }
            }
        }


        private void ScaleSelectorControl_ScaleChanged(object sender, double scaleFactor) {
            _currentScaleFactor = scaleFactor;
            ApplyDocumentScale();
        }

        private void ApplyDocumentScale() {
            DocumentScaleTransform.ScaleX = _currentScaleFactor;
            DocumentScaleTransform.ScaleY = _currentScaleFactor;
        }


        //
        // Internal logic
        //
        private void LoadOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.GroupedDocuments.Clear();
            foreach (Document doc in _dte.Documents) {
                this.AddDocumentToGroup(new ShellDocument(doc));
            }
        }

        private void AddDocumentToPreview(ShellDocument shellDocument) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, нет ли уже этого документа в Preview
            var existing = this.PreviewDocuments.FirstOrDefault(d => string.Equals(d.FullName, shellDocument.Document.FullName, StringComparison.OrdinalIgnoreCase));
            if (existing == null) {
                this.PreviewDocuments.Clear(); // Только один документ в режиме предварительного просмотра
                this.PreviewDocuments.Add(new DocumentInfo(shellDocument));
            }
        }
        private void RemoveDocumentFromPreview(string fullName) {
            var doc = this.PreviewDocuments.FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
            if (doc != null) {
                this.PreviewDocuments.Remove(doc);
            }
        }


        private void AddDocumentToGroup(ShellDocument shellDocument) {
            ThreadHelper.ThrowIfNotOnUIThread();
            string projectName = shellDocument.GetDocumentProjectName();

            // Ищем или создаем группу
            var group = this.GroupedDocuments.FirstOrDefault(g => string.Equals(g.Name, projectName, StringComparison.OrdinalIgnoreCase));
            if (group == null) {
                group = new ProjectGroup { Name = projectName };
                this.GroupedDocuments.Add(group);
                this.SortGroups();
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == shellDocument.Document.FullName)) {
                group.Items.Add(new DocumentInfo(shellDocument));
                this.SortDocumentsInGroups();
            }
        }

        private void MoveDocumentToGroup(DocumentInfo docInfo) {
            string projectName = docInfo.ProjectName;

            // Ищем или создаем группу
            var group = this.GroupedDocuments.FirstOrDefault(g => string.Equals(g.Name, projectName, StringComparison.OrdinalIgnoreCase));
            if (group == null) {
                group = new ProjectGroup { Name = projectName };
                this.GroupedDocuments.Add(group);
                this.SortGroups();
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == docInfo.FullName)) {
                group.Items.Add(docInfo);
                this.SortDocumentsInGroups();
            }
        }

        private void MoveDocumentToProjectGroup(string fullName, string projectName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Ищем документ
            var docInfo = this.GroupedDocuments.SelectMany(g => g.Items).FirstOrDefault(d => d.FullName == fullName);
            if (docInfo == null) {
                return;
            }

            // Перемещаем документ в выбранную группу
            this.RemoveDocumentFromGroup(fullName);

            // Создаем или находим группу
            var group = this.GroupedDocuments.FirstOrDefault(g => g.Name == projectName);
            if (group == null) {
                group = new ProjectGroup { Name = projectName };
                this.GroupedDocuments.Add(group);
                this.SortGroups();
            }

            // Добавляем документ в выбранную группу
            group.Items.Add(docInfo);
            this.SortDocumentsInGroups();
        }


        private void RemoveDocumentFromGroup(string fullName) {
            ThreadHelper.ThrowIfNotOnUIThread(); 
            
            foreach (var group in this.GroupedDocuments.ToList()) {
                var docToRemove = group.Items.FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
                if (docToRemove != null) {
                    group.Items.Remove(docToRemove);
                    if (group.Items.Count == 0) {
                        this.GroupedDocuments.Remove(group);
                    }
                    break;
                }
            }
        }
        

        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in this.GroupedDocuments) {
                var docInfo = group.Items.FirstOrDefault(d => string.Equals(d.FullName, oldPath, StringComparison.OrdinalIgnoreCase));
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

        // Метод сортировки документов внутри групп
        private void SortDocumentsInGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.GroupedDocuments) {
                var sortedDocs = group.Items.OrderBy(d => d.DisplayName).ToList();
                group.Items.Clear();
                foreach (var doc in sortedDocs) {
                    group.Items.Add(doc);
                }
            }
        }

        private void SortGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Сортируем группы по имени (по алфавиту)
            var sortedGroups = this.GroupedDocuments.OrderBy(g => g.Name).ToList();

            // Очищаем и добавляем отсортированные группы
            this.GroupedDocuments.Clear();
            foreach (var group in sortedGroups) {
                this.GroupedDocuments.Add(group);
            }
        }


        private void OpenDocumentAsPinned(string documentFullPath) {
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

        private void DeselectNonActiveDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Находим группу, содержащую активный документ
            var activeGroup = this.GroupedDocuments
                .FirstOrDefault(g => g.Items.Any(d => d.FullName == _activeShellDocument.Document.FullName));

            // Проходим по всем группам
            foreach (var group in this.GroupedDocuments) {
                if (group == activeGroup) {
                    continue; // Пропускаем группу с активным документом
                }

                foreach (var docInfo in group.Items) {
                    var listBox = this.FindListBoxContainingDocument(docInfo.FullName);
                    if (listBox != null) {
                        listBox.SelectedItem = null;
                    }
                }
            }
        }

        private ListBox FindListBoxContainingDocument(string fullName) {
            var listBoxes = Helpers.UI.FindVisualChildren<ListBox>(this);

            foreach (var listBox in listBoxes) {
                if (listBox.ItemsSource is IEnumerable<DocumentInfo> documents) {
                    if (documents.Any(d => d.FullName == fullName)) {
                        return listBox;
                    }
                }
            }

            return null;
        }


        private bool IsTemporaryFile(string fullPath) {
            string extension = Path.GetExtension(fullPath);
            return extension.Equals(".TMP", StringComparison.OrdinalIgnoreCase) ||
                   fullPath.Contains("~") && fullPath.Contains(".TMP");
        }
        private string GetSolutionDirectory() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var solution = _dte.Solution;
            return string.IsNullOrEmpty(solution.FullName) ? null : Path.GetDirectoryName(solution.FullName);
        }
    }
}