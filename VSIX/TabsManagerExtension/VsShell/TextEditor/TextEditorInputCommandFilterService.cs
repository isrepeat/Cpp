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
using TabsManagerExtension.VsShell.Document.Services;


namespace TabsManagerExtension.VsShell.TextEditor.Services {
    /// <summary>
    /// Серивис, автоматически отслеживающий активный редактор и переустанавливающий фильтр команд.
    /// Внешние подписчики могут подписаться на события команд один раз — фильтр будет переустановлен при смене редактора.
    /// </summary>
    public sealed class TextEditorInputCommandFilterService :
        TabsManagerExtension.Services.SingletonServiceBase<TextEditorInputCommandFilterService>,
        TabsManagerExtension.Services.IExtensionService {

        private static readonly VSConstants.VSStd2KCmdID[] _trackedStd2Commands = new[] {
            VSConstants.VSStd2KCmdID.TAB,
            VSConstants.VSStd2KCmdID.UP,
            VSConstants.VSStd2KCmdID.DOWN,
            VSConstants.VSStd2KCmdID.LEFT,
            VSConstants.VSStd2KCmdID.RIGHT,
            VSConstants.VSStd2KCmdID.RETURN,
            VSConstants.VSStd2KCmdID.DELETE,
            VSConstants.VSStd2KCmdID.BACKSPACE,
        };

        private static readonly VSConstants.VSStd97CmdID[] _trackedStd97Commands = new[] {
            VSConstants.VSStd97CmdID.Delete,
        };

        private IVsTextView? _currentTextView;
        private TextEditorCommandFilter? _currentFilter;

        private readonly HashSet<FrameworkElement> _trackedElements = new();
        private FrameworkElement? _lastInputTarget;

        public TextEditorInputCommandFilterService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return new[] {
                typeof(VsShell.Document.Services.VsDocumentActivationTrackerService),
                typeof(VsShell.Solution.Services.VsWindowFrameActivationTrackerService),
            };
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Document.Services.VsDocumentActivationTrackerService.Instance.OnDocumentActivated += this.OnDocumentActivatedExternally;
            VsShell.Solution.Services.VsWindowFrameActivationTrackerService.Instance.VsWindowFrameActivated += this.OnVsWindowFrameActivated;
            
            this.InstallToActiveEditor();

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorInputCommandFilterService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Solution.Services.VsWindowFrameActivationTrackerService.Instance.VsWindowFrameActivated -= this.OnVsWindowFrameActivated;
            VsShell.Document.Services.VsDocumentActivationTrackerService.Instance.OnDocumentActivated -= this.OnDocumentActivatedExternally;
            this.UninstallFilter();
            ClearInstance();

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorInputCommandFilterService] Shutdown.");
        }


        //
        // Api
        //
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


        //
        // Event handlers
        //
        private void OnDocumentActivatedExternally(_EventArgs.DocumentNavigationEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                this.InstallToActiveEditor();
            }), DispatcherPriority.Background);
        }


        private void OnVsWindowFrameActivated(IVsWindowFrame vsWindowFrame) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Используем диспетчер чтобы дать время IVsTextView стать активным.
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

        private void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) {
            // ...
        }


        //
        // Internal logic
        //
        private void InstallToActiveEditor() {
            //using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("TextEditorCommandFilterService.InstallToActiveEditor()");

            if (PackageServices.VsTextManager.GetActiveView(1, null, out var newView) == VSConstants.S_OK) {
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

            var filter = new TextEditorCommandFilter(_trackedStd2Commands, _trackedStd97Commands);
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