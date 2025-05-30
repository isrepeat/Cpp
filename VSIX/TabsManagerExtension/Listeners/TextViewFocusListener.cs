using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;

namespace TabsManagerExtension {
    [Export(typeof(IWpfTextViewCreationListener))]
    [ContentType("any")]
    [TextViewRole(PredefinedTextViewRoles.Document)]
    public class TextViewFocusListener : IWpfTextViewCreationListener {
        public void TextViewCreated(IWpfTextView textView) {
            textView.GotAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"[TextEditor] GOT FOCUS");
                Helpers.FocusWatcher.NotifyFocusGot("TextEditor");
            };

            textView.LostAggregateFocus += (sender, e) => {
                //Helpers.Diagnostic.Logger.LogDebug($"[TextEditor] LOST FOCUS");
                Helpers.FocusWatcher.NotifyFocusLost("TextEditor");
            };
        }
    }
}