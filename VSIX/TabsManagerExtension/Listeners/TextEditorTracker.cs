using System;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;
using System.Collections.Generic;

namespace TabsManagerExtension {
    [Export(typeof(IWpfTextViewCreationListener))]
    [ContentType("any")]
    [TextViewRole(PredefinedTextViewRoles.Document)]
    public class TextEditorTracker : IWpfTextViewCreationListener {
        [Import]
        internal ITextDocumentFactoryService TextDocumentFactoryService { get; set; }

        public static event Action<string> OnTextEditorDocumentChanged;

        private static string _lastHandledFilePath;


        public void TextViewCreated(IWpfTextView textView) {
            textView.GotAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"[TextEditor] GOT FOCUS");
                Helpers.FocusWatcher.NotifyFocusGot("TextEditor");
            };

            textView.LostAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"[TextEditor] LOST FOCUS");
                Helpers.FocusWatcher.NotifyFocusLost("TextEditor");
            };

            //// WARNING: not always triggered when document changed.
            //textView.LayoutChanged += (sender, e) => {
            //    if (!TextDocumentFactoryService.TryGetTextDocument(textView.TextBuffer, out var doc)) {
            //        return;
            //    }

            //    string filePath = doc.FilePath;

            //    if (string.IsNullOrEmpty(filePath) ||
            //        string.Equals(filePath, _lastHandledFilePath, StringComparison.OrdinalIgnoreCase)) {
            //        return;
            //    }

            //    _lastHandledFilePath = filePath;

            //    // Асинхронный вызов, чтобы не мешать редактору
            //    ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
            //        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            //        OnTextEditorDocumentChanged?.Invoke(filePath);
            //    });
            //};
        }
    }



    internal static class DocumentActivationTracker {
        private static IVsRunningDocumentTable _rdt;
        private static uint _cookie;
        private static string _lastActivatedFilePath;

        public static event Action<string> OnDocumentActivated;

        public static void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_rdt != null) {
                return;
            }

            _rdt = (IVsRunningDocumentTable)Package.GetGlobalService(typeof(SVsRunningDocumentTable));
            if (_rdt == null) {
                throw new InvalidOperationException("Failed to get IVsRunningDocumentTable");
            }

            var handler = new RdtEventHandler();
            int hr = _rdt.AdviseRunningDocTableEvents(handler, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);
        }

        public static void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_rdt != null && _cookie != 0) {
                _rdt.UnadviseRunningDocTableEvents(_cookie);
                _cookie = 0;
                _rdt = null;
            }
        }

        private class RdtEventHandler : IVsRunningDocTableEvents {
            public int OnBeforeDocumentWindowShow(uint docCookie, int fFirstShow, IVsWindowFrame pFrame) {
                ThreadHelper.ThrowIfNotOnUIThread();

                _rdt.GetDocumentInfo(docCookie, out _, out _, out _, out string moniker,
                    out _, out _, out _);

                if (!string.IsNullOrEmpty(moniker) &&
                    !string.Equals(moniker, _lastActivatedFilePath, StringComparison.OrdinalIgnoreCase)) {
                    _lastActivatedFilePath = moniker;
                    OnDocumentActivated?.Invoke(moniker);
                }

                return VSConstants.S_OK;
            }

            public int OnAfterDocumentWindowHide(uint docCookie, IVsWindowFrame pFrame) => VSConstants.S_OK;
            public int OnAfterSave(uint docCookie) => VSConstants.S_OK;
            public int OnBeforeSave(uint docCookie) => VSConstants.S_OK;
            public int OnAfterAttributeChange(uint docCookie, uint grfAttribs) => VSConstants.S_OK;
            public int OnAfterFirstDocumentLock(uint docCookie, uint lockType, uint readLocks, uint editLocks) => VSConstants.S_OK;
            public int OnBeforeLastDocumentUnlock(uint docCookie, uint lockType, uint readLocks, uint editLocks) => VSConstants.S_OK;
        }
    }
}