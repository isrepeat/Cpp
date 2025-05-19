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
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

using TabsManagerExtension.Helpers.Ex;

namespace TabsManagerExtension {
    public partial class TabsManagerToolWindowControl : UserControl, INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        // [Events]:
        private EnvDTE80.DTE2 _dte;
        private EnvDTE.WindowEvents _windowEvents;
        private EnvDTE.DocumentEvents _documentEvents;
        private EnvDTE.SolutionEvents _solutionEvents;
        
        // [Timeres]:
        private FileSystemWatcher _fileWatcher;
        private DispatcherTimer _saveStateCheckTimer;

        // [Document collections]:
        public ObservableCollection<DocumentInfo> PreviewDocuments { get; set; } = new ObservableCollection<DocumentInfo>();
        public ObservableCollection<ProjectGroup> GroupedDocuments { get; set; } = new ObservableCollection<ProjectGroup>();
        private CollectionViewSource GroupedDocumentsViewSource { get; set; }

        private DocumentInfo _activeDocument;


        private double _scaleFactor = 0.9;
        public double ScaleFactor {
            get => _scaleFactor;
            set {
                if (_scaleFactor != value) {
                    _scaleFactor = value;
                    OnPropertyChanged();
                    ApplyDocumentScale();
                }
            }
        }

        public TabsManagerToolWindowControl() {
            InitializeComponent();
            InitializeDTE();
            InitializeFileWatcher();
            InitializeTimers();
            InitializeDocumentsViewSource();

            base.DataContext = this;
            LoadOpenDocuments();
        }

        private void InitializeDTE() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));

            _documentEvents = _dte.Events.DocumentEvents;
            _documentEvents.DocumentOpened += DocumentOpenedHandler;
            _documentEvents.DocumentSaved += DocumentSavedHandler;
            _documentEvents.DocumentClosing += DocumentClosingHandler;

            // Подписываемся на событие смены активного окна (вкладки)
            _windowEvents = _dte.Events.WindowEvents;
            _windowEvents.WindowActivated += OnWindowActivated;

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
            this.GroupedDocumentsViewSource = new CollectionViewSource { Source = GroupedDocuments };

            // Сортировка групп по имени (по алфавиту)
            this.GroupedDocumentsViewSource.SortDescriptions.Add(new SortDescription("Name", ListSortDirection.Ascending));

            // Применяем сортировку внутри групп через метод
            this.GroupedDocumentsViewSource.View.CollectionChanged += (s, e) => SortDocumentsInGroups();
        }


        //
        // Event handlers
        //
        private void DocumentOpenedHandler(EnvDTE.Document document) {
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

        private void DocumentSavedHandler(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.SaveStateCheckTimerHandler(null, null);
        }

        private void DocumentClosingHandler(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.RemoveDocumentFromPreview(document.FullName);
            this.RemoveDocumentFromGroup(document.FullName);
        }

        private void OnWindowActivated(EnvDTE.Window gotFocus, EnvDTE.Window lostFocus) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"OnWindowActivated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, что окно имеет связанный документ
            if (gotFocus?.Document != null) {
                this.ActivateDocument(gotFocus.Document);
            }
        }

        private void OnSolutionClosing() {
            GroupedDocuments.Clear();
            _fileWatcher?.Dispose();
        }


        // Обработчик изменения файла (без TMP-файлов)
        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            if (this.IsTemporaryFile(e.FullPath)) {
                return; // Игнорируем временные файлы
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.UpdateDocumentUI(e.FullPath);
            });
        }

        // Обработчик переименования файла (без TMP-файлов)
        private void OnFileRenamed(object sender, RenamedEventArgs e) {
            if (this.IsTemporaryFile(e.FullPath) || this.IsTemporaryFile(e.OldFullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.UpdateDocumentUI(e.OldFullPath, e.FullPath);
            });
        }

        // Обработчик удаления файла (без TMP-файлов)
        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            if (this.IsTemporaryFile(e.FullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.RemoveDocumentFromGroup(e.FullPath);
            });
        }

        private void SaveStateCheckTimerHandler(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Создаем копию для безопасного перебора
            var groupsCopy = GroupedDocuments.ToList();

            foreach (var group in groupsCopy) {
                var itemsCopy = group.Items.ToList();

                foreach (var docInfo in itemsCopy) {
                    var document = _dte.Documents.Cast<EnvDTE.Document>().FirstOrDefault(d => d.FullName == docInfo.FullName);
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
        private void InteractiveArea_MouseEnter(object sender, MouseEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseEnter()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is FrameworkElement interactiveArea) {
                // Находим родительский ListBoxItem (где привязаны данные)
                var listBoxItem = FindParent<ListBoxItem>(interactiveArea);
                if (listBoxItem == null) return;

                // Получаем привязанный объект (DocumentInfo)
                if (listBoxItem.DataContext is DocumentInfo documentInfo) {
                    var position = interactiveArea.PointToScreen(new Point(interactiveArea.ActualWidth + 0, 0));
                    var mainWindow = Application.Current.MainWindow;
                    var relativePoint = mainWindow.PointFromScreen(position);

                    if (documentInfo.ShellDocument != null) {
                        documentInfo.UpdateProjectReferenceList();
                    }

                    MyVirtualPopup.InteractiveArea_MouseEnter();
                    MyVirtualPopup.ShowPopup(relativePoint, documentInfo);
                }
            }
        }

        private void InteractiveArea_MouseLeave(object sender, MouseEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseLeave()");

            // Проверяем, существует ли Popup и уведомляем о покидании области
            MyVirtualPopup.InteractiveArea_MouseLeave();
        }


        // Универсальный метод поиска родителя
        private T FindParent<T>(DependencyObject child) where T : DependencyObject {
            while (child != null) {
                if (child is T parent)
                    return parent;

                child = VisualTreeHelper.GetParent(child);
            }
            return null;
        }


        private void ProjectMenuItem_Click(object sender, RoutedEventArgs e) {
            if (sender is Button button && button.CommandParameter is DocumentProjectReferenceInfo documentProjectReferenceInfo) {
                MoveDocumentToProjectGroup(
                    documentProjectReferenceInfo.DocumentInfo.FullName,
                    documentProjectReferenceInfo.ProjectInfo.Name
                    );
            }
        }

        private void KeepDocumentOpen_Click(object sender, RoutedEventArgs e) {
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

        private void PinDocument_Click(object sender, RoutedEventArgs e) {
        }
        private void CloseDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                var document = _dte.Documents.Cast<EnvDTE.Document>().FirstOrDefault(d => string.Equals(d.FullName, fullName, StringComparison.OrdinalIgnoreCase));
                if (document != null) {
                    document.Close();
                }
            }
        }

        private void DocumentList_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is ListBox listBox && listBox.SelectedItem is DocumentInfo docInfo) {
                var document = _dte.Documents.Cast<EnvDTE.Document>()
                    .FirstOrDefault(d => string.Equals(d.FullName, docInfo.FullName, StringComparison.OrdinalIgnoreCase));

                if (document != null) {
                    if (this.ActivateDocument(document)) {
                        document.Activate();
                    }
                }
            }
        }


        private void ScaleSelectorControl_ScaleChanged(object sender, double scaleFactor) {
            ApplyDocumentScale();
        }

        private void ApplyDocumentScale() {
            if (DocumentScaleTransform != null) {
                DocumentScaleTransform.ScaleX = ScaleFactor;
                DocumentScaleTransform.ScaleY = ScaleFactor;
            }
        }


        //
        // Internal logic
        //
        private void LoadOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.GroupedDocuments.Clear();
            foreach (EnvDTE.Document doc in _dte.Documents) {
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
        private void RemoveDocumentFromPreview(string documentFullName) {
            var doc = this.PreviewDocuments.FirstOrDefault(d => string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
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

        private void MoveDocumentToProjectGroup(string documentFullName, string projectName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Ищем документ
            var docInfo = this.GroupedDocuments.SelectMany(g => g.Items).FirstOrDefault(d => d.FullName == documentFullName);
            if (docInfo == null) {
                return;
            }

            // Перемещаем документ в выбранную группу
            this.RemoveDocumentFromGroup(documentFullName);

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


        private void RemoveDocumentFromGroup(string documentFullName) {
            ThreadHelper.ThrowIfNotOnUIThread(); 
            
            foreach (var group in this.GroupedDocuments.ToList()) {
                var docToRemove = group.Items.FirstOrDefault(d => string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
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

        private bool ActivateDocument(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, уже ли этот документ активен
            if (_activeDocument != null && string.Equals(_activeDocument.FullName, document.FullName, StringComparison.OrdinalIgnoreCase)) {
                return false;
            }
            _activeDocument = new DocumentInfo(new ShellDocument(document));

            this.SelectDocumentInUI(_activeDocument.ShellDocument.Document);
            this.DeselectNonActiveDocuments();
            return true;
        }


        // Метод выбора документа в UI ListBox (без активации)
        private void SelectDocumentInUI(EnvDTE.Document document) {
            foreach (var group in GroupedDocuments) {
                var activeDoc = group.Items.FirstOrDefault(d => d.FullName == document.FullName);
                if (activeDoc != null) {
                    var listBox = FindListBoxContainingDocument(document.FullName);
                    if (listBox != null) {
                        listBox.SelectedItem = activeDoc;
                    }
                    return;
                }
            }
        }

        private void DeselectNonActiveDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Находим группу, содержащую активный документ
            var activeGroup = this.GroupedDocuments
                .FirstOrDefault(g => g.Items.Any(d => d.FullName == _activeDocument.FullName));

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

        private ListBox FindListBoxContainingDocument(string documentFullName) {
            var listBoxes = Helpers.UI.FindVisualChildren<ListBox>(this);

            foreach (var listBox in listBoxes) {
                if (listBox.ItemsSource is IEnumerable<DocumentInfo> documents) {
                    if (documents.Any(d => d.FullName == documentFullName)) {
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



        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}