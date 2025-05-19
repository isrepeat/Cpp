using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TabsManagerExtension.Controls {
    public partial class VirtualPopupControl : UserControl {
        private DispatcherTimer closePopupTimer;

        public VirtualPopupControl() {
            InitializeComponent();
        }

        // Свойство для передачи контента в Popup
        public object PopupContent {
            get { return GetValue(PopupContentProperty); }
            set { SetValue(PopupContentProperty, value); }
        }

        public static readonly DependencyProperty PopupContentProperty =
            DependencyProperty.Register("PopupContent", typeof(object), typeof(VirtualPopupControl), new PropertyMetadata(null));

        // Свойство для имени документа (с поддержкой привязки)
        public string DocumentName {
            get { return (string)GetValue(DocumentNameProperty); }
            set { SetValue(DocumentNameProperty, value); }
        }

        public static readonly DependencyProperty DocumentNameProperty =
            DependencyProperty.Register("DocumentName", typeof(string), typeof(VirtualPopupControl),
                new PropertyMetadata(string.Empty, OnDocumentNameChanged));

        // Обработчик изменений свойства DocumentName
        private static void OnDocumentNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            var control = d as VirtualPopupControl;
            control?.UpdateDocumentName();
        }

        // Обновление текста в TextBlock (если нужно)
        private void UpdateDocumentName() {
            // Это можно убрать, если привязка работает напрямую
            if (PopupElement != null && PopupElement.IsOpen) {
                Helpers.Diagnostic.Logger.LogDebug($"DocumentName updated to: {DocumentName}");
            }
        }

        // Метод для показа Popup
        public void ShowPopup(Point position, string documentName) {
            DocumentName = documentName; // Автоматически обновляем название документа
            PopupElement.HorizontalOffset = position.X;
            PopupElement.VerticalOffset = position.Y;
            PopupElement.IsOpen = true;
            closePopupTimer?.Stop();
        }

        // Запуск таймера закрытия Popup
        public void StartClosePopupTimer() {
            if (closePopupTimer == null) {
                closePopupTimer = new DispatcherTimer();
                closePopupTimer.Interval = TimeSpan.FromMilliseconds(300); // Задержка 300 мс
                closePopupTimer.Tick += (s, e) => {
                    closePopupTimer.Stop();
                    if (!PopupElement.IsMouseOver) {
                        HidePopup();
                    }
                };
            }

            closePopupTimer.Start();
        }

        // Метод для скрытия Popup
        private void HidePopup() {
            PopupElement.IsOpen = false;
        }

        // Событие при наведении на Popup (останавливаем таймер)
        private void PopupContent_MouseEnter(object sender, MouseEventArgs e) {
            closePopupTimer?.Stop();
        }

        // Событие при уходе мыши из Popup (запуск таймера на закрытие)
        private void PopupContent_MouseLeave(object sender, MouseEventArgs e) {
            StartClosePopupTimer();
        }
    }
}
