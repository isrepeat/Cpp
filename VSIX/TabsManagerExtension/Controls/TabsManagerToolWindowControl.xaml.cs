using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.ComponentModel.Design;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;
using Helpers.Ex;


namespace TabsManagerExtension {
    public partial class TabsManagerToolWindowControl : UserControl, INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        // Properties:
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

        private Helpers.SortedObservableCollection<TabItemsGroup> _sortedTabItemGroups;
        public Helpers.SortedObservableCollection<TabItemsGroup> SortedTabItemGroups {
            get => _sortedTabItemGroups;
            private set {
                if (_sortedTabItemGroups != value) {
                    _sortedTabItemGroups = value;
                    OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<Helpers.IMenuItem> _contextMenuItems;
        public ObservableCollection<Helpers.IMenuItem> ContextMenuItems {
            get => _contextMenuItems;
            private set {
                if (_contextMenuItems != value) {
                    _contextMenuItems = value;
                    OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<Helpers.IMenuItem> _virtualMenuItems;
        public ObservableCollection<Helpers.IMenuItem> VirtualMenuItems {
            get => _virtualMenuItems;
            private set {
                if (_virtualMenuItems != value) {
                    _virtualMenuItems = value;
                    OnPropertyChanged();
                }
            }
        }


        // Internal:
        private EnvDTE80.DTE2 _dte;
        private EnvDTE.WindowEvents _windowEvents;
        private EnvDTE.DocumentEvents _documentEvents;
        private EnvDTE.SolutionEvents _solutionEvents;

        private DispatcherTimer _tabsManagerStateTimer;
        private FileSystemWatcher _fileWatcher;
        private VsSelectionTracker? _vsSelectionTracker;

        private Helpers.GroupsSelectionCoordinator<TabItemsGroup, TabItemBase> _tabItemsSelectionCoordinator;

        public ICommand OnTabItemPinCommand { get; }
        public ICommand OnTabItemCloseCommand { get; }
        public ICommand OnTabItemKeepOpenedCommand { get; }
        public ICommand OnTabItemContextMenuOpenCommand { get; }
        public ICommand OnTabItemContextMenuClosedCommand { get; }
        public ICommand OnTabItemVirtualMenuOpenCommand { get; }
        public ICommand OnTabItemVirtualMenuClosedCommand { get; }

        public TabsManagerToolWindowControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
            this.Unloaded += this.OnUnloaded;
            this.PreviewMouseDown += this.OnPreviewMouseDown;
            base.DataContext = this;

            this.OnTabItemPinCommand = new Helpers.RelayCommand<object>(this.OnTabItemPin);
            this.OnTabItemCloseCommand = new Helpers.RelayCommand<object>(this.OnTabItemClose);
            this.OnTabItemKeepOpenedCommand = new Helpers.RelayCommand<object>(this.OnTabItemKeepOpened);

            this.OnTabItemContextMenuOpenCommand = new Helpers.RelayCommand<object>(this.OnTabItemContextMenuOpen);
            this.OnTabItemContextMenuClosedCommand = new Helpers.RelayCommand<object>(this.OnTabItemContextMenuClosed);
            
            this.OnTabItemVirtualMenuOpenCommand = new Helpers.RelayCommand<object>(this.OnTabItemVirtualMenuOpen);
            this.OnTabItemVirtualMenuClosedCommand = new Helpers.RelayCommand<object>(this.OnTabItemVirtualMenuClosed);
        }


        //
        // ░ Self handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        //
        private void OnLoaded(object sender, RoutedEventArgs e) {
            this.InitializeDTE();
            this.InitializeFileWatcher();
            this.InitializeVsShellTrackers();
            this.InitializeTabItemsSelectionCoordinator();
            this.InitializeBackgroundRoutine();

            // Entry point:
            this.LoadOpenDocuments();
        }

        private void OnUnloaded(object sender, RoutedEventArgs e) {
            this.UninitializeTabItemsSelectionCoordinator();
            this.UninitializeVsShellTrackers();
            this.UninitializeFileWatcher();
            this.UninitializeDTE();
        }

        private void OnPreviewMouseDown(object sender, MouseButtonEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (e.OriginalSource is DependencyObject d && !this.ex_HasInteractiveElementOnPathFrom(d)) {
                // Клик по пустому пространству.

                var selectedTabItem = _tabItemsSelectionCoordinator.PrimarySelection?.Item;
                if (selectedTabItem is IActivatableTab activatableTab) {
                    activatableTab.Activate(); // Инициирует фокус редактора (или окна на его месте).
                }

                // Переводи фокус с редактора на наш контрол (чтобы редактор стал не активным, например для ввода).
                this.FocusStealer.Focus();

                // Глобально сбрасываем клавишный фокус со всего.
                //Keyboard.ClearFocus()

                Helpers.GlobalFlags.SetFlag("TextEditorFrameFocused", true);
                e.Handled = true;
            }
        }


        //
        // ░ Initialization
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        // 
        // ░ DTE
        //
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

        private void UninitializeDTE() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _solutionEvents.BeforeClosing -= OnSolutionClosing;

            _windowEvents.WindowClosing -= OnWindowClosing;
            _windowEvents.WindowActivated -= OnWindowActivated;

            _documentEvents.DocumentClosing -= OnDocumentClosing;
            _documentEvents.DocumentSaved -= OnDocumentSaved;
            _documentEvents.DocumentOpened -= OnDocumentOpened;
        }


        //
        // ░ FileWatcher 
        //
        private void InitializeFileWatcher() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var solution = _dte.Solution;
            var solutionDir = string.IsNullOrEmpty(solution.FullName) 
                ? null 
                : Path.GetDirectoryName(solution.FullName);

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

        private void UninitializeFileWatcher() {
            if (_fileWatcher != null) {
                try {
                    _fileWatcher.EnableRaisingEvents = false;

                    // Удаляем обработчики событий, чтобы отложенные события не вызывались
                    _fileWatcher.Changed -= this.OnFileChanged;
                    _fileWatcher.Renamed -= this.OnFileRenamed;
                    _fileWatcher.Deleted -= this.OnFileDeleted;

                    // Dispatcher.BeginInvoke(..., DispatcherPriority.ApplicationIdle) — ждет, пока текущий UI-цикл и все запланированные задачи завершатся.
                    // Таким образом Dispose() вызывается после завершения Run(...) внутри событий FileSystemWatcher
                    Dispatcher.BeginInvoke(new Action(() => {
                        try {
                            _fileWatcher.Dispose();
                        }
                        catch (Exception ex) {
                            Helpers.Diagnostic.Logger.LogError($"Delayed dispose of FileSystemWatcher failed: {ex}");
                        }
                        finally {
                            _fileWatcher = null;
                        }
                    }), DispatcherPriority.ApplicationIdle);
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"Error while scheduling FileSystemWatcher disposal: {ex}");
                }
            }
        }


        //
        // ░ VsShellTrackers 
        //
        private void InitializeVsShellTrackers() {
            _vsSelectionTracker = new VsSelectionTracker();
            _vsSelectionTracker.VsWindowFrameActivated += this.OnVsWindowFrameActivated;

            DocumentActivationTracker.Initialize();
            DocumentActivationTracker.OnDocumentActivated += this.OnDocumentActivatedExternally;
        }
        private void UninitializeVsShellTrackers() {
            DocumentActivationTracker.OnDocumentActivated -= this.OnDocumentActivatedExternally;
            DocumentActivationTracker.Dispose();

            _vsSelectionTracker.VsWindowFrameActivated -= this.OnVsWindowFrameActivated;
        }


        //
        // ░ TabItemsSelectionCoordinator 
        //
        private void InitializeTabItemsSelectionCoordinator() {
            var defaultTabItemGroupComparer = Comparer<TabItemsGroup>.Create((a, b) => string.Compare(a.GroupName, b.GroupName, StringComparison.OrdinalIgnoreCase));
            var priorityGroups = new List<Helpers.PriorityGroup<TabItemsGroup>> {
                new Helpers.PriorityGroup<TabItemsGroup> {
                    Position = Helpers.ItemPosition.Top,
                    Predicate = tabItemGroup => tabItemGroup.GroupName.StartsWith("__Preview"),
                    Comparer = defaultTabItemGroupComparer
                }
            };
            this.SortedTabItemGroups = new Helpers.SortedObservableCollection<TabItemsGroup>(
                defaultTabItemGroupComparer,
                priorityGroups
                );

            _tabItemsSelectionCoordinator = new Helpers.GroupsSelectionCoordinator<TabItemsGroup, TabItemBase>(this.SortedTabItemGroups);
            _tabItemsSelectionCoordinator.OnItemSelectionChanged = this.OnTabItemSelectionChanged;
            _tabItemsSelectionCoordinator.OnSelectionStateChanged = this.OnSelectionStateChanged;
        }

        private void UninitializeTabItemsSelectionCoordinator() {
        }


        // 
        // ░ BackgroundRoutine
        //
        private void InitializeBackgroundRoutine() {
            _tabsManagerStateTimer = new DispatcherTimer();
            _tabsManagerStateTimer.Interval = TimeSpan.FromMilliseconds(200);
            _tabsManagerStateTimer.Tick += this.TabsManagerStateTimerHandler;
            _tabsManagerStateTimer.Start();
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        // 
        // ░ DTE
        //
        private void OnDocumentOpened(EnvDTE.Document document) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentOpened()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document?.Name}");

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
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentSaved()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document?.Name}");

            this.TabsManagerStateTimerHandler(null, null);
        }


        private void OnDocumentClosing(EnvDTE.Document document) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"document.Name = {document?.Name}");

            var tabItemDocument = this.FindTabItem(document);
            if (tabItemDocument != null) {
                this.RemoveTabItemFromGroups(tabItemDocument);
            }
            else {
                Helpers.Diagnostic.Logger.LogWarning($"\"{document?.Name}\" not found in collections");
            }
        }


        private void OnWindowActivated(EnvDTE.Window gotFocus, EnvDTE.Window lostFocus) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowActivated()");
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

            var addedOrExistTabItem = this.AddTabItemToDefaultGroupIfMissing(tabItem);
            addedOrExistTabItem.IsSelected = true; // Select after tabItem exist added to group.

            this.UpdateWindowTabsInfo();
        }


        private void OnWindowClosing(EnvDTE.Window closingWindow) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnWindowClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"closingWindow.Caption = {closingWindow?.Caption}");
        }


        private void OnSolutionClosing() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionClosing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // TODO: try replace with this.Unload()
            this.SortedTabItemGroups.Clear();
            this.UninitializeFileWatcher();
        }
        

        // 
        // ░ FileWatcher
        //
        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileChanged()");

            if (this.IsTemporaryFile(e.FullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.UpdateDocumentUI(e.FullPath);
            });
        }

        private void OnFileRenamed(object sender, RenamedEventArgs e) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileRenamed()");

            if (this.IsTemporaryFile(e.FullPath) || this.IsTemporaryFile(e.OldFullPath)) {
                return;
            }

            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                this.UpdateDocumentUI(e.OldFullPath, e.FullPath);
            });
        }

        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnFileDeleted()");

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


        //
        // ░ VsShellTrackers 
        //
        private void OnVsWindowFrameActivated(IVsWindowFrame vsWindowFrame) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnVsWindowFrameActivated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // NOTE: При запуске VS всегда активен последний документ сессии и
            //       OnVsWindowFrameActivated не сработает при переактивации этого же документа в ручную,
            //       поэтому явно ставим TextEditorFrameFocused = true в OnPreviewMouseDown.

            //VSFM_Dock = 0,     // ToolWindow: фиксировано сбоку, снизу и т.п. (Solution Explorer, Output и т.п.)
            //VSFM_MDIChild = 1, // Документное окно: занимает пространство текстового редактора, находится во вкладках
            //VSFM_Float = 2     // Плавающее окно: вытащено за пределы главного окна
            vsWindowFrame.GetProperty((int)__VSFPROPID.VSFPROPID_FrameMode, out var mode);
            Helpers.Diagnostic.Logger.LogDebug($"vsWindowFrame.FrameMode = {(VSFRAMEMODE)(int)mode}");

            bool isMdiChild = mode != null && (VSFRAMEMODE)(int)mode == VSFRAMEMODE.VSFM_MdiChild;
            Helpers.GlobalFlags.SetFlag("TextEditorFrameFocused", isMdiChild);

            //// Получаем содержимое активного окна (View)
            //vsWindowFrame.GetProperty((int)__VSFPROPID.VSFPROPID_DocView, out var docView);

            //// Если это кодовое окно, запрашиваем его "основной" текстовый редактор
            //if (docView is IVsCodeWindow codeWindow) {
            //    if (codeWindow.GetPrimaryView(out var textView) == VSConstants.S_OK && textView != null) {
            //        // Это полноценный редактор
            //    }
            //}
        }

        private void OnDocumentActivatedExternally(string documentFullName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentActivatedExternally()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"documentFullName = {documentFullName}");

            var tabItem = this.FindTabItem(documentFullName);
            if (tabItem != null) {
                tabItem.IsSelected = true;
            }
        }


        // 
        // ░ TabItemsSelectionCoordinator
        //
        private void OnTabItemSelectionChanged(TabItemsGroup group, TabItemBase tabItem, bool isSelected) {
            Helpers.Diagnostic.Logger.LogDebug($"[{(isSelected ? "Selected" : "Unselected")}] {tabItem.Caption} in group {group.GroupName}");

            if (isSelected) {
                this.ActivatePrimaryTabItem();
            }
        }

        private void OnSelectionStateChanged(Helpers.Enums.SelectionState selectionState) {
            switch (selectionState) {
                case Helpers.Enums.SelectionState.Single:
                    this.ActivatePrimaryTabItem();
                    break;

                case Helpers.Enums.SelectionState.Multiple:
                    break;
            }
        }


        // 
        // ░ BackgroundRoutine
        //
        private void TabsManagerStateTimerHandler(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            // NOTE: Нужно использовать копии коллекций для безопасного перебора.
            // Поэтому в foreach вызывай .ToList() у коллекций.

            // === [A] Обновление статуса сохранения документов ===
            foreach (var tabItemsGroup in this.SortedTabItemGroups.ToList()) {
                foreach (var tabItem in tabItemsGroup.Items.ToList()) {
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
            var previewGroup = this.SortedTabItemGroups.FirstOrDefault(g => g.IsPreviewGroup);
            if (previewGroup != null) {
                foreach (var tabItemDocument in previewGroup.Items.OfType<TabItemDocument>().ToList()) {
                    if (!tabItemDocument.ShellDocument.IsDocumentInPreviewTab()) {
                        this.MoveDocumentFromPreviewToMainGroup(tabItemDocument);
                    }
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
                var toRemove = group.Items
                    .OfType<TabItemWindow>()
                    .Where(w => !openWindowIds.Contains(w.WindowId))
                    .ToList();

                foreach (var tab in toRemove) {
                    this.RemoveTabItemFromGroups(tab);
                }
            }

            // === [D] Обновление окон типа TabItemWindow ===
            this.UpdateWindowTabsInfo();
        }


        //
        // ░ UI click handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        //
        // ░ Commands
        //
        private void AAA(object parameter) {
        }
        private void OnTabItemPin(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnTabItemPin()");
            ThreadHelper.ThrowIfNotOnUIThread();
        }

        private void OnTabItemClose(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnTabItemClose()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is TabItemBase tabItem) {
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

                this.VirtualMenuControl.HideImmediately();
            }
        }

        private void OnTabItemKeepOpened(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnTabItemKeepOpened()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is TabItemBase tabItem) {
                if (tabItem is TabItemDocument tabItemDocument) {
                    this.MoveDocumentFromPreviewToMainGroup(tabItemDocument);
                    tabItemDocument.ShellDocument.OpenDocumentAsPinned();
                }
            }
        }


        // 
        // ░ ContextMenu
        //
        private void OnTabItemContextMenuOpen(object parameter) {
            if (parameter is Controls.MenuControl.ContextMenuOpenRequest contextMenuOpenRequest) {
                if (contextMenuOpenRequest.DataContext is TabItemBase tabItem) {
                    switch (_tabItemsSelectionCoordinator.SelectionState) {
                        case Helpers.Enums.SelectionState.Single:
                            if (tabItem is TabItemDocument tabItemDocument) {
                                tabItemDocument.Metadata.SetFlag("IsCtxMenuOpenned", true);

                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.OpenTabLocation,
                                        Command = new Helpers.RelayCommand<object>(this.AAA)
                                    },
                                    new Helpers.MenuItemSeparator(),
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.CloseTab,
                                        Command = new Helpers.RelayCommand<object>(this.OnTabItemClose)
                                    },
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.PinTab,
                                        Command = new Helpers.RelayCommand<object>(this.AAA)
                                    }
                                };
                            }
                            else if (tabItem is TabItemWindow tabItemWindow) {
                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.CloseTab,
                                        Command = new Helpers.RelayCommand<object>(this.AAA)
                                    },
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.PinTab,
                                        Command = new Helpers.RelayCommand<object>(this.AAA)
                                    }
                                };
                            }
                            break;

                        case Helpers.Enums.SelectionState.Multiple:
                            bool isTabItemAmongSelectedItems = _tabItemsSelectionCoordinator.SelectedItems
                                .Any(entry => ReferenceEquals(entry.Item, tabItem));

                            if (isTabItemAmongSelectedItems) {
                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemDefault {
                                        Header = Constants.UI.CloseSelectedTabs,
                                        Command = new Helpers.RelayCommand<object>(this.AAA)
                                    }
                                };
                            }
                            else {
                                contextMenuOpenRequest.ShouldOpen = false;
                                tabItem.IsSelected = true;
                            }
                            break;
                    }
                }
            }
        }

        private void OnTabItemContextMenuClosed(object parameter) {
            if (parameter is TabItemBase tabItem) {
                tabItem.Metadata.SetFlag("IsCtxMenuOpenned", false);
            }
        }


        // 
        // ░ VirtualMenu
        //
        private void InteractiveArea_MouseEnter(object sender, MouseEventArgs e) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseEnter()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (sender is FrameworkElement interactiveArea) {
                // Находим родительский ListViewItem (где привязаны данные)
                var listViewItem = Helpers.VisualTree.FindParentByType<ListViewItem>(interactiveArea);
                if (listViewItem == null) {
                    return;
                }

                // Получаем привязанный объект (TabItemDocument)
                if (listViewItem.DataContext is TabItemDocument tabItemDocument) {
                    if (tabItemDocument.ShellDocument != null) {
                        tabItemDocument.UpdateProjectReferenceList();
                    }
                    var screenPoint = interactiveArea.ex_ToDpiAwareScreen(new Point(interactiveArea.ActualWidth + 20, -60));
                    this.VirtualMenuControl.Show(screenPoint, tabItemDocument);
                }
            }
        }

        private void InteractiveArea_MouseLeave(object sender, MouseEventArgs e) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"InteractiveArea_MouseLeave()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.VirtualMenuControl.Hide();
        }

        private void OnTabItemVirtualMenuOpen(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnTabItemVirtualMenuOpen()");

            if (parameter is Controls.MenuControl.ContextMenuOpenRequest contextMenuOpenRequest) {
                if (contextMenuOpenRequest.DataContext is TabItemBase tabItem) {
                    if (tabItem is TabItemDocument tabItemDocument) {
                        this.VirtualMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                            new Helpers.MenuItemDefault {
                                Header = tabItem.Caption,
                                Command = new Helpers.RelayCommand<object>(this.AAA)
                            },
                            new Helpers.MenuItemSeparator(),
                            new Helpers.MenuItemDefault {
                                Header = Constants.UI.OpenTabLocation,
                                Command = new Helpers.RelayCommand<object>(this.AAA)
                            },
                            new Helpers.MenuItemDefault {
                                Header = Constants.UI.CloseTab,
                                Command = new Helpers.RelayCommand<object>(this.OnTabItemClose)
                            },
                            new Helpers.MenuItemDefault {
                                Header = Constants.UI.PinTab,
                                Command = new Helpers.RelayCommand<object>(this.AAA)
                            }
                        };
                    }
                    else if (tabItem is TabItemWindow tabItemWindow) {
                        this.VirtualMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                            new Helpers.MenuItemDefault {
                                Header = Constants.UI.CloseTab,
                                Command = new Helpers.RelayCommand<object>(this.AAA)
                            },
                            new Helpers.MenuItemDefault {
                                Header = Constants.UI.PinTab,
                                Command = new Helpers.RelayCommand<object>(this.AAA)
                            }
                        };
                    }
                }
            }
        }
        private void OnTabItemVirtualMenuClosed(object parameter) {
        }


        private void ProjectMenuItem_Click(object sender, RoutedEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("ProjectMenuItem_Click()");

            if (sender is Button button && button.CommandParameter is DocumentProjectReferenceInfo documentProjectReferenceInfo) {
                this.MoveDocumentToProjectGroup(
                    documentProjectReferenceInfo.TabItemDocument,
                    documentProjectReferenceInfo.TabItemProject
                    );
            }
        }


        // 
        // ░ ScaleSelectorControl
        //
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
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        //
        // ░ Adding tabs
        //
        private void LoadOpenDocuments() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("LoadOpenDocuments()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.SortedTabItemGroups.Clear();
            foreach (EnvDTE.Document document in _dte.Documents) {
                var tabItemDocument = new TabItemDocument(document);
                this.AddTabItemToDefaultGroupIfMissing(tabItemDocument);
            }

            // NOTE: В стандартном TabsManager открытыие окна [ToolWindows] сохраняются с предыдущей сессии
            // видимо в конфиг файле, т.к. среди _dte.Windows их нет.
            //
            // TODO: Добавляй ToolWindows не из открытых окон, а из конфиг файла хранящего предыдущую сессию.
            foreach (EnvDTE.Window window in _dte.Windows) {
                if (window.Document != null) {
                    continue; // skip documents
                }

                var shellWindow = new ShellWindow(window);
                if (!shellWindow.IsTabWindow()) {
                    continue; // skip non tab windows
                }

                var tabItemWindow = new TabItemWindow(shellWindow);
                this.AddTabItemToDefaultGroupIfMissing(tabItemWindow);
            }

            this.SyncActiveDocumentWithPrimaryTabItem();
        }

        private void AddDocumentToPreview(TabItemDocument tabItemDocument) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddDocumentToPreview()");
            ThreadHelper.ThrowIfNotOnUIThread();

            var previewGroup = this.SortedTabItemGroups.FirstOrDefault(g => g.IsPreviewGroup);
            if (previewGroup != null) {
                this.RemoveTabItemsGroup(previewGroup);
            }
            this.AddTabItemToGroupIfMissing(tabItemDocument, "__Preview__");
            tabItemDocument.IsPreviewTab = true;
            tabItemDocument.IsSelected = true; // Select after tabItem exist added to group.
        }


        private TabItemBase AddTabItemToDefaultGroupIfMissing(TabItemBase tabItem) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToDefaultGroupIfMissing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            string groupName;

            if (tabItem is TabItemDocument doc) {
                groupName = doc.ShellDocument.GetDocumentProjectName();
            }
            else if (tabItem is TabItemWindow) {
                groupName = "[Tool Windows]";
            }
            else if (tabItem is TabItemProject proj) {
                groupName = proj.ShellProject.Project.Name;
            }
            else {
                groupName = "Other";
            }

            return this.AddTabItemToGroupIfMissing(tabItem, groupName);
        }

        private TabItemBase AddTabItemToGroupIfMissing(TabItemBase tabItem, string groupName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToGroup()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItem.Caption = {tabItem?.Caption}");
            Helpers.Diagnostic.Logger.LogParam($"groupName = {groupName}");

            var existingItem = this.FindTabItem(tabItem);
            if (existingItem != null) {
                return existingItem;
            }

            var group = this.SortedTabItemGroups.FirstOrDefault(g => g.GroupName == groupName);
            if (group == null) {
                group = new TabItemsGroup { GroupName = groupName };
                this.SortedTabItemGroups.Add(group);
            }

            group.Items.Add(tabItem);
            return tabItem;
        }


        // 
        // ░ Removing tabs
        //
        private void MoveDocumentFromPreviewToMainGroup(TabItemDocument tabItemDocument) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("MoveDocumentFromPreviewToMainGroup()");

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItemDocument.ShellDocument.Document.Name = {tabItemDocument?.ShellDocument.Document.Name}");

            if (tabItemDocument == null) {
                return;
            }
            if (!tabItemDocument.IsPreviewTab) {
                return;
            }

            var previewGroup = this.SortedTabItemGroups.FirstOrDefault(g => g.IsPreviewGroup);
            if (previewGroup != null) {
                this.RemoveTabItemsGroup(previewGroup);
            }
            this.AddTabItemToDefaultGroupIfMissing(tabItemDocument);
            tabItemDocument.IsPreviewTab = false;
        }

        private void MoveDocumentToProjectGroup(TabItemDocument tabItemDocument, TabItemProject tabItemProject) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("MoveDocumentToProjectGroup()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItemDocument.FullName = {tabItemDocument?.FullName}");
            Helpers.Diagnostic.Logger.LogParam($"tabItemProject.Caption = {tabItemProject?.Caption}");

            this.RemoveTabItemFromGroups(tabItemDocument);
            this.AddTabItemToGroupIfMissing(tabItemDocument, tabItemProject.Caption);
        }


        private void RemoveTabItemsGroup(TabItemsGroup tabItemsGroup) {
            if (this.SortedTabItemGroups.Remove(tabItemsGroup)) {
                Helpers.Diagnostic.Logger.LogDebug($"Removed group \"{tabItemsGroup.GroupName}\"");
            }
        }

        private void RemoveTabItemFromGroups(TabItemBase tabItem) {
            foreach (var group in this.SortedTabItemGroups.ToList()) {
                // Удаляем связанный TabItems из выбранных (если используется SelectionCoordinator и UI)
                if (group.Items.Remove(tabItem)) {
                    Helpers.Diagnostic.Logger.LogDebug($"Removed tab \"{tabItem.Caption}\" from group \"{group.GroupName}\"");

                    if (!group.Items.Any()) {
                        if (this.SortedTabItemGroups.Remove(group)) {
                            Helpers.Diagnostic.Logger.LogDebug($"Removed group \"{group.GroupName}\"");
                        }
                    }
                    break;
                }
            }
        }


        // 
        // ░ Updating tabs
        //
        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in this.SortedTabItemGroups) {
                var docInfo = group.Items.FirstOrDefault(d => string.Equals(d.FullName, oldPath, StringComparison.OrdinalIgnoreCase));
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

        private void UpdateWindowTabsInfo() {
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


        // 
        // ░ Activating tabs
        //
        private void ActivatePrimaryTabItem() {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("ActivatePrimaryTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            var primaryTabItem = _tabItemsSelectionCoordinator.PrimarySelection?.Item;
            if (primaryTabItem is IActivatableTab activatableTab) {
                if (_dte.ActiveDocument != null && string.Equals(_dte.ActiveDocument.FullName, primaryTabItem.FullName, StringComparison.OrdinalIgnoreCase)) {
                    Helpers.Diagnostic.Logger.LogDebug($"Skip Activate, alredy active - \"{primaryTabItem.Caption}\"");
                    return;
                }

                Helpers.Diagnostic.Logger.LogDebug($"Activate - \"{primaryTabItem.Caption}\"");
                activatableTab.Activate();
            }
        }

        private void SyncActiveDocumentWithPrimaryTabItem() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var activeWindow = _dte.ActiveWindow;
            if (activeWindow == null) {
                return;
            }

            var selectedTabItem = _tabItemsSelectionCoordinator.PrimarySelection?.Item;
            TabItemBase targetTabItem = null;

            if (ShellWindow.IsTabWindow(activeWindow)) {
                // Document or Tool Window can be activated.
                if (activeWindow.Document == null) {
                    if (string.Equals(activeWindow.Caption, selectedTabItem?.Caption, StringComparison.OrdinalIgnoreCase)) {
                        return;
                    }
                    Helpers.Diagnostic.Logger.LogDebug($"Sync tabs with activeWindow.Caption = {activeWindow.Caption}");
                    targetTabItem = this.FindTabItem(activeWindow);
                }
                else {
                    if (string.Equals(activeWindow.Document.FullName, selectedTabItem?.FullName, StringComparison.OrdinalIgnoreCase)) {
                        return;
                    }
                    Helpers.Diagnostic.Logger.LogDebug($"Sync tabs with activeWindow.Document.Name = {activeWindow.Document.Name}");
                    targetTabItem = this.FindTabItem(activeWindow.Document);
                }
            }
            else {
                // Only Document can be activated (for example when choose document from SolutionExplorer)
                var activeDocument = _dte.ActiveDocument;
                if (activeDocument == null) {
                    return;
                }
                if (string.Equals(activeDocument.FullName, selectedTabItem?.FullName, StringComparison.OrdinalIgnoreCase)) {
                    return;
                }
                Helpers.Diagnostic.Logger.LogDebug($"Sync tabs with activeDocument.Name = {activeDocument.Name}");
                targetTabItem = this.FindTabItem(activeDocument);
            }

            if (targetTabItem != null) {
                targetTabItem.IsSelected = true;
            }
        }


        //
        // ░ Helpers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
        //
        private TabItemDocument FindTabItem(EnvDTE.Document document) {
            return this.FindTabItemWithGroup(document)?.Item;
        }

        private TabItemWindow FindTabItem(EnvDTE.Window window) {
            return this.FindTabItemWithGroup(window)?.Item;
        }

        private TabItemBase FindTabItem(TabItemBase tabItem) {
            return this.FindTabItemWithGroup(tabItem)?.Item;
        }

        private TabItemDocument FindTabItem(string documentFullName) {
            return this.FindTabItemWithGroup(documentFullName)?.Item;
        }


        private (TabItemDocument Item, TabItemsGroup Group)? FindTabItemWithGroup(EnvDTE.Document document) {
            var result = this.FindTabItemWithGroup(new TabItemDocument(document));
            if (result is { Item: TabItemDocument doc, Group: var group }) {
                return (doc, group);
            }
            return null;
        }

        private (TabItemWindow Item, TabItemsGroup Group)? FindTabItemWithGroup(EnvDTE.Window window) {
            var result = this.FindTabItemWithGroup(new TabItemWindow(window));
            if (result is { Item: TabItemWindow win, Group: var group }) {
                return (win, group);
            }
            return null;
        }

        private (TabItemBase Item, TabItemsGroup Group)? FindTabItemWithGroup(TabItemBase tabItem) {
            if (tabItem is TabItemWindow tabItemWindow) {
                return this.FindTabItemWithGroupBy<TabItemWindow>(
                    w => string.Equals(w.WindowId, tabItemWindow.WindowId, StringComparison.OrdinalIgnoreCase));
            }
            return this.FindTabItemWithGroupBy<TabItemBase>(
                t => string.Equals(t.FullName, tabItem.FullName, StringComparison.OrdinalIgnoreCase));
        }

        private (TabItemDocument Item, TabItemsGroup Group)? FindTabItemWithGroup(string documentFullName) {
            return this.FindTabItemWithGroupBy<TabItemDocument>(
                    d => string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
        }


        private (T Item, TabItemsGroup Group)? FindTabItemWithGroupBy<T>(Func<T, bool> predicate) where T : TabItemBase {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.SortedTabItemGroups) {
                var match = group.Items
                    .OfType<T>()
                    .FirstOrDefault(predicate);

                if (match != null) {
                    return (match, group);
                }
            }

            return null;
        }


        private void ForEachTab<T>(Action<T> action) where T : TabItemBase {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.SortedTabItemGroups) {
                foreach (var tabItem in group.Items.OfType<T>()) {
                    action(tabItem);
                }
            }
        }

        private bool IsTemporaryFile(string fullPath) {
            string extension = Path.GetExtension(fullPath);
            return extension.Equals(".TMP", StringComparison.OrdinalIgnoreCase) ||
                   fullPath.Contains("~") && fullPath.Contains(".TMP");
        }


        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}