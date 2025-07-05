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
    /// Service that installs a command filter on the active IVsTextView to track navigation commands
    /// like Go To Definition (F12) or Open File (CTRL+SHIFT+G).
    ///
    /// Whenever such a navigation command is executed and leads to activating another file,
    /// this service raises the <b><see cref="NavigatedToFile"/></b> event with both the old and new file paths.
    /// </summary>
    public sealed class TextEditorFileNavigationCommandFilterService :
        TabsManagerExtension.Services.SingletonServiceBase<TextEditorFileNavigationCommandFilterService>,
        TabsManagerExtension.Services.IExtensionService {

        /// <summary>
        /// Raised when navigation to a new file occurs via navigation command.
        /// Provides the old file and the new file paths.
        /// </summary>
        public event Action<_EventArgs.DocumentNavigationEventArgs>? OnNavigatedToDocument;

        private static readonly VSConstants.VSStd2KCmdID[] _navigationStd2kCommands = new[] {
            VSConstants.VSStd2KCmdID.OPENFILE,
        };

        private static readonly VSConstants.VSStd97CmdID[] _navigationStd97Commands = new[] {
            VSConstants.VSStd97CmdID.FileOpen,
            VSConstants.VSStd97CmdID.GotoDefn,
        };

        private IVsTextView? _currentTextView;
        private TextEditorCommandFilter? _currentFilter;

        private bool _navigationToFileScheduled = false;

        public TextEditorFileNavigationCommandFilterService() { }

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

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorFileNavigationCommandFilterService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Solution.Services.VsWindowFrameActivationTrackerService.Instance.VsWindowFrameActivated -= this.OnVsWindowFrameActivated;
            VsShell.Document.Services.VsDocumentActivationTrackerService.Instance.OnDocumentActivated -= this.OnDocumentActivatedExternally;
            this.UninstallFilter();
            ClearInstance();

            Helpers.Diagnostic.Logger.LogDebug("[TextEditorFileNavigationCommandFilterService] Shutdown.");
        }


        //
        // Event handlers
        //
        private void OnDocumentActivatedExternally(_EventArgs.DocumentNavigationEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_navigationToFileScheduled) {
                _navigationToFileScheduled = false;
                this.OnNavigatedToDocument?.Invoke(e);
            }

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


        private void OnCommandIntercepted(Guid cmdGroup, uint cmdId) {
            // ...
        }


        private void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) {
            if (cmdGroup == VSConstants.VSStd2K) {
                var std2kCmd = (VSConstants.VSStd2KCmdID)cmdId;

                if (_navigationStd2kCommands.Contains(std2kCmd)) {
                    _navigationToFileScheduled = true;
                }
            }
            else if (cmdGroup == VSConstants.GUID_VSStandardCommandSet97) {
                var std97Cmd = (VSConstants.VSStd97CmdID)cmdId;

                if (_navigationStd97Commands.Contains(std97Cmd)) {
                    _navigationToFileScheduled = true;
                }
            }
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

            var filter = new TextEditorCommandFilter();
            int result = view.AddCommandFilter(filter, out IOleCommandTarget next);

            if (result == VSConstants.S_OK) {
                filter.SetNext(next);
                filter.CommandPassedThrough += this.OnCommandPassedThrough;
                filter.CommandIntercepted += this.OnCommandIntercepted;
                filter.IsEnabled = true;

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
    }
}