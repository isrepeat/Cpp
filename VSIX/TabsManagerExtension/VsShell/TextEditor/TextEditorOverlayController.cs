using System;
using System.Linq;
using System.Windows;
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

namespace TabsManagerExtension.VsShell.TextEditor.Overlay {

    /// <summary>
    /// Отвечает за управление жизненным циклом визуального оверлея (`TextEditorOverlayControl`),
    /// который добавляется поверх текстового редактора через `AdornerOverlayManager`.
    /// 
    /// Контроллер следит за количеством открытых документов через DTE и:
    /// - создаёт оверлей при появлении хотя бы одного редактора,
    /// - удаляет оверлей, когда все редакторы закрыты.
    /// </summary>
    public class TextEditorOverlayController {
        private readonly EnvDTE80.DTE2 _dte;

        private Helpers.AdornerOverlayManager<Controls.TextEditorOverlayControl>? _overlayManager;

        /// <summary>
        /// Инициализирует контроллер, привязанный к текущему экземпляру Visual Studio (DTE).
        /// </summary>
        public TextEditorOverlayController(EnvDTE80.DTE2 dte) {
            _dte = dte;
        }


        public void Show() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.EnsureCreated();
        }

        public void Hide() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this.EnsureDisposed();
        }

        public void Update() {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (_overlayManager == null || !_overlayManager.IsAttached) {
                return;
            }

            var textEditorOverlayControl = _overlayManager.Overlay;
            if (textEditorOverlayControl != null) {
                textEditorOverlayControl.LoadAnchorsFromActiveDocument();
            }
        }



        /// <summary>
        /// Обновляет состояние оверлея:
        /// - создаёт, если есть хотя бы один открытый документ;
        /// - уничтожает, если все документы закрыты.
        /// </summary>
        public void UpdateState() {
            ThreadHelper.ThrowIfNotOnUIThread();

            bool hasOpenDocuments = _dte.Documents.Cast<EnvDTE.Document>().Any();
            if (hasOpenDocuments) {
                this.EnsureCreated();
            }
            else {
                this.EnsureDisposed();
            }
        }

        /// <summary>
        /// Создаёт визуальный оверлей, если он ещё не создан или был откреплён.
        /// </summary>
        private void EnsureCreated() {
            if (_overlayManager != null && _overlayManager.IsAttached) {
                return;
            }

            var overlayTargaet = this.TryFindOverlayTarget();
            if (overlayTargaet == null) {
                // Редактор ещё не загружен — повторим позже
                Helpers.Diagnostic.Logger.LogDebug("TextEditor not loaded yet, try later");
                Dispatcher.CurrentDispatcher.BeginInvoke(new Action(UpdateState), DispatcherPriority.ApplicationIdle);
                return;
            }
            
            var overlay = new Controls.TextEditorOverlayControl();
            _overlayManager = new Helpers.AdornerOverlayManager<Controls.TextEditorOverlayControl>(
                overlayTargaet,
                overlay
                );
            Helpers.Diagnostic.Logger.LogDebug("AdornerOverlayManager created");
        }

        /// <summary>
        /// Удаляет визуальный оверлей, если он существует.
        /// </summary>
        private void EnsureDisposed() {
            if (_overlayManager == null) {
                return;
            }

            _overlayManager.Remove();
            _overlayManager = null;

            Helpers.Diagnostic.Logger.LogDebug("AdornerOverlayManager disposed");
        }


        /// <summary>
        /// Ищет WpfTextViewHost и возвращает его родителя с именем PART_ContentPanel.
        /// </summary>
        private FrameworkElement TryFindOverlayTarget() {
            // NOTE: Элемент с именем "PART_ContentPanel" не уникален, поэтому сначала
            //       ищем уникальный элемент текстового редаквтора - WpfTextViewHost, а затем его родителя.
            var viewHost = Helpers.VisualTree.FindChildByType(Application.Current.MainWindow, "WpfTextViewHost");
            if (viewHost == null) {
                return null;
            }

            var panel = Helpers.VisualTree.FindParentByName(viewHost, "PART_ContentPanel");
            return panel;
        }
    }
}