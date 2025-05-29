using Helpers;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;
using System.ComponentModel.Composition;

namespace TabsManagerExtension {

    [Export(typeof(IWpfTextViewCreationListener))]
    [ContentType("any")] // для обычного кода
    [TextViewRole(PredefinedTextViewRoles.Document)]
    public class MyTextViewListener : IWpfTextViewCreationListener {
        public void TextViewCreated(IWpfTextView textView) {
            textView.GotAggregateFocus += (sender, e) => {
                FocusWatcher.NotifyFocusGot("TextEditor");
            };

            textView.LostAggregateFocus += (sender, e) => {
                FocusWatcher.NotifyFocusLost("TextEditor");
            };
        }
    }
}
