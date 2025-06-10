using System;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;


namespace TabsManagerExtension.VsShell.TextEditor {
    public class TextEditorCommandFilter : OleCommandFilterBase {

        public event Action<Guid, uint>? CommandPassedThrough;
        public event Action<Guid, uint>? CommandIntercepted;
        public bool IsEnabled { get; set; } = false;

        private readonly IReadOnlyCollection<VSConstants.VSStd2KCmdID> _trackedCommands;
        private readonly IReadOnlyCollection<VSConstants.VSStd97CmdID> _trackedMappedToStd97Commands;
        private readonly Dictionary<VSConstants.VSStd2KCmdID, VSConstants.VSStd97CmdID> _2kTo97 = new();
        private readonly Dictionary<VSConstants.VSStd97CmdID, VSConstants.VSStd2KCmdID> _97To2k = new();

        public TextEditorCommandFilter(IEnumerable<VSConstants.VSStd2KCmdID> trackedCommands) {
            _trackedCommands = new HashSet<VSConstants.VSStd2KCmdID>(trackedCommands);
            _trackedMappedToStd97Commands = VsShell.VsCommandMapper.GetMappedStd97FromStd2kCommands(trackedCommands);
        }

        public Key? TryResolveKey(Guid cmdGroup, uint cmdId) {
            if (cmdGroup == VSConstants.VSStd2K && Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), (int)cmdId)) {
                var std2kCmd = (VSConstants.VSStd2KCmdID)cmdId;
                return VsShell.VsCommandMapper.TryMapToKey(std2kCmd);
            }

            if (cmdGroup == VSConstants.GUID_VSStandardCommandSet97 && Enum.IsDefined(typeof(VSConstants.VSStd97CmdID), (int)cmdId)) {
                var std97Cmd = (VSConstants.VSStd97CmdID)cmdId;
                if (_trackedMappedToStd97Commands.Contains(std97Cmd) && VsShell.VsCommandMapper.TryMapStd97ToStd2kCommand(std97Cmd, out var std2kCmd)) {
                    return VsShell.VsCommandMapper.TryMapToKey(std2kCmd);
                }
            }

            return null;
        }


        protected override bool TryHandleCommand(Guid cmdGroup, uint cmdId) {
            if (!IsEnabled) {
                return false;
            }

            if (cmdGroup == VSConstants.VSStd2K && Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), (int)cmdId)) {
                var cmd = (VSConstants.VSStd2KCmdID)cmdId;
                return _trackedCommands.Contains(cmd);
            }

            if (cmdGroup == VSConstants.GUID_VSStandardCommandSet97 && Enum.IsDefined(typeof(VSConstants.VSStd97CmdID), (int)cmdId)) {
                var cmd97 = (VSConstants.VSStd97CmdID)cmdId;
                return _trackedMappedToStd97Commands.Contains(cmd97);
            }

            return false;
        }

        protected override void OnCommandIntercepted(Guid cmdGroup, uint cmdId) {
            this.CommandIntercepted?.Invoke(cmdGroup, cmdId);
        }

        protected override void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) {
            this.CommandPassedThrough?.Invoke(cmdGroup, cmdId);
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
            VSConstants.VSStd2KCmdID.DELETE,
            VSConstants.VSStd2KCmdID.BACKSPACE,
        };

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsSelectionTrackerService.Instance.VsWindowFrameActivated += this.OnVsWindowFrameActivated;
            VsShell.TextEditor.Services.DocumentActivationTrackerService.Instance.OnDocumentActivated += this.OnDocumentActivatedExternally;
            
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
                this.Disable();
                _lastInputTarget = null;
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
            ThreadHelper.ThrowIfNotOnUIThread();

            // Используем диспетчер чтобы дать время IVsTextView стать активным.
            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                this.InstallToActiveEditor();
            }), DispatcherPriority.Background);
        }


        private void OnDocumentActivatedExternally(string _ /* file path unused */) {
            ThreadHelper.ThrowIfNotOnUIThread();

            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                this.InstallToActiveEditor();
            }), DispatcherPriority.Background);
        }


        private void OnTargetGotFocus(object sender, RoutedEventArgs e) {
            this.Enable();
            _lastInputTarget = sender as FrameworkElement;
        }


        private void OnTargetLostFocus(object sender, RoutedEventArgs e) {
            this.Disable();
            _lastInputTarget = null;
        }


        private void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) {
            // ...
        }


        private void OnCommandIntercepted(Guid cmdGroup, uint cmdId) {
            if (_lastInputTarget == null || !_lastInputTarget.IsKeyboardFocusWithin) {
                return;
            }

            if (_currentFilter != null) {
                var key = _currentFilter.TryResolveKey(cmdGroup, cmdId);
                if (key.HasValue) {
                    this.RedirectKeyInput(_lastInputTarget, key.Value);
                }
            }
        }


        private void InstallToActiveEditor() {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.InstallToActiveEditor()");

            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager != null && textManager.GetActiveView(1, null, out var newView) == VSConstants.S_OK) {
                if (newView != null) {
                    if (!ReferenceEquals(_currentTextView, newView)) {
                        this.UninstallFilter();
                        this.InstallFilterToView(newView);
                    }
                }
            }
        }


        private void InstallFilterToView(IVsTextView view) {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.InstallFilterToView()");
            ThreadHelper.ThrowIfNotOnUIThread();

            var filter = new TextEditorCommandFilter(_defaultCommands);
            int result = view.AddCommandFilter(filter, out IOleCommandTarget next);

            if (result == VSConstants.S_OK) {
                filter.SetNext(next);
                filter.CommandPassedThrough += this.OnCommandPassedThrough;
                filter.CommandIntercepted += this.OnCommandIntercepted;

                _currentFilter = filter;
                _currentTextView = view;
            }
            else {
                Helpers.Diagnostic.Logger.LogWarning($"[TextEditorCommandFilterController] Не удалось установить фильтр. HRESULT = 0x{result:X8}");
            }
        }


        private void UninstallFilter() {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.UninstallFilter()");

            if (_currentFilter != null) {
                _currentFilter.IsEnabled = false;
                _currentFilter.CommandIntercepted -= this.OnCommandIntercepted;
                _currentFilter.CommandPassedThrough -= this.OnCommandPassedThrough;
            }
            if (_currentTextView != null) {
                _currentTextView.RemoveCommandFilter(_currentFilter);
            }
            _currentTextView = null;
            _currentFilter = null;
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
    }
}