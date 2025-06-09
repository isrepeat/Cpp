using System;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;
using System.Windows.Threading;


namespace TabsManagerExtension.VsShell.TextEditor {
    public class TextEditorCommandFilter : OleCommandFilterBase {
        /// <summary>
        /// Управляет тем, активна ли фильтрация команд.
        /// Если false — все команды проходят дальше без перехвата.
        /// </summary>
        public bool IsEnabled { get; set; } = false;

        /// <summary>
        /// Вызывается, когда команда была перехвачена и не передана редактору.
        /// </summary>
        public event Action<VSConstants.VSStd2KCmdID>? CommandIntercepted;

        /// <summary>
        /// Вызывается, когда команда была передана дальше (редактору).
        /// </summary>
        public event Action<VSConstants.VSStd2KCmdID>? CommandPassedThrough;


        private readonly HashSet<VSConstants.VSStd2KCmdID> _interceptedCommands;

        public TextEditorCommandFilter(IEnumerable<VSConstants.VSStd2KCmdID> blockedCommands) {
            _interceptedCommands = new HashSet<VSConstants.VSStd2KCmdID>(blockedCommands);
        }

        protected override bool TryHandleCommand(Guid cmdGroup, uint cmdId) {
            if (!IsEnabled || cmdGroup != VSConstants.VSStd2K) {
                return false;
            }

            var cmd = (VSConstants.VSStd2KCmdID)cmdId;
            return _interceptedCommands.Contains(cmd);
        }

        protected override void OnCommandIntercepted(Guid cmdGroup, uint cmdId) {
            if (cmdGroup == VSConstants.VSStd2K && Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), (int)cmdId)) {
                var cmd = (VSConstants.VSStd2KCmdID)cmdId;
                CommandIntercepted?.Invoke(cmd);
            }
        }

        protected override void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) {
            if (cmdGroup == VSConstants.VSStd2K && Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), (int)cmdId)) {
                var cmd = (VSConstants.VSStd2KCmdID)cmdId;
                CommandPassedThrough?.Invoke(cmd);
            }
        }

        public static string FormatCommand(VSConstants.VSStd2KCmdID cmd) {
            if (Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), cmd)) {
                var name = ((VSConstants.VSStd2KCmdID)cmd).ToString();
                return $"VSStd2K::{name}";
            }
            return $"VSStd2K::Unknown({cmd})";
        }
    }
}


namespace TabsManagerExtension.VsShell.TextEditor.Services {
    /// <summary>
    /// Серивис, автоматически отслеживающий активный редактор и переустанавливающий фильтр команд.
    /// Внешние подписчики могут подписаться на события команд один раз — фильтр будет переустановлен при смене редактора.
    /// </summary>
    public sealed class TextEditorCommandFilterService :
        TabsManagerExtension.Services.SingletonServiceBase<TextEditorCommandFilterService>,
        TabsManagerExtension.Services.IExtensionService {

        public event Action<VSConstants.VSStd2KCmdID>? CommandIntercepted;
        public event Action<VSConstants.VSStd2KCmdID>? CommandPassedThrough;

        private IVsTextView? _currentTextView;
        private TextEditorCommandFilter? _currentFilter;

        private readonly HashSet<FrameworkElement> _trackedElements = new();
        private FrameworkElement? _lastInputTarget;

        private static readonly VSConstants.VSStd2KCmdID[] _defaultCommands = new[] {
            VSConstants.VSStd2KCmdID.TAB,
            VSConstants.VSStd2KCmdID.UP,
            VSConstants.VSStd2KCmdID.DOWN,
            VSConstants.VSStd2KCmdID.LEFT,
            VSConstants.VSStd2KCmdID.RIGHT,
            VSConstants.VSStd2KCmdID.RETURN,
            VSConstants.VSStd2KCmdID.BACKSPACE,
        };

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsSelectionTrackerService.Instance.VsWindowFrameActivated += this.OnVsWindowFrameActivated;
            VsShell.TextEditor.Services.DocumentActivationTrackerService.Instance.OnDocumentActivated += this.OnDocumentActivatedExternally;
            this.CommandIntercepted += this.OnCommandIntercepted;
            this.InstallToActiveEditor();

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorCommandFilterController] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.TextEditor.Services.DocumentActivationTrackerService.Instance.OnDocumentActivated -= this.OnDocumentActivatedExternally;
            VsShell.Services.VsSelectionTrackerService.Instance.VsWindowFrameActivated -= this.OnVsWindowFrameActivated;
            this.UninstallFilter();
            ClearInstance();

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorCommandFilterController] Shutdown.");
        }


        public void AddTrackedInputElement(FrameworkElement element) {
            if (element == null || _trackedElements.Contains(element)) {
                return;
            }

            _trackedElements.Add(element);

            element.GotKeyboardFocus += OnTargetGotFocus;
            element.LostKeyboardFocus += OnTargetLostFocus;

            if (element.IsKeyboardFocusWithin) {
                this.Enable();
                _lastInputTarget = element;
            }
        }


        public void RemoveTrackedInputElement(FrameworkElement element) {
            if (element == null || !_trackedElements.Contains(element)) {
                return;
            }

            _trackedElements.Remove(element);

            element.GotKeyboardFocus -= OnTargetGotFocus;
            element.LostKeyboardFocus -= OnTargetLostFocus;

            if (ReferenceEquals(_lastInputTarget, element)) {
                this.UpdateFocusStateDeferred();
            }
        }


        public void Enable() {
            if (_currentFilter != null) {
                _currentFilter.IsEnabled = true;
            }
        }

        public void Disable() {
            if (_currentFilter != null) {
                _currentFilter.IsEnabled = false;
            }
        }


        private void OnVsWindowFrameActivated(IVsWindowFrame vsWindowFrame) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.OnVsWindowFrameActivated()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.OnActiveViewChanged();
        }


        private void OnDocumentActivatedExternally(string _ /* file path unused */) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.OnDocumentActivatedExternally()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this.OnActiveViewChanged();
        }


        private void OnActiveViewChanged() {
            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager != null &&
                textManager.GetActiveView(1, null, out var newView) == VSConstants.S_OK &&
                newView != null &&
                !ReferenceEquals(_currentTextView, newView)) {

                this.UninstallFilter();
                this.InstallFilterToView(newView);
            }
        }


        private void OnCommandIntercepted(VSConstants.VSStd2KCmdID cmd) {
            if (_lastInputTarget != null && _lastInputTarget.IsKeyboardFocusWithin) {
                var key = this.TryMapCommandToKey(cmd);
                if (key.HasValue) {
                    this.RedirectKeyInput(_lastInputTarget, key.Value);
                }
            }
        }


        private void OnTargetGotFocus(object sender, RoutedEventArgs e) {
            this.Enable();
            _lastInputTarget = sender as FrameworkElement;
        }


        private void OnTargetLostFocus(object sender, RoutedEventArgs e) {
            // нужен, чтобы дождаться перехода фокуса (иначе LostFocus будет до GotFocus следующего).
            this.UpdateFocusStateDeferred();
        }



        private void InstallToActiveEditor() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager != null &&
                textManager.GetActiveView(1, null, out var view) == VSConstants.S_OK &&
                view != null) {
                this.InstallFilterToView(view);
            }
        }

        private void InstallFilterToView(IVsTextView view) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var filter = new TextEditorCommandFilter(_defaultCommands);
            int result = view.AddCommandFilter(filter, out IOleCommandTarget next);

            if (result == VSConstants.S_OK) {
                filter.SetNext(next);
                filter.CommandIntercepted += cmd => this.CommandIntercepted?.Invoke(cmd);
                filter.CommandPassedThrough += cmd => this.CommandPassedThrough?.Invoke(cmd);

                _currentFilter = filter;
                _currentTextView = view;

                Helpers.Diagnostic.Logger.LogDebug("[TextEditorCommandFilterController] Фильтр установлен.");
            }
            else {
                Helpers.Diagnostic.Logger.LogWarning($"[TextEditorCommandFilterController] Не удалось установить фильтр. HRESULT = 0x{result:X8}");
            }
        }

        private void UninstallFilter() {
            if (_currentFilter != null) {
                _currentFilter.IsEnabled = false;
            }
            _currentTextView = null;
            _currentFilter = null;

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorCommandFilterController] Фильтр отключён.");
        }



        /// <summary>
        /// Проверяет, остался ли фокус внутри одного из отслеживаемых элементов.
        /// Если нет — отключает фильтрацию ввода и сбрасывает текущую цель.
        /// Использует отложенное выполнение (BeginInvoke), потому что в момент LostFocus
        /// следующий элемент ещё не получил фокус, и прямой вызов даст ложный результат.
        /// </summary>
        private void UpdateFocusStateDeferred() {
            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                if (!_trackedElements.Any(el => el.IsKeyboardFocusWithin)) {
                    this.Disable();
                    _lastInputTarget = null;
                }
            }), DispatcherPriority.Background);
        }

        private void RedirectKeyInput(FrameworkElement target, Key key) {
            var inputEvent = new KeyEventArgs(
                Keyboard.PrimaryDevice,
                PresentationSource.FromVisual(target),
                Environment.TickCount,
                key);
            inputEvent.RoutedEvent = Keyboard.KeyDownEvent;

            InputManager.Current.ProcessInput(inputEvent);
        }

        private Key? TryMapCommandToKey(VSConstants.VSStd2KCmdID cmd) {
            return cmd switch {
                VSConstants.VSStd2KCmdID.TAB => Key.Tab,
                VSConstants.VSStd2KCmdID.UP => Key.Up,
                VSConstants.VSStd2KCmdID.DOWN => Key.Down,
                VSConstants.VSStd2KCmdID.LEFT => Key.Left,
                VSConstants.VSStd2KCmdID.RIGHT => Key.Right,
                VSConstants.VSStd2KCmdID.RETURN => Key.Enter,
                VSConstants.VSStd2KCmdID.BACKSPACE => Key.Back,
                _ => null
            };
        }
    }
}