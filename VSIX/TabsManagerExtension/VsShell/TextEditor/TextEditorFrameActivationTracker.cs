using System;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;
using System.Collections.Generic;
using Microsoft.VisualStudio.TextManager.Interop;

namespace TabsManagerExtension {
    /// <summary>
    /// Отслеживает смену активного окна в Visual Studio (IVsWindowFrame),
    /// и определяет, является ли оно редактором кода (текстовым редактором).
    /// Используется, чтобы, например, обновлять стиль UI в зависимости от того,
    /// активно ли текстовое окно (под фиолетовой рамкой).
    /// </summary>
    public sealed class TextEditorFrameActivationTracker : IVsSelectionEvents, IDisposable {
        private uint _selectionEventsCookie;
        private readonly IVsMonitorSelection _monitorSelection;
        private readonly IVsUIShell _uiShell;

        /// <summary>
        /// Вызывается при каждом изменении активного окна.
        /// Значение true означает, что текущее активное окно — это редактор (IVsTextView).
        /// </summary>
        public event Action<bool>? TextEditorFrameActivated;

        public TextEditorFrameActivationTracker() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _monitorSelection = (IVsMonitorSelection)Package.GetGlobalService(typeof(SVsShellMonitorSelection));
            _uiShell = (IVsUIShell)Package.GetGlobalService(typeof(SVsUIShell));

            // Подписываемся на события смены активного окна
            _monitorSelection.AdviseSelectionEvents(this, out _selectionEventsCookie);
        }

        /// <summary>
        /// Вызывается Visual Studio при изменении активного окна.
        /// </summary>
        public int OnElementValueChanged(uint elementid, object oldValue, object newValue) {
            if (elementid == (uint)VSConstants.VSSELELEMID.SEID_WindowFrame) {
                bool isTextEditorFrame = IsTextEditorFrame(newValue);
                this.TextEditorFrameActivated?.Invoke(isTextEditorFrame);
            }
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Проверяет, соответствует ли переданный IVsWindowFrame текстовому редактору.
        /// Важно: frame.GetProperty(VSFPROPID_DocView) возвращает IVsCodeWindow,
        /// а не напрямую IVsTextView. Поэтому нужно получить PrimaryView из CodeWindow.
        ///
        /// Если внутри окна есть действующий IVsTextView — считаем, что это редактор.
        /// Это означает, что Visual Studio будет считать это окно "редакторским",
        /// и вокруг него будет отображаться активная рамка (в т.ч. фиолетовая).
        /// </summary>
        private bool IsTextEditorFrame(object newValue) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (newValue is IVsWindowFrame frame) {
                // Получаем содержимое активного окна (View)
                frame.GetProperty((int)__VSFPROPID.VSFPROPID_DocView, out var docView);

                // Если это кодовое окно, запрашиваем его "основной" текстовый редактор
                if (docView is IVsCodeWindow codeWindow) {
                    if (codeWindow.GetPrimaryView(out var textView) == VSConstants.S_OK && textView != null) {
                        // Это полноценный редактор — возвращаем true
                        return true;
                    }
                }
            }

            return false;
        }

        public void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_selectionEventsCookie != 0) {
                _monitorSelection.UnadviseSelectionEvents(_selectionEventsCookie);
                _selectionEventsCookie = 0;
            }
        }

        // Остальные методы интерфейса можно не реализовывать — они нам не нужны
        public int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) => VSConstants.S_OK;

        public int OnSelectionChanged(
            IVsHierarchy pHierOld, uint itemidOld,
            IVsMultiItemSelect pMISOld, ISelectionContainer pSCOld,
            IVsHierarchy pHierNew, uint itemidNew,
            IVsMultiItemSelect pMISNew, ISelectionContainer pSCNew) => VSConstants.S_OK;
    }
}