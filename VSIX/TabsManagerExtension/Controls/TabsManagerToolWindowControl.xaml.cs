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
        private DispatcherTimer _tabsManagerStateTimer;

        // [Document collections]:
        public ObservableCollection<TabItemDocument> PreviewDocuments { get; set; } = new ObservableCollection<TabItemDocument>();
        public ObservableCollection<TabItemGroup> SortedTabItemGroups { get; set; } = new ObservableCollection<TabItemGroup>();
        private CollectionViewSource SortedTabItemGroupsViewSource { get; set; }

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
            _tabsManagerStateTimer = new DispatcherTimer();
            _tabsManagerStateTimer.Interval = TimeSpan.FromMilliseconds(200);
            _tabsManagerStateTimer.Tick += TabsManagerStateTimerHandler;
            _tabsManagerStateTimer.Start();
        }

        public void InitializeDocumentsViewSource() {
            // Создаем представление с сортировкой
            this.SortedTabItemGroupsViewSource = new CollectionViewSource { Source = this.SortedTabItemGroups };

            // Сортировка групп по имени (по алфавиту)
            this.SortedTabItemGroupsViewSource.SortDescriptions.Add(new SortDescription("GroupName", ListSortDirection.Ascending));

            // Применяем сортировку внутри групп через метод
            this.SortedTabItemGroupsViewSource.View.CollectionChanged += (s, e) => SortDocumentsInGroups();
        }


        //
        // Event handlers
        //
        private void OnDocumentOpened(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentOpened()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document.Name}");

            var tabItemDocument = this.FindTabItem(document);
            if (tabItemDocument == null) {
                tabItemDocument = new TabItemDocument(document);
            }

            if (tabItemDocument.ShellDocument.IsDocumentInPreviewTab()) {
                this.AddDocumentToPreview(tabItemDocument);
            }
            else {
                this.AddTabItemToDefaultGroupIfMissing(tabItemDocument);
            }
        }


        private void OnDocumentSaved(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentSaved()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document.Name}");

            this.TabsManagerStateTimerHandler(null, null);
        }


        private void OnDocumentClosing(EnvDTE.Document document) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document.Name}");

            var tabItemDocument = this.FindTabItem(document);
            if (tabItemDocument != null) {
                this.RemoveDocumentFromPreview(tabItemDocument);
                this.RemoveTabItemFromGroups(tabItemDocument);
            }
            else {
                Helpers.Diagnostic.Logger.LogError($"TabItemDocument not found in collections");
            }
        }


        private void OnWindowActivated(EnvDTE.Window gotFocus, EnvDTE.Window lostFocus) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowActivated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"gotFocus.Caption = {gotFocus?.Caption}");
            Helpers.Diagnostic.Logger.LogParam($"lostFocus.Caption = {lostFocus?.Caption}");

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
                tabItem = new TabItemDocument(activatedShellWindow.Window.Document);
            }
            else {
                tabItem = new TabItemWindow(activatedShellWindow);
            }

            this.AddTabItemToDefaultGroupIfMissing(tabItem);
            this.UpdateWindowTabs();
            this.SelectTabItem(tabItem);
        }


        private void OnWindowClosing(EnvDTE.Window closingWindow) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"closingWindow.Caption = {closingWindow.Caption}");
        }


        private void OnSolutionClosing() {
            this.SortedTabItemGroups.Clear();
            _fileWatcher?.Dispose();
        }


        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            //using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileChanged()");

            if (this.IsTemporaryFile(e.FullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.UpdateDocumentUI(e.FullPath);
            });
        }

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

        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            //using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileDeleted()");

            if (this.IsTemporaryFile(e.FullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var documentFullName = e.FullPath;
                var tabItemDocument = this.FindTabItem(documentFullName);
                if (tabItemDocument != null) {
                    this.RemoveTabItemFromGroups(tabItemDocument);
                }
            });
        }


        private void TabsManagerStateTimerHandler(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            // NOTE: Нужно использовать копии коллекций для безопасного перебора.
            // Поэтому в foreach вызывай .ToList() у коллекций.

            // === [A] Обновление статуса сохранения документов ===
            foreach (var tabItemGroup in this.SortedTabItemGroups.ToList()) {
                foreach (var tabItem in tabItemGroup.TabItems.ToList()) {
                    var document = _dte.Documents.Cast<EnvDTE.Document>()
                        .FirstOrDefault(d => d.FullName == tabItem.FullName);

                    if (document != null) {
                        if (document.Saved) {
                            tabItem.Caption = tabItem.Caption.TrimEnd('*');
                        }
                        else {
                            if (!tabItem.Caption.EndsWith("*")) {
                                tabItem.Caption += "*";
                            }
                        }
                    }
                }
            }


            // === [B] Перемещение preview-документов в основную группу ===
            foreach (var tabItemDocument in this.PreviewDocuments.ToList()) {
                if (!tabItemDocument.ShellDocument.IsDocumentInPreviewTab()) {
                    this.MoveDocumentFromPreviewToMainGroup(tabItemDocument);
                }
            }


            // === [C] Удаление закрытых окон типа TabItemWindow ===
            var openWindowIds = new HashSet<string>();

            try {
                openWindowIds = _dte.Windows
                    .Cast<EnvDTE.Window>() // Приводим COM-коллекцию к типизированной, чтобы использовать LINQ
                    .Select(w => ShellWindow.GetWindowId(w))
                    .Where(id => !string.IsNullOrEmpty(id))
                    .ToHashSet();
            }
            catch (Exception ex) {
                // Иногда обращение к _dte.Windows может выбросить COMException,особенно если окно закрывается
                // или COM-объект уже недоступен. Поэтому оборачиваем перебор в try-catch для устойчивости.
                Helpers.Diagnostic.Logger.LogError($"Failed to enumerate windows: {ex.Message}");
            }

            foreach (var group in this.SortedTabItemGroups.ToList()) {
                var toRemove = group.TabItems
                    .OfType<TabItemWindow>()
                    .Where(w => !openWindowIds.Contains(w.WindowId))
                    .ToList();

                foreach (var tab in toRemove) {
                    this.RemoveTabItemFromGroups(tab);
                }
            }

            // === [D] Обновление окон типа TabItemWindow ===
            this.UpdateWindowTabs();
        }



        //
        // UI click handlers
        //
        private void InteractiveArea_MouseEnter(object sender, MouseEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseEnter()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is FrameworkElement interactiveArea) {
                // Находим родительский ListBoxItem (где привязаны данные)
                var listBoxItem = Helpers.UI.FindParent<ListBoxItem>(interactiveArea);
                if (listBoxItem == null) return;

                // Получаем привязанный объект (DocumentInfo)
                if (listBoxItem.DataContext is TabItemDocument tabItemDocument) {
                    var position = interactiveArea.PointToScreen(new Point(interactiveArea.ActualWidth + 0, 0));
                    var mainWindow = Application.Current.MainWindow;
                    var relativePoint = mainWindow.PointFromScreen(position);

                    if (tabItemDocument.ShellDocument != null) {
                        tabItemDocument.UpdateProjectReferenceList();
                    }

                    this.MyVirtualPopup.InteractiveArea_MouseEnter();
                    this.MyVirtualPopup.ShowPopup(relativePoint, tabItemDocument);
                }
            }
        }

        private void InteractiveArea_MouseLeave(object sender, MouseEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseLeave()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.MyVirtualPopup.InteractiveArea_MouseLeave();
        }


        private void ProjectMenuItem_Click(object sender, RoutedEventArgs e) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("ProjectMenuItem_Click()");

            if (sender is Button button && button.CommandParameter is DocumentProjectReferenceInfo documentProjectReferenceInfo) {
                this.MoveDocumentToProjectGroup(
                    documentProjectReferenceInfo.TabItemDocument,
                    documentProjectReferenceInfo.TabItemProject
                    );
            }
        }

        private void KeepTabOpen_Click(object sender, RoutedEventArgs e) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("KeepTabOpen_Click()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is Button button && button.CommandParameter is TabItemBase tabItem) {
                if (tabItem is TabItemDocument tabItemDocument) {
                    if (this.PreviewDocuments.Contains(tabItemDocument)) {
                        this.MoveDocumentFromPreviewToMainGroup(tabItemDocument);
                        tabItemDocument.ShellDocument.OpenDocumentAsPinned();
                    }
                }
            }
        }

        private void PinTab_Click(object sender, RoutedEventArgs e) {
        }

        private void CloseTab_Click(object sender, RoutedEventArgs e) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("CloseTab_Click()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is Button button && button.CommandParameter is TabItemBase tabItem) {
                if (tabItem is TabItemDocument tabItemDocument) {
                    Helpers.Diagnostic.Logger.LogDebug($"close document \"{tabItemDocument.ShellDocument.Document.FullName}\"");
                    tabItemDocument.ShellDocument.Document.Close();
                    // Удаление произойдёт через OnDocumentClosing
                }
                else if (tabItem is TabItemWindow tabItemWindow) {
                    Helpers.Diagnostic.Logger.LogDebug($"close window \"{tabItemWindow.ShellWindow.Window.Caption}\"");
                    tabItemWindow.ShellWindow.Window.Close();

                    // Удаляем вручную, так как события не будет
                    this.RemoveTabItemFromGroups(tabItemWindow);
                }
            }
        }


        private void TabList_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("TabList_SelectionChanged()");
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
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("LoadOpenDocuments()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.SortedTabItemGroups.Clear();
            foreach (EnvDTE.Document document in _dte.Documents) {
                var tabItemDocument = new TabItemDocument(document);
                this.AddTabItemToDefaultGroupIfMissing(tabItemDocument);
            }
            // TODO: add open tool windows (Git)
        }

        private void AddDocumentToPreview(TabItemDocument tabItemDocument) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("AddDocumentToPreview()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, нет ли уже этого документа в Preview
            var existing = this.PreviewDocuments.FirstOrDefault(d => string.Equals(d.FullName, tabItemDocument.ShellDocument.Document.FullName, StringComparison.OrdinalIgnoreCase));
            if (existing == null) {
                this.PreviewDocuments.Clear(); // Только один документ в режиме предварительного просмотра
                this.PreviewDocuments.Add(tabItemDocument);
            }
        }


        private void AddTabItemToDefaultGroupIfMissing(TabItemBase tabItem) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToDefaultGroupIfMissing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            string groupName;

            if (tabItem is TabItemDocument doc) {
                groupName = doc.ShellDocument.GetDocumentProjectName();
            }
            else if (tabItem is TabItemWindow) {
                groupName = "Windows";
            }
            else if (tabItem is TabItemProject proj) {
                groupName = proj.ShellProject.Project.Name;
            }
            else {
                groupName = "Other";
            }

            this.AddTabItemToGroupIfMissing(tabItem, groupName);
        }

        private void AddTabItemToGroupIfMissing(TabItemBase tabItem, string groupName) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToGroup()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItem.Caption = {tabItem.Caption}");
            Helpers.Diagnostic.Logger.LogParam($"groupName = {groupName}");

            var existingItem = this.FindTabItem(tabItem);
            if (existingItem != null) {
                return;
            }

            var group = this.SortedTabItemGroups.FirstOrDefault(g => g.GroupName == groupName);
            if (group == null) {
                group = new TabItemGroup { GroupName = groupName };
                this.SortedTabItemGroups.Add(group);
                this.SortGroups();
            }

            group.TabItems.Add(tabItem);
            this.SortDocumentsInGroups();
        }



        private void MoveDocumentFromPreviewToMainGroup(TabItemDocument tabItemDocument) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("MoveDocumentFromPreviewToMainGroup()");

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItemDocument.ShellDocument.Document.Name = {tabItemDocument.ShellDocument.Document.Name}");

            if (tabItemDocument == null) {
                return;
            }
            this.PreviewDocuments.Remove(tabItemDocument);
            this.AddTabItemToDefaultGroupIfMissing(tabItemDocument);
        }

        private void MoveDocumentToProjectGroup(TabItemDocument tabItemDocument, TabItemProject tabItemProject) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("MoveDocumentToProjectGroup()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItemDocument.FullName = {tabItemDocument.FullName}");
            Helpers.Diagnostic.Logger.LogParam($"tabItemProject.Caption = {tabItemProject.Caption}");

            this.RemoveTabItemFromGroups(tabItemDocument);
            this.AddTabItemToGroupIfMissing(tabItemDocument, tabItemProject.Caption);
        }


        private void RemoveDocumentFromPreview(TabItemDocument tabItemDocument) {
            this.PreviewDocuments.Remove(tabItemDocument);
        }

        private void RemoveTabItemFromGroups(TabItemBase tabItem) {
            foreach (var group in this.SortedTabItemGroups.ToList()) {
                if (group.TabItems.Remove(tabItem)) {
                    Helpers.Diagnostic.Logger.LogDebug($"Removed tab \"{tabItem.Caption}\" from group \"{group.GroupName}\"");

                    if (!group.TabItems.Any()) {
                        this.SortedTabItemGroups.Remove(group);
                    }

                    break; // нашли и удалили — можно выходить
                }
            }
        }


        private void ActivateTabItem(TabItemBase tabItem) {
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("ActivateTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItem.Caption = {tabItem.Caption}");

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
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("SelectTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItem.Caption = {tabItem.Caption}");

            if (_activeTabItem != null &&
                string.Equals(_activeTabItem.FullName, tabItem.FullName, StringComparison.OrdinalIgnoreCase)) {
                return;
            }

            _activeTabItem = tabItem;

            this.SelectTabItemInUI(_activeTabItem);
            this.DeselectNonActiveTabItemGroups();
        }

        private void SelectTabItemInUI(TabItemBase tabItem) {
            foreach (var group in this.SortedTabItemGroups) {
                var match = group.TabItems.FirstOrDefault(d => d.FullName == tabItem.FullName);
                if (match != null) {
                    var listBox = this.FindListBoxContainingDocument(tabItem.FullName);
                    if (listBox != null) {
                        Helpers.Diagnostic.Logger.LogDebug($"listBox.SelectedItem = \"{match.Caption}\" for \"{tabItem.FullName}\"");
                        listBox.SelectedItem = match;
                    }
                    return;
                }
            }
        }

        private void DeselectNonActiveTabItemGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var activeGroup = this.SortedTabItemGroups
                .FirstOrDefault(g => g.TabItems.Any(d => d.FullName == _activeTabItem.FullName));

            foreach (var group in this.SortedTabItemGroups) {
                if (group == activeGroup)
                    continue;

                foreach (var tabItem in group.TabItems) {
                    var listBox = this.FindListBoxContainingDocument(tabItem.FullName);
                    if (listBox != null) {
                        Helpers.Diagnostic.Logger.LogDebug($"listBox.SelectedItem = null for \"{tabItem.FullName}\"");
                        listBox.SelectedItem = null;
                    }
                }
            }
        }




        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in this.SortedTabItemGroups) {
                var docInfo = group.TabItems.FirstOrDefault(d => string.Equals(d.FullName, oldPath, StringComparison.OrdinalIgnoreCase));
                if (docInfo != null) {
                    if (newPath == null) {
                        // Обновляем только имя (в случае изменения)
                        docInfo.Caption = Path.GetFileName(oldPath);
                    }
                    else {
                        // Обновляем имя и полный путь (в случае переименования)
                        docInfo.FullName = newPath;
                        docInfo.Caption = Path.GetFileName(newPath);
                    }
                    return;
                }
            }
        }


        private void UpdateWindowTabs() {
            this.ForEachTab<TabItemWindow>(tabItemWindow => {
                ThreadHelper.ThrowIfNotOnUIThread();
                try {
                    var matchingWindow = _dte.Windows
                        .Cast<EnvDTE.Window>()
                        .FirstOrDefault(w => ShellWindow.GetWindowId(w) == tabItemWindow.WindowId);

                    if (matchingWindow != null && tabItemWindow.Caption != matchingWindow.Caption) {
                        Helpers.Diagnostic.Logger.LogDebug($"Updating TabItemWindow caption: '{tabItemWindow.Caption}' → '{matchingWindow.Caption}'");
                        tabItemWindow.Caption = matchingWindow.Caption;
                        tabItemWindow.FullName = matchingWindow.Caption;
                    }
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"Failed to update caption for TabItemWindow: {ex.Message}");
                }
            });
        }



        // Метод сортировки документов внутри групп
        private void SortDocumentsInGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.SortedTabItemGroups) {
                var sortedDocs = group.TabItems.OrderBy(d => d.Caption).ToList();
                group.TabItems.Clear();
                foreach (var doc in sortedDocs) {
                    group.TabItems.Add(doc);
                }
            }
        }

        private void SortGroups() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Сортируем группы по имени (по алфавиту)
            var sortedGroups = this.SortedTabItemGroups.OrderBy(g => g.GroupName).ToList();

            // Очищаем и добавляем отсортированные группы
            this.SortedTabItemGroups.Clear();
            foreach (var group in sortedGroups) {
                this.SortedTabItemGroups.Add(group);
            }
        }



        private TabItemDocument FindTabItem(EnvDTE.Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();
            return this.FindTabItem(new TabItemDocument(new ShellDocument(document))) as TabItemDocument;
        }

        private TabItemWindow FindTabItem(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();
            return this.FindTabItem(new TabItemWindow(window)) as TabItemWindow;
        }

        private TabItemBase FindTabItem(TabItemBase tabItem) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (tabItem is TabItemWindow tabItemWindow) {
                return this.FindTabItemBy<TabItemWindow>(w =>
                    string.Equals(w.WindowId, tabItemWindow.WindowId, StringComparison.OrdinalIgnoreCase));
            }

            return this.FindTabItemBy<TabItemBase>(t =>
                string.Equals(t.FullName, tabItem.FullName, StringComparison.OrdinalIgnoreCase));
        }

        private TabItemDocument FindTabItem(string documentFullName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            return this.FindTabItemBy<TabItemDocument>(d =>
                string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
        }

        private T FindTabItemBy<T>(Func<T, bool> predicate) where T : TabItemBase {
            ThreadHelper.ThrowIfNotOnUIThread();

            // PreviewDocuments — только для TabItemDocument
            if (typeof(T) == typeof(TabItemDocument)) {
                var preview = this.PreviewDocuments
                    .OfType<T>()
                    .FirstOrDefault(predicate);

                if (preview != null) {
                    return preview;
                }
            }

            foreach (var group in this.SortedTabItemGroups) {
                var match = group.TabItems
                    .OfType<T>()
                    .FirstOrDefault(predicate);

                if (match != null) {
                    return match;
                }
            }

            return null;
        }

        private void ForEachTab<T>(Action<T> action) where T : TabItemBase {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.SortedTabItemGroups) {
                foreach (var tabItem in group.TabItems.OfType<T>()) {
                    action(tabItem);
                }
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