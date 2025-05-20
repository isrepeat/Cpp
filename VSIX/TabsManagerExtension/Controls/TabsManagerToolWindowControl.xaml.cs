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
        public ObservableCollection<TabItemDocument> PreviewDocuments { get; set; } = new ObservableCollection<TabItemDocument>();
        public ObservableCollection<TabItemGroup> GroupedTabItems { get; set; } = new ObservableCollection<TabItemGroup>();
        private CollectionViewSource GroupedTabItemsViewSource { get; set; }

        private TabItemBase _activeTabItem;


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
            _documentEvents.DocumentOpened += OnDocumentOpened;
            _documentEvents.DocumentSaved += OnDocumentSaved;
            _documentEvents.DocumentClosing += OnDocumentClosing;

            _windowEvents = _dte.Events.WindowEvents;
            _windowEvents.WindowCreated += OnWindowCreated;
            _windowEvents.WindowActivated += OnWindowActivated;
            _windowEvents.WindowClosing += OnWindowClosing;

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
            this.GroupedTabItemsViewSource = new CollectionViewSource { Source = this.GroupedTabItems };

            // Сортировка групп по имени (по алфавиту)
            this.GroupedTabItemsViewSource.SortDescriptions.Add(new SortDescription("Name", ListSortDirection.Ascending));

            // Применяем сортировку внутри групп через метод
            this.GroupedTabItemsViewSource.View.CollectionChanged += (s, e) => SortDocumentsInGroups();
        }


        //
        // Event handlers
        //
        private void OnDocumentOpened(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentOpened()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | document.Name = {document.Name}");

            var shellDocument = new ShellDocument(document);
            if (shellDocument.IsDocumentInPreviewTab()) {
                this.AddDocumentToPreview(shellDocument);
            }
            else {
                this.AddDocumentToGroup(shellDocument);
            }
        }


        private void OnDocumentSaved(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentSaved()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | document.Name = {document.Name}");

            this.SaveStateCheckTimerHandler(null, null);
        }


        private void OnDocumentClosing(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | document.Name = {document.Name}");

            this.RemoveDocumentFromPreview(document.FullName);
            this.RemoveDocumentFromGroup(document.FullName);
        }


        private void OnWindowActivated(EnvDTE.Window gotFocus, EnvDTE.Window lostFocus) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowActivated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | gotFocus.Caption = {gotFocus.Caption}");
            Helpers.Diagnostic.Logger.LogDebug($"  | lostFocus.Caption = {lostFocus.Caption}");

            if (gotFocus == null) {
                Helpers.Diagnostic.Logger.LogDebug($"gotFocus == null");
                return;
            }

            var activatedShellWindow = new ShellWindow(gotFocus);
            if (!activatedShellWindow.IsTabWindow()) {
                Helpers.Diagnostic.Logger.LogDebug($"Skip non tab window - \"{activatedShellWindow.Window.Caption}\"");
                return;
            }

            TabItemBase tabItem;
            if (activatedShellWindow.Window.Document != null) {
                tabItem = new TabItemDocument(new ShellDocument(activatedShellWindow.Window.Document));
            }
            else {
                tabItem = new TabItemWindow(activatedShellWindow.Window);
            }

            this.AddTabItemToGroupIfMissing(tabItem);
            this.SelectTabItem(tabItem);
        }


        private void OnWindowCreated(EnvDTE.Window createdWindow) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowCreated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | createdWindow.Caption = {createdWindow.Caption}");

            //if (createdWindow?.Document == null) {
            //    AddWindowToGroup(createdWindow);
            //}
        }


        private void OnWindowClosing(EnvDTE.Window closingWindow) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogDebug($"  | closingWindow.Caption = {closingWindow.Caption}");
        }


        private void OnSolutionClosing() {
            this.GroupedTabItems.Clear();
            _fileWatcher?.Dispose();
        }


        // Обработчик изменения файла (без TMP-файлов)
        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            //using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileChanged()");

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
            //using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileRenamed()");

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
            //using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileDeleted()");

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
            var groupedTabItemsCopy = this.GroupedTabItems.ToList();

            foreach (var tabItemGroup in groupedTabItemsCopy) {
                var tabItemsCopy = tabItemGroup.Items.ToList();

                foreach (var tabItem in tabItemsCopy) {
                    var document = _dte.Documents.Cast<EnvDTE.Document>().FirstOrDefault(d => d.FullName == tabItem.FullName);
                    if (document != null) {
                        if (document.Saved) {
                            tabItem.Name = tabItem.Name.TrimEnd('*');
                        }
                        else {
                            if (!tabItem.Name.EndsWith("*")) {
                                tabItem.Name += "*";
                            }
                        }
                    }
                }
            }


            foreach (var tabItemDocument in this.PreviewDocuments.ToList()) {
                if (!tabItemDocument.ShellDocument.IsDocumentInPreviewTab()) {
                    // Перемещаем документ из предварительного просмотра в основную группу
                    this.MoveDocumentToGroup(tabItemDocument);
                    this.PreviewDocuments.Remove(tabItemDocument);
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
                if (listBoxItem.DataContext is TabItemDocument tabItemDocument) {
                    var position = interactiveArea.PointToScreen(new Point(interactiveArea.ActualWidth + 0, 0));
                    var mainWindow = Application.Current.MainWindow;
                    var relativePoint = mainWindow.PointFromScreen(position);

                    if (tabItemDocument.ShellDocument != null) {
                        tabItemDocument.UpdateProjectReferenceList();
                    }

                    MyVirtualPopup.InteractiveArea_MouseEnter();
                    MyVirtualPopup.ShowPopup(relativePoint, tabItemDocument);
                }
            }
        }

        private void InteractiveArea_MouseLeave(object sender, MouseEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseLeave()");

            // Проверяем, существует ли Popup и уведомляем о покидании области
            this.MyVirtualPopup.InteractiveArea_MouseLeave();
        }


        // Универсальный метод поиска родителя
        private T FindParent<T>(DependencyObject child) where T : DependencyObject {
            while (child != null) {
                if (child is T parent) {
                    return parent;
                }
                child = VisualTreeHelper.GetParent(child);
            }
            return null;
        }


        private void ProjectMenuItem_Click(object sender, RoutedEventArgs e) {
            if (sender is Button button && button.CommandParameter is DocumentProjectReferenceInfo documentProjectReferenceInfo) {
                this.MoveDocumentToProjectGroup(
                    documentProjectReferenceInfo.TabItemDocument.FullName,
                    documentProjectReferenceInfo.TabItemProject.Name
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
                    this.OpenDocumentAsPinned(fullName);
                    // Перемещаем документ в основную группу
                    this.MoveDocumentToGroup(previewDoc);
                    this.PreviewDocuments.Remove(previewDoc);
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

            if (sender is ListBox listBox && listBox.SelectedItem is TabItemBase tabItem) {
                this.ActivateTabItem(tabItem); // И активируем, и обновляем UI
            }
        }

        private void ScaleSelectorControl_ScaleChanged(object sender, double scaleFactor) {
            this.ApplyDocumentScale();
        }

        private void ApplyDocumentScale() {
            if (this.DocumentScaleTransform != null) {
                this.DocumentScaleTransform.ScaleX = this.ScaleFactor;
                this.DocumentScaleTransform.ScaleY = this.ScaleFactor;
            }
        }


        //
        // Internal logic
        //
        private void LoadOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.GroupedTabItems.Clear();
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
                this.PreviewDocuments.Add(new TabItemDocument(shellDocument));
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
            var group = this.GroupedTabItems.FirstOrDefault(g => string.Equals(g.Name, projectName, StringComparison.OrdinalIgnoreCase));
            if (group == null) {
                group = new TabItemGroup { Name = projectName };
                this.GroupedTabItems.Add(group);
                this.SortGroups();
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == shellDocument.Document.FullName)) {
                group.Items.Add(new TabItemDocument(shellDocument));
                this.SortDocumentsInGroups();
            }
        }

        private void AddTabItemToGroupIfMissing(TabItemBase item) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Уже есть такой документ — не добавляем
            bool alreadyExists = this.GroupedTabItems
                .SelectMany(g => g.Items)
                .Any(x => string.Equals(x.FullName, item.FullName, StringComparison.OrdinalIgnoreCase));

            if (alreadyExists)
                return;

            string groupName;

            if (item is TabItemDocument doc) {
                groupName = doc.ShellDocument.GetDocumentProjectName();
            }
            else if (item is TabItemWindow) {
                groupName = "Windows";
            }
            else if (item is TabItemProject proj) {
                groupName = proj.ShellProject.Project.Name;
            }
            else {
                groupName = "Other";
            }

            var group = this.GroupedTabItems.FirstOrDefault(g => g.Name == groupName);
            if (group == null) {
                group = new TabItemGroup { Name = groupName };
                this.GroupedTabItems.Add(group);
            }

            group.Items.Add(item);
            this.SortDocumentsInGroups();
        }


        private void AddWindowToGroup(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();
            // TODO: implement
        }

        private void MoveDocumentToGroup(TabItemDocument tabItemDocument) {
            //string projectName = docInfo.ProjectName;
            string projectName = tabItemDocument.ShellDocument.GetDocumentProjectName();

            // Ищем или создаем группу
            var group = this.GroupedTabItems.FirstOrDefault(g => string.Equals(g.Name, projectName, StringComparison.OrdinalIgnoreCase));
            if (group == null) {
                group = new TabItemGroup { Name = projectName };
                this.GroupedTabItems.Add(group);
                this.SortGroups();
            }

            // Проверяем, что документа еще нет в группе
            if (!group.Items.Any(d => d.FullName == tabItemDocument.FullName)) {
                group.Items.Add(tabItemDocument);
                this.SortDocumentsInGroups();
            }
        }

        private void MoveDocumentToProjectGroup(string documentFullName, string projectName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Ищем документ
            var docInfo = this.GroupedTabItems.SelectMany(g => g.Items).FirstOrDefault(d => d.FullName == documentFullName);
            if (docInfo == null) {
                return;
            }

            // Перемещаем документ в выбранную группу
            this.RemoveDocumentFromGroup(documentFullName);

            // Создаем или находим группу
            var group = this.GroupedTabItems.FirstOrDefault(g => g.Name == projectName);
            if (group == null) {
                group = new TabItemGroup { Name = projectName };
                this.GroupedTabItems.Add(group);
                this.SortGroups();
            }

            // Добавляем документ в выбранную группу
            group.Items.Add(docInfo);
            this.SortDocumentsInGroups();
        }


        private void RemoveDocumentFromGroup(string documentFullName) {
            ThreadHelper.ThrowIfNotOnUIThread(); 
            
            foreach (var group in this.GroupedTabItems.ToList()) {
                var docToRemove = group.Items.FirstOrDefault(d => string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
                if (docToRemove != null) {
                    group.Items.Remove(docToRemove);
                    if (group.Items.Count == 0) {
                        this.GroupedTabItems.Remove(group);
                    }
                    break;
                }
            }
        }


        private void ActivateTabItem(TabItemBase tabItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (tabItem is IActivatableTab activatable) {
                // Важно: откладываем вызов .Activate() до следующего тика UI-диспетчера,
                // чтобы избежать COMException при активации окна/документа из обработчика SelectionChanged
                Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                    activatable.Activate();
                }), DispatcherPriority.Background);
            }

            this.SelectTabItem(tabItem);
        }

        private void SelectTabItem(TabItemBase tabItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_activeTabItem != null &&
                string.Equals(_activeTabItem.FullName, tabItem.FullName, StringComparison.OrdinalIgnoreCase)) {
                return;
            }

            _activeTabItem = tabItem;

            this.SelectTabItemInUI(_activeTabItem);
            this.DeselectNonActiveTabItems();
        }

        private void SelectTabItemInUI(TabItemBase tabItem) {
            foreach (var group in this.GroupedTabItems) {
                var match = group.Items.FirstOrDefault(d => d.FullName == tabItem.FullName);
                if (match != null) {
                    var listBox = this.FindListBoxContainingDocument(tabItem.FullName);
                    if (listBox != null) {
                        listBox.SelectedItem = match;
                    }
                    return;
                }
            }
        }

        private void DeselectNonActiveTabItems() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var activeGroup = this.GroupedTabItems
                .FirstOrDefault(g => g.Items.Any(d => d.FullName == _activeTabItem.FullName));

            foreach (var group in this.GroupedTabItems) {
                if (group == activeGroup)
                    continue;

                foreach (var item in group.Items) {
                    var listBox = this.FindListBoxContainingDocument(item.FullName);
                    if (listBox != null) {
                        listBox.SelectedItem = null;
                    }
                }
            }
        }




        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in this.GroupedTabItems) {
                var docInfo = group.Items.FirstOrDefault(d => string.Equals(d.FullName, oldPath, StringComparison.OrdinalIgnoreCase));
                if (docInfo != null) {
                    if (newPath == null) {
                        // Обновляем только имя (в случае изменения)
                        docInfo.Name = Path.GetFileName(oldPath);
                    }
                    else {
                        // Обновляем имя и полный путь (в случае переименования)
                        docInfo.FullName = newPath;
                        docInfo.Name = Path.GetFileName(newPath);
                    }
                    return;
                }
            }
        }

        // Метод сортировки документов внутри групп
        private void SortDocumentsInGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.GroupedTabItems) {
                var sortedDocs = group.Items.OrderBy(d => d.Name).ToList();
                group.Items.Clear();
                foreach (var doc in sortedDocs) {
                    group.Items.Add(doc);
                }
            }
        }

        private void SortGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Сортируем группы по имени (по алфавиту)
            var sortedGroups = this.GroupedTabItems.OrderBy(g => g.Name).ToList();

            // Очищаем и добавляем отсортированные группы
            this.GroupedTabItems.Clear();
            foreach (var group in sortedGroups) {
                this.GroupedTabItems.Add(group);
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





        private ListBox FindListBoxContainingDocument(string fullName) {
            var listBoxes = Helpers.UI.FindVisualChildren<ListBox>(this);

            foreach (var listBox in listBoxes) {
                foreach (var item in listBox.Items) {
                    if (item is TabItemBase tab && tab.FullName == fullName) {
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