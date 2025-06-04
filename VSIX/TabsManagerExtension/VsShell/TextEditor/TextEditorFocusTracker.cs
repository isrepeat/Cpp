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
    public class TextEditorFocusTracker : IWpfTextViewCreationListener {
        [Import]
        internal ITextDocumentFactoryService TextDocumentFactoryService { get; set; }

        public static event Action<string> OnTextEditorDocumentChanged;

        private static string _lastHandledFilePath;


        public void TextViewCreated(IWpfTextView textView) {
            textView.GotAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"FocusGot [TextEditor]");
                Helpers.FocusWatcher.NotifyFocusGot("TextEditor");
            };

            textView.LostAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"FocusLost [TextEditor]");
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
}