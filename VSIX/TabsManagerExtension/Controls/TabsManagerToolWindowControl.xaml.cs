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
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;
using Task = System.Threading.Tasks.Task;
using Helpers.Ex;
using TabsManagerExtension.State.Document;


namespace TabsManagerExtension.Controls {
    public partial class TabsManagerToolWindowControl : Helpers.BaseUserControl {

        // Properties:
        private Helpers.SortedObservableCollection<TabItemsGroupBase> _sortedTabItemsGroups;
        public Helpers.SortedObservableCollection<TabItemsGroupBase> SortedTabItemsGroups {
            get => _sortedTabItemsGroups;
            private set {
                if (_sortedTabItemsGroups != value) {
                    _sortedTabItemsGroups = value;
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

        private double _scaleFactorUI = 1.0;
        public double ScaleFactorUI {
            get => _scaleFactorUI;
            set {
                if (_scaleFactorUI != value) {
                    _scaleFactorUI = value;
                    OnPropertyChanged();
                    this.ApplyScaleUI();
                }
            }
        }

        private double _scaleFactorTabsCompactness = 1.0;
        public double ScaleFactorTabsCompactness {
            get => _scaleFactorTabsCompactness;
            set {
                if (_scaleFactorTabsCompactness != value) {
                    _scaleFactorTabsCompactness = value;
                    OnPropertyChanged();
                    this.ApplyScaleTabsCompactness();
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

        private Helpers.GroupsSelectionCoordinator<TabItemsGroupBase, TabItemBase> _tabItemsSelectionCoordinator;
        private VsShell.TextEditor.Overlay.TextEditorOverlayController _textEditorOverlayController;


        public ICommand OnPinTabItemCommand { get; }
        public ICommand OnUnpinTabItemCommand { get; }
        public ICommand OnCloseTabItemCommand { get; }
        public ICommand OnKeepOpenedTabItemCommand { get; }
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

            this.OnPinTabItemCommand = new Helpers.RelayCommand<object>(this.OnPinTabItem);
            this.OnUnpinTabItemCommand = new Helpers.RelayCommand<object>(this.OnUnpinTabItem);
            this.OnCloseTabItemCommand = new Helpers.RelayCommand<object>(this.OnCloseTabItem);
            this.OnKeepOpenedTabItemCommand = new Helpers.RelayCommand<object>(this.OnKeepOpenedTabItem);

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
            Services.ExtensionServices.BeginUsage();

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

            Services.ExtensionServices.EndUsage();
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
                //Keyboard.ClearFocus();

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

                    var watcherToDispose = _fileWatcher;
                    _fileWatcher = null;

                    // Dispatcher.BeginInvoke(..., DispatcherPriority.ApplicationIdle) — ждет, пока текущий UI-цикл и все запланированные задачи завершатся.
                    // Таким образом Dispose() вызывается после завершения Run(...) внутри событий FileSystemWatcher
                    Dispatcher.BeginInvoke(new Action(() => {
                        try {
                            // Копируем ссылку на _fileWatcher чтобы продлить жизнь, т.к. _fileWatcher уже может быть удален
                            // во время исполнения лямбды, но его ресурсы так и не будут освобождены.
                            watcherToDispose.Dispose();
                        }
                        catch (Exception ex) {
                            Helpers.Diagnostic.Logger.LogError($"Delayed dispose of FileSystemWatcher failed: {ex}");
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
            VsShell.Services.VsSelectionTrackerService.Instance.VsWindowFrameActivated += this.OnVsWindowFrameActivated;
            VsShell.TextEditor.Services.DocumentActivationTrackerService.Instance.OnDocumentActivated += this.OnDocumentActivatedExternally;
        }
        private void UninitializeVsShellTrackers() {
            VsShell.TextEditor.Services.DocumentActivationTrackerService.Instance.OnDocumentActivated -= this.OnDocumentActivatedExternally;
            VsShell.Services.VsSelectionTrackerService.Instance.VsWindowFrameActivated -= this.OnVsWindowFrameActivated;
        }


        //
        // ░ TabItemsSelectionCoordinator 
        //
        private void InitializeTabItemsSelectionCoordinator() {
            var defaultTabItemsGroupComparer = Comparer<TabItemsGroupBase>.Create((a, b) => string.Compare(a.GroupName, b.GroupName, StringComparison.OrdinalIgnoreCase));
            var priorityGroups = new List<Helpers.PriorityGroup<TabItemsGroupBase>> {
                new Helpers.PriorityGroup<TabItemsGroupBase> {
                    Position = Helpers.ItemPosition.Top,
                    InsertMode = Helpers.ItemInsertMode.SingleWithReplaceExisting,
                    Predicate = g => g is TabItemsPreviewGroup,
                    Comparer = defaultTabItemsGroupComparer
                },
                new Helpers.PriorityGroup<TabItemsGroupBase> {
                    Position = Helpers.ItemPosition.Top + 1,
                    InsertMode = Helpers.ItemInsertMode.Single,
                    Predicate = g => g is SeparatorTabItemsGroup separator && separator.Key == "Preview-Pinned",
                    Comparer = defaultTabItemsGroupComparer
                },
                new Helpers.PriorityGroup<TabItemsGroupBase> {
                    Position = Helpers.ItemPosition.Top + 2,
                    Predicate = g => g is TabItemsPinnedGroup,
                    Comparer = defaultTabItemsGroupComparer
                },
                new Helpers.PriorityGroup<TabItemsGroupBase> {
                    Position = Helpers.ItemPosition.Top + 3,
                    InsertMode = Helpers.ItemInsertMode.Single,
                    Predicate = g => g is SeparatorTabItemsGroup separator && separator.Key == "Pinned-Default",
                    Comparer = defaultTabItemsGroupComparer
                },
                new Helpers.PriorityGroup<TabItemsGroupBase> {
                    Position = Helpers.ItemPosition.Middle,
                    Predicate = g => g is TabItemsDefaultGroup,
                    Comparer = defaultTabItemsGroupComparer
                }
            };
            this.SortedTabItemsGroups = new Helpers.SortedObservableCollection<TabItemsGroupBase>(
                defaultTabItemsGroupComparer,
                priorityGroups
                );

            _tabItemsSelectionCoordinator = new Helpers.GroupsSelectionCoordinator<TabItemsGroupBase, TabItemBase>(this.SortedTabItemsGroups);
            _tabItemsSelectionCoordinator.OnItemSelectionChanged = this.OnTabItemSelectionChanged;
            _tabItemsSelectionCoordinator.OnSelectionStateChanged = this.OnSelectionStateChanged;


            _textEditorOverlayController = new VsShell.TextEditor.Overlay.TextEditorOverlayController(_dte);
        }

        private void UninitializeTabItemsSelectionCoordinator() {
            _textEditorOverlayController.Hide();
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
                this.AddTabItemToAutoDeterminedGroupIfMissing(tabItemDocument);
            }

            _textEditorOverlayController.Update();
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

            var addedOrExistTabItem = this.AddTabItemToAutoDeterminedGroupIfMissing(tabItem);
            addedOrExistTabItem.IsSelected = true;

            this.UpdateWindowTabsInfo();

            _textEditorOverlayController.Update();
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
            this.SortedTabItemsGroups.Clear();
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


            if (isMdiChild) {
                // Получаем содержимое активного окна (View)
                vsWindowFrame.GetProperty((int)__VSFPROPID.VSFPROPID_DocView, out var docView);

                // Если это кодовое окно, запрашиваем его "основной" текстовый редактор
                if (docView is IVsCodeWindow codeWindow) {
                    if (codeWindow.GetPrimaryView(out var textView) == VSConstants.S_OK && textView != null) {
                        // Это полноценный редактор
                        _textEditorOverlayController.Show();
                        return;
                    }
                }
            }
            _textEditorOverlayController.Hide();
        }

        private void OnDocumentActivatedExternally(string documentFullName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnDocumentActivatedExternally()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"documentFullName = {documentFullName}");

            var tabItem = this.FindTabItem(documentFullName);
            if (tabItem != null) {
                tabItem.Metadata.SetFlag("IsActivatedExternally", true);
                tabItem.IsSelected = true;
            }
        }
        

        // 
        // ░ TabItemsSelectionCoordinator
        //
        private void OnTabItemSelectionChanged(TabItemsGroupBase group, TabItemBase tabItem, bool isSelected) {
            Helpers.Diagnostic.Logger.LogDebug($"[{(isSelected ? "Selected" : "Unselected")}] {tabItem.Caption} in group {group.GroupName}");

            var isActivatedExtarnally = tabItem.Metadata.GetFlag("IsActivatedExternally");
            if (isActivatedExtarnally) {
                tabItem.Metadata.SetFlag("IsActivatedExternally", false);
            }

            if (isSelected) {
                // При внешней активации (например, из Solution Explorer) фокус остаётся вне редактора —
                // в этом случае не трогаем его вручную, чтобы не сбивать пользовательский фокус.
                if (!isActivatedExtarnally) {
                    this.ActivatePrimaryTabItem();
                }
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
            foreach (var tabItemsGroup in this.SortedTabItemsGroups.ToList()) {
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
            var previewGroup = this.SortedTabItemsGroups.FirstOrDefault(g => g is TabItemsPreviewGroup);
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

            foreach (var group in this.SortedTabItemsGroups.ToList()) {
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
        private void OnPinTabItem(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnPinTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is not TabItemBase tabItem) {
                return;
            }

            // Если вкладка уже закреплена — ничего не делаем
            if (tabItem.IsPinnedTab) {
                return;
            }

            // Ищем вкладку и группу, в которой она сейчас находится
            var current = this.FindTabItemWithGroup(tabItem);
            if (current == null) {
                return; // Не найдена — нечего обрабатывать
            }

            var item = current.Value.Item;
            var oldGroup = current.Value.Group;

            this.RemoveTabItemFromGroup(item, oldGroup);
            this.AddTabItemToGroupIfMissing(tabItem, new TabItemsPinnedGroup(oldGroup.GroupName));
        }


        private void OnUnpinTabItem(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnUnpinTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is not TabItemBase tabItem) {
                return;
            }

            if (!tabItem.IsPinnedTab) {
                return;
            }

            var current = this.FindTabItemWithGroup(tabItem);
            if (current == null) {
                return;
            }

            var item = current.Value.Item;
            var oldGroup = current.Value.Group;

            this.RemoveTabItemFromGroup(item, oldGroup);
            this.AddTabItemToGroupIfMissing(tabItem, new TabItemsDefaultGroup(oldGroup.GroupName));
        }


        private void OnCloseTabItem(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnCloseTabItem()");
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

        private void OnCloseSelectedTabItems(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnCloseSelectedTabItems()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is TabItemBase tabItem) {
                this.VirtualMenuControl.HideImmediately();
            }
        }

        private void OnKeepOpenedTabItem(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnKeepOpenedTabItem()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is TabItemBase tabItem) {
                if (tabItem is TabItemDocument tabItemDocument) {
                    this.MoveDocumentFromPreviewToMainGroup(tabItemDocument);
                    tabItemDocument.ShellDocument.OpenDocumentAsPinned();
                }
            }
        }

        private void OnOpenLocationTabItem(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnOpenTabLocation()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is TabItemBase tabItem) {
                if (tabItem is TabItemDocument tabItemDocument) {
                    try {
                        string filePath = tabItemDocument.FullName;

                        if (System.IO.File.Exists(filePath)) {
                            string args = $"/select,\"{filePath}\"";
                            System.Diagnostics.Process.Start("explorer.exe", args);
                        }
                        else {
                            Helpers.Diagnostic.Logger.LogWarning($"File not found: {filePath}");
                        }
                    }
                    catch (Exception ex) {
                        Helpers.Diagnostic.Logger.LogError($"Failed to open tab location: {ex.Message}");
                    }
                }

                this.VirtualMenuControl.HideImmediately();
            }
        }

        private void OnMoveTabItemToRelatedProject(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnMoveTabItemToRelatedProject()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (parameter is DocumentProjectReferenceInfo documentProjectReferenceInfo) {
                this.MoveDocumentToProjectGroup(
                    documentProjectReferenceInfo.TabItemDocument,
                    documentProjectReferenceInfo.TabItemProject
                    );
            }
        }


        // 
        // ░ ContextMenu
        //
        private void OnTabItemContextMenuOpen(object parameter) {
            if (parameter is Controls.MenuControl.MenuOpeningArgs contextMenuOpeningArgs) {
                if (contextMenuOpeningArgs.DataContext is TabItemBase tabItem) {

                    switch (_tabItemsSelectionCoordinator.SelectionState) {
                        case Helpers.Enums.SelectionState.Single:
                            if (tabItem is TabItemDocument tabItemDocument) {
                                tabItemDocument.Metadata.SetFlag("IsCtxMenuOpenned", true);

                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemCommand {
                                        Header = State.Constants.UI.OpenTabLocation,
                                        Command = new Helpers.RelayCommand<object>(this.OnOpenLocationTabItem),
                                        CommandParameterContext = contextMenuOpeningArgs.DataContext,
                                    },
                                    new Helpers.MenuItemSeparator(),
                                    new Helpers.MenuItemCommand {
                                        Header = State.Constants.UI.CloseTab,
                                        Command = new Helpers.RelayCommand<object>(this.OnCloseTabItem),
                                        CommandParameterContext = contextMenuOpeningArgs.DataContext,
                                    },
                                };
                            }
                            else if (tabItem is TabItemWindow tabItemWindow) {
                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemCommand {
                                        Header = State.Constants.UI.CloseTab,
                                        Command = new Helpers.RelayCommand<object>(this.OnCloseTabItem),
                                        CommandParameterContext = contextMenuOpeningArgs.DataContext,
                                    },
                                };
                            }
                            break;

                        case Helpers.Enums.SelectionState.Multiple:
                            bool isTabItemAmongSelectedItems = _tabItemsSelectionCoordinator.SelectedItems
                                .Any(entry => ReferenceEquals(entry.Item, tabItem));

                            if (isTabItemAmongSelectedItems) {
                                this.ContextMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                                    new Helpers.MenuItemCommand {
                                        Header = State.Constants.UI.CloseSelectedTabs,
                                        Command = new Helpers.RelayCommand<object>(this.OnCloseTabItem),
                                        CommandParameterContext = contextMenuOpeningArgs.DataContext,
                                    }
                                };
                            }
                            else {
                                contextMenuOpeningArgs.ShouldOpen = false;
                                tabItem.IsSelected = true;
                            }
                            break;
                    }
                }
            }
        }

        private void OnTabItemContextMenuClosed(object parameter) {
            if (parameter is Controls.MenuControl.MenuClosedArgs contextMenuClosedArgs) {
                if (contextMenuClosedArgs.DataContext is TabItemBase tabItem) {
                    tabItem.Metadata.SetFlag("IsCtxMenuOpenned", false);
                }
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
                    if (this.VirtualMenuControl.CurrentMenuDataContext is TabItemDocument previousTabItemDocument) {
                        previousTabItemDocument.Metadata.SetFlag("IsVirtualMenuOpenned", false);
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
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnTabItemVirtualMenuOpen()");

            if (parameter is Controls.MenuControl.MenuOpeningArgs virtualMenuOpeningArgs) {
                if (virtualMenuOpeningArgs.DataContext is TabItemBase tabItem) {
                    
                    if (tabItem is TabItemDocument tabItemDocument) {
                        tabItemDocument.Metadata.SetFlag("IsVirtualMenuOpenned", true);

                        this.VirtualMenuItems = new ObservableCollection<Helpers.IMenuItem> {
                            new Helpers.MenuItemHeader {
                                Header = tabItem.Caption,
                            },
                            new Helpers.MenuItemCommand {
                                Header = State.Constants.UI.OpenTabLocation,
                                Command = new Helpers.RelayCommand<object>(this.OnOpenLocationTabItem),
                                CommandParameterContext = virtualMenuOpeningArgs.DataContext,
                            },
                            new Helpers.MenuItemCommand {
                                Header = State.Constants.UI.CloseTab,
                                Command = new Helpers.RelayCommand<object>(this.OnCloseTabItem),
                                CommandParameterContext = virtualMenuOpeningArgs.DataContext,
                            },
                        };
                        if (tabItemDocument.ShellDocument != null) {
                            tabItemDocument.UpdateProjectReferenceList();

                            if (tabItemDocument.ProjectReferenceList.Count > 1) {
                                this.VirtualMenuItems.Add(new Helpers.MenuItemSeparator());

                                foreach (var projRefEntry in tabItemDocument.ProjectReferenceList) {
                                    this.VirtualMenuItems.Add(new Helpers.MenuItemCommand {
                                        Header = projRefEntry.TabItemProject.Caption,
                                        Command = new Helpers.RelayCommand<object>(this.OnMoveTabItemToRelatedProject),
                                        CommandParameterContext = projRefEntry,
                                    });
                                }
                            }
                        }
                    }
                    else if (tabItem is TabItemWindow tabItemWindow) {
                        virtualMenuOpeningArgs.ShouldOpen = false;
                    }
                }
            }
        }
        private void OnTabItemVirtualMenuClosed(object parameter) {
            if (parameter is Controls.MenuControl.MenuClosedArgs virtualMenuClosedArgs) {
                if (virtualMenuClosedArgs.DataContext is TabItemBase tabItem) {
                    tabItem.Metadata.SetFlag("IsVirtualMenuOpenned", false);
                }
            }
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
        // ░ ScaleSelectorControl(s)
        //
        private void ApplyScaleUI() {
            if (this.DocumentScaleTransform != null) {
                this.DocumentScaleTransform.ScaleX = this.ScaleFactorUI;
                this.DocumentScaleTransform.ScaleY = this.ScaleFactorUI;
            }
        }

        private void ApplyScaleTabsCompactness() {
            Helpers.BaseUserControlResourceHelper.UpdateDynamicResource(this, "AppTabItemHeight", this.ScaleFactorTabsCompactness * 22);
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

            this.SortedTabItemsGroups.ToList();
            this.SortedTabItemsGroups.Clear();
            foreach (EnvDTE.Document document in _dte.Documents) {
                var tabItemDocument = new TabItemDocument(document);
                this.AddTabItemToAutoDeterminedGroupIfMissing(tabItemDocument);
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
                this.AddTabItemToAutoDeterminedGroupIfMissing(tabItemWindow);
            }

            this.SyncActiveDocumentWithPrimaryTabItem();

            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                if (VsShell.TextEditor.TextEditorControlHelper.IsEditorActive()) {
                    Helpers.GlobalFlags.SetFlag("TextEditorFrameFocused", true);
                    _textEditorOverlayController.Show();
                    _textEditorOverlayController.Update();
                }
            }), DispatcherPriority.Background);
        }


        private void AddDocumentToPreview(TabItemDocument tabItemDocument) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddDocumentToPreview()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // No need remove old because preview tab item group
            // guarded with ItemInsertMode == SingleWithReplaceExisting.

            var addedOrExistTabItem = this.AddTabItemToGroupIfMissing(tabItemDocument, new TabItemsPreviewGroup());
            addedOrExistTabItem.IsSelected = true;
        }


        private TabItemBase AddTabItemToAutoDeterminedGroupIfMissing(TabItemBase tabItem) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToAutoDeterminedGroupIfMissing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            TabItemsGroupBase tabItemGroup = null;

            if (tabItem is TabItemDocument doc) {
                tabItemGroup = new TabItemsDefaultGroup(doc.ShellDocument.GetDocumentProjectName());
            }
            else if (tabItem is TabItemWindow) {
                tabItemGroup = new TabItemsDefaultGroup("[Tool Windows]");
            }
            else {
                tabItemGroup = new TabItemsDefaultGroup("Other");
            }

            return this.AddTabItemToGroupIfMissing(tabItem, tabItemGroup);
        }


        private TabItemBase AddTabItemToGroupIfMissing(TabItemBase tabItem, TabItemsGroupBase tabItemGroup) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("AddTabItemToGroupIfMissing()");
            ThreadHelper.ThrowIfNotOnUIThread();

            Helpers.Diagnostic.Logger.LogParam($"tabItem.Caption = {tabItem?.Caption}");
            Helpers.Diagnostic.Logger.LogParam($"tabItemGroup.GroupName = {tabItemGroup?.GroupName}");

            var existingTabItem = this.FindTabItem(tabItem);
            if (existingTabItem != null) {
                return existingTabItem;
            }

            var existingGroup = this.SortedTabItemsGroups
                .FirstOrDefault(g => g.GetType() == tabItemGroup.GetType() && g.GroupName == tabItemGroup.GroupName);

            if (existingGroup == null) {
                this.SortedTabItemsGroups.Add(tabItemGroup);
                this.UpdateSeparatorsBetweenGroups();
                existingGroup = tabItemGroup;
            }

            // Изменяем флаги tabItem в зависимости от типа
            if (tabItem is TabItemDocument doc) {
                doc.IsPreviewTab = tabItemGroup is TabItemsPreviewGroup;
            }
            tabItem.IsPinnedTab = tabItemGroup is TabItemsPinnedGroup;

            existingGroup.Items.Add(tabItem);
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

            var previewGroup = this.SortedTabItemsGroups.FirstOrDefault(g => g is TabItemsPreviewGroup);
            if (previewGroup != null) {
                this.RemoveTabItemsGroup(previewGroup);
            }
            this.AddTabItemToAutoDeterminedGroupIfMissing(tabItemDocument);
            tabItemDocument.IsPreviewTab = false;
        }

        private void MoveDocumentToProjectGroup(TabItemDocument tabItemDocument, TabItemProject tabItemProject) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("MoveDocumentToProjectGroup()");
            ThreadHelper.ThrowIfNotOnUIThread();

            // Log params:
            Helpers.Diagnostic.Logger.LogParam($"tabItemDocument.FullName = {tabItemDocument?.FullName}");
            Helpers.Diagnostic.Logger.LogParam($"tabItemProject.Caption = {tabItemProject?.Caption}");

            this.RemoveTabItemFromGroups(tabItemDocument);
            this.AddTabItemToGroupIfMissing(tabItemDocument, new TabItemsDefaultGroup(tabItemProject.Caption));
        }

        private void RemoveTabItemFromGroups(TabItemBase tabItem) {
            foreach (var group in this.SortedTabItemsGroups.ToList()) {
                if (group.Items.Contains(tabItem)) {
                    this.RemoveTabItemFromGroup(tabItem, group);
                    break;
                }
            }
        }

        private void RemoveTabItemFromGroup(TabItemBase tabItem, TabItemsGroupBase group) {
            if (group.Items.Remove(tabItem)) {
                Helpers.Diagnostic.Logger.LogDebug($"Removed tab \"{tabItem.Caption}\" from group \"{group.GroupName}\"");

                if (!group.Items.Any()) {
                    this.RemoveTabItemsGroup(group);
                }
            }
        }

        private void RemoveTabItemsGroup(TabItemsGroupBase tabItemsGroup) {
            if (this.SortedTabItemsGroups.Remove(tabItemsGroup)) {
                Helpers.Diagnostic.Logger.LogDebug($"Removed group \"{tabItemsGroup.GroupName}\"");
                this.UpdateSeparatorsBetweenGroups();
            }
        }

        private void UpdateSeparatorsBetweenGroups() {
            // Remove existing separators
            foreach (var sep in this.SortedTabItemsGroups.OfType<SeparatorTabItemsGroup>().ToList()) {
                this.SortedTabItemsGroups.Remove(sep);
            }

            if (this.HasGroup<TabItemsPreviewGroup>() &&
                (this.HasGroup<TabItemsPinnedGroup>() || this.HasGroup<TabItemsDefaultGroup>())) {
                this.SortedTabItemsGroups.Add(new SeparatorTabItemsGroup("Preview-Pinned"));
            }

            if (this.HasGroup<TabItemsPinnedGroup>() && this.HasGroup<TabItemsDefaultGroup>()) {
                this.SortedTabItemsGroups.Add(new SeparatorTabItemsGroup("Pinned-Default"));
            }
        }


        private bool HasGroup<T>() where T : TabItemsGroupBase {
            return this.SortedTabItemsGroups.OfType<T>().Any();
        }


        // 
        // ░ Updating tabs
        //
        // Обновление документа в UI после изменения или переименования
        private void UpdateDocumentUI(string oldPath, string newPath = null) {
            foreach (var group in this.SortedTabItemsGroups) {
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


        private (TabItemDocument Item, TabItemsGroupBase Group)? FindTabItemWithGroup(EnvDTE.Document document) {
            var result = this.FindTabItemWithGroup(new TabItemDocument(document));
            if (result is { Item: TabItemDocument doc, Group: var group }) {
                return (doc, group);
            }
            return null;
        }

        private (TabItemWindow Item, TabItemsGroupBase Group)? FindTabItemWithGroup(EnvDTE.Window window) {
            var result = this.FindTabItemWithGroup(new TabItemWindow(window));
            if (result is { Item: TabItemWindow win, Group: var group }) {
                return (win, group);
            }
            return null;
        }

        private (TabItemBase Item, TabItemsGroupBase Group)? FindTabItemWithGroup(TabItemBase tabItem) {
            if (tabItem is TabItemWindow tabItemWindow) {
                return this.FindTabItemWithGroupBy<TabItemWindow>(
                    w => string.Equals(w.WindowId, tabItemWindow.WindowId, StringComparison.OrdinalIgnoreCase));
            }
            return this.FindTabItemWithGroupBy<TabItemBase>(
                t => string.Equals(t.FullName, tabItem.FullName, StringComparison.OrdinalIgnoreCase));
        }

        private (TabItemDocument Item, TabItemsGroupBase Group)? FindTabItemWithGroup(string documentFullName) {
            return this.FindTabItemWithGroupBy<TabItemDocument>(
                    d => string.Equals(d.FullName, documentFullName, StringComparison.OrdinalIgnoreCase));
        }


        private (T Item, TabItemsGroupBase Group)? FindTabItemWithGroupBy<T>(Func<T, bool> predicate) where T : TabItemBase {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var group in this.SortedTabItemsGroups) {
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

            foreach (var group in this.SortedTabItemsGroups) {
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
    }
}