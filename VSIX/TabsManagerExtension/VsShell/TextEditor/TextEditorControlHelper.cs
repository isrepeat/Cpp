using System;
using System.Windows.Threading;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.ComponentModelHost;

namespace TabsManagerExtension.VsShell.TextEditor {
    /// <summary>
    /// Управляет фокусом и режимом редактирования активного текстового редактора.
    /// </summary>
    public static class TextEditorControlHelper {
        private static IReadOnlyRegion? _readOnlyRegion;

        /// <summary>
        /// Принудительно возвращает фокус редактору.
        /// Важно: это активирует редактор в WPF и внутри Visual Studio Shell,
        /// вызывая фиолетовую рамку вокруг вкладки.
        /// </summary>
        public static void FocusEditor() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Получаем глобальный сервис текстового менеджера — через него можно получить активный редактор.
            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager == null) {
                return;
            }

            // Получаем текущий активный IVsTextView (компонент редактора, управляющий текстом).
            // Параметр 'mustHaveFocus' = 1 означает, что мы хотим только активный редактор.
            // Этот объект нужен, чтобы передать Visual Studio команду на активацию редактора.
            if (textManager.GetActiveView(1, null, out IVsTextView vsTextView) == VSConstants.S_OK && vsTextView != null) {

                // Отправляем явную команду фокусировки редактору.
                // Это **не просто установка фокуса ввода**, а сигнал для среды Visual Studio (shell),
                // что данный редактор — текущий активный документ.
                //
                // На основании этого фокуса среда:
                // - Отображает цветную рамку (по умолчанию фиолетовую) вокруг редактора;
                // - Считает этот документ активным для команд, панелей, поиска и прочего;
                // - Не сбрасывает визуальную "рамку активности", даже если фокус ввода уходит на другие элементы UI.
                //
                // Визуальная рамка вокруг редактора (вкладки или окна) считается активной в том числе на основании
                // того, **для какого IVsTextView последним был вызван SendExplicitFocus()**.
                vsTextView.SendExplicitFocus();
            }
        }

        /// <summary>
        /// Возвращает WPF-фокус в редактор (визуальный элемент), не влияя на состояние Shell.
        /// </summary>
        public static void FocusEditorVisualElement() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var viewHost = TryGetActiveViewHost();
            viewHost?.TextView.VisualElement.Focus();
        }


        public static bool IsEditorActive() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager == null || textManager.GetActiveView(1, null, out var activeView) != VSConstants.S_OK) {
                return false;
            }

            var componentModel = (IComponentModel?)Package.GetGlobalService(typeof(SComponentModel));
            var adapter = componentModel?.GetService<IVsEditorAdaptersFactoryService>();
            var wpfViewHost = adapter?.GetWpfTextViewHost(activeView);

            if (wpfViewHost == null) {
                return false;
            }

            // WPF-фокус
            return wpfViewHost.TextView.VisualElement.IsKeyboardFocusWithin;
        }



        /// <summary>
        /// Делает текстовый редактор временно нередактируемым, добавляя read-only регион ко всему тексту.
        /// Обратный вызов восстанавливает возможность редактирования.
        /// </summary>
        public static void SetEditorEditable(bool isEditable) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var viewHost = TryGetActiveViewHost();
            if (viewHost == null) {
                return;
            }

            var buffer = viewHost.TextView.TextBuffer;

            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() => {
                ThreadHelper.ThrowIfNotOnUIThread();

                try {
                    if (!isEditable) {
                        if (_readOnlyRegion == null) {
                            var edit = buffer.CreateReadOnlyRegionEdit();
                            _readOnlyRegion = edit.CreateReadOnlyRegion(new Span(0, buffer.CurrentSnapshot.Length));
                            edit.Apply();
                        }
                    }
                    else {
                        if (_readOnlyRegion != null) {
                            var edit = buffer.CreateReadOnlyRegionEdit();
                            edit.RemoveReadOnlyRegion(_readOnlyRegion);
                            edit.Apply();
                            _readOnlyRegion = null;
                        }
                    }
                }
                catch (InvalidOperationException ex) {
                    System.Diagnostics.Debug.WriteLine($"[EditorControlHelper] SetEditorEditable failed: {ex.Message}");
                }
            }), DispatcherPriority.Background);
        }

        /// <summary>
        /// Скрывает или отображает каретку редактора без потери фокуса.
        /// </summary>
        public static void SetCaretVisible(bool isVisible) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var viewHost = TryGetActiveViewHost();
            if (viewHost != null) {
                viewHost.TextView.Caret.IsHidden = !isVisible;
            }
        }

        /// <summary>
        /// Получает текущий активный текстовый редактор (WPF view host).
        /// </summary>
        public static IWpfTextViewHost? TryGetActiveViewHost() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var textManager = (IVsTextManager)Package.GetGlobalService(typeof(SVsTextManager));
            if (textManager == null || textManager.GetActiveView(1, null, out var vsTextView) != VSConstants.S_OK) {
                return null;
            }

            var componentModel = (IComponentModel?)Package.GetGlobalService(typeof(SComponentModel));
            var adapter = componentModel?.GetService<IVsEditorAdaptersFactoryService>();
            return adapter?.GetWpfTextViewHost(vsTextView);
        }
    }
}