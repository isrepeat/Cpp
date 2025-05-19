using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TabsManagerExtension.Controls {
    public partial class VirtualPopupControl : UserControl {
        private DispatcherTimer closePopupTimer;
        private bool isMouseOverInteractiveArea = false;
        private bool hasUserInteracted = false; // Новый флаг взаимодействия
        private double defaultOpacity = 0.1; // Начальная прозрачность (10%)
        private double maxOpacity = 1.0;     // Полная прозрачность (100%)

        public VirtualPopupControl() {
            InitializeComponent();
            PopupOpacity = defaultOpacity;
        }

        // Свойство для управления прозрачностью Popup (привязано к Border)
        public double PopupOpacity {
            get { return (double)GetValue(PopupOpacityProperty); }
            set { SetValue(PopupOpacityProperty, value); }
        }

        public static readonly DependencyProperty PopupOpacityProperty =
            DependencyProperty.Register("PopupOpacity", typeof(double), typeof(VirtualPopupControl),
                new PropertyMetadata(0.1));

        // Свойство для передачи контента в Popup
        public object PopupContent {
            get { return GetValue(PopupContentProperty); }
            set { SetValue(PopupContentProperty, value); }
        }

        public static readonly DependencyProperty PopupContentProperty =
            DependencyProperty.Register("PopupContent", typeof(object), typeof(VirtualPopupControl), new PropertyMetadata(null));

        public string DocumentName {
            get { return (string)GetValue(DocumentNameProperty); }
            set { SetValue(DocumentNameProperty, value); }
        }

        public static readonly DependencyProperty DocumentNameProperty =
            DependencyProperty.Register("DocumentName", typeof(string), typeof(VirtualPopupControl),
                new PropertyMetadata(string.Empty, OnDocumentNameChanged));

        private static void OnDocumentNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            var control = d as VirtualPopupControl;
            control?.UpdateDocumentName();
        }

        private void UpdateDocumentName() {
            if (PopupElement != null && PopupElement.IsOpen) {
                Helpers.Diagnostic.Logger.LogDebug($"DocumentName updated to: {DocumentName}");
            }
        }

        // Запуск таймера закрытия Popup
        public void StartClosePopupTimer() {
            if (closePopupTimer == null) {
                closePopupTimer = new DispatcherTimer();
                closePopupTimer.Interval = TimeSpan.FromMilliseconds(300);
                closePopupTimer.Tick += (s, e) => {
                    closePopupTimer.Stop();
                    if (!PopupElement.IsMouseOver) {
                        HidePopup();
                    }
                };
            }

            closePopupTimer.Start();
        }

        // Метод для показа Popup
        public void ShowPopup(Point position, string documentName) {
            if (!isMouseOverInteractiveArea) {
                return;
            }

            DocumentName = documentName;
            PopupElement.HorizontalOffset = position.X;
            PopupElement.VerticalOffset = position.Y;
            PopupElement.IsOpen = true;
            closePopupTimer?.Stop();
            PopupOpacity = hasUserInteracted ? maxOpacity : defaultOpacity; // Прозрачность зависит от флага
        }

        // Метод для скрытия Popup
        private void HidePopup() {
            PopupElement.IsOpen = false;
            isMouseOverInteractiveArea = false;
            hasUserInteracted = false; // Сбрасываем флаг при закрытии
            PopupOpacity = defaultOpacity;
        }

        // Методы для отслеживания состояния InteractiveArea
        public void InteractiveArea_MouseEnter() {
            isMouseOverInteractiveArea = true;
        }

        public void InteractiveArea_MouseLeave() {
            isMouseOverInteractiveArea = false;
            StartClosePopupTimer();
        }

        // Событие при наведении на Popup (увеличиваем прозрачность)
        private void PopupContent_MouseEnter(object sender, MouseEventArgs e) {
            closePopupTimer?.Stop();
            PopupOpacity = maxOpacity;
            hasUserInteracted = true; // Фиксируем факт взаимодействия
        }

        // Событие при уходе мыши из Popup (уменьшаем прозрачность)
        private void PopupContent_MouseLeave(object sender, MouseEventArgs e) {
            if (!hasUserInteracted) {
                PopupOpacity = defaultOpacity;
            }
            StartClosePopupTimer();
        }
    }
}
