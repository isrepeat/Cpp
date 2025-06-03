using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TabsManagerExtension.Controls {
    public partial class VirtualPopupControl : UserControl {
        private DispatcherTimer showTimer;
        private DispatcherTimer hideTimer;

        private bool hasUserInteracted = false;
        private bool isPopupVisible = false;

        private double defaultOpacity = 0.1;
        private double maxOpacity = 1.0;

        private Point pendingPosition;
        private object pendingDataContext;

        public VirtualPopupControl() {
            this.InitializeComponent();
            this.PopupOpacity = this.defaultOpacity;
        }

        // Свойство для управления прозрачностью Popup (привязано к Border)
        public double PopupOpacity {
            get { return (double)GetValue(PopupOpacityProperty); }
            set { SetValue(PopupOpacityProperty, value); }
        }

        public static readonly DependencyProperty PopupOpacityProperty =
            DependencyProperty.Register(
                "PopupOpacity",
                typeof(double),
                typeof(VirtualPopupControl),
                new PropertyMetadata(0.1));


        // Свойство для передачи контента в Popup
        public object PopupContent {
            get { return GetValue(PopupContentProperty); }
            set { SetValue(PopupContentProperty, value); }
        }

        public static readonly DependencyProperty PopupContentProperty =
            DependencyProperty.Register(
                "PopupContent",
                typeof(object),
                typeof(VirtualPopupControl),
                new PropertyMetadata(null));

        /// <summary>
        /// Публичный вызов показа popup.
        /// Если popup уже открыт — обновляет контент и позицию мгновенно.
        /// Если popup закрыт — запускает таймер, и через 300 мс показывает popup, если пользователь всё ещё на элементе.
        /// </summary>
        public void Show(Point position, object dataContext) {
            this.CancelHideTimer(); // предотвращаем закрытие, если вдруг оно было запущено

            if (this.PopupElement.IsOpen) {
                // Popup уже открыт — просто обновим содержимое
                this.UpdateContent(position, dataContext);
                return;
            }

            // Сохраняем параметры до срабатывания таймера
            this.pendingPosition = position;
            this.pendingDataContext = dataContext;

            if (this.showTimer == null) {
                this.showTimer = new DispatcherTimer {
                    Interval = TimeSpan.FromMilliseconds(300)
                };

                this.showTimer.Tick += (s, e) => {
                    this.showTimer.Stop();

                    // После задержки — отображаем popup
                    this.InternalShowPopup(this.pendingPosition, this.pendingDataContext);
                    this.pendingDataContext = null;
                };
            }

            this.showTimer.Stop(); // сбрасываем, если вызов был повторным
            this.showTimer.Start();
        }

        /// <summary>
        /// Публичный вызов скрытия popup.
        /// Таймер даёт пользователю 300 мс «переместиться» в сам popup.
        /// Если мышь в popup не попала — он закроется.
        /// </summary>
        public void Hide() {
            this.CancelShowTimer(); // отменяем отложенный показ, если ещё не отработал

            if (this.hideTimer == null) {
                this.hideTimer = new DispatcherTimer {
                    Interval = TimeSpan.FromMilliseconds(300)
                };

                this.hideTimer.Tick += (s, e) => {
                    this.hideTimer.Stop();

                    // Если мышь не в popup — закрываем
                    if (!this.PopupElement.IsMouseOver) {
                        this.InternalHidePopup();
                    }
                };
            }

            this.hideTimer.Stop(); // защита от повторного вызова
            this.hideTimer.Start();
        }

        /// <summary>
        /// Обновление содержимого и позиции popup без его закрытия.
        /// Вызывается при повторном наведении, когда popup уже показан.
        /// </summary>
        private void UpdateContent(Point position, object dataContext) {
            this.DataContext = dataContext;
            this.PopupElement.HorizontalOffset = position.X;
            this.PopupElement.VerticalOffset = position.Y;
            this.PopupOpacity = this.hasUserInteracted ? this.maxOpacity : this.defaultOpacity;
        }

        /// <summary>
        /// Внутренний метод показа popup.
        /// Устанавливает DataContext, позицию, делает видимым.
        /// </summary>
        private void InternalShowPopup(Point position, object dataContext) {
            this.isPopupVisible = true;
            this.DataContext = dataContext;
            this.PopupElement.HorizontalOffset = position.X;
            this.PopupElement.VerticalOffset = position.Y;
            this.PopupElement.IsOpen = true;
            this.PopupOpacity = this.hasUserInteracted ? this.maxOpacity : this.defaultOpacity;
        }

        /// <summary>
        /// Внутренний метод скрытия popup и сброса состояния.
        /// </summary>
        private void InternalHidePopup() {
            this.isPopupVisible = false;
            this.PopupElement.IsOpen = false;
            this.hasUserInteracted = false;
            this.PopupOpacity = this.defaultOpacity;
        }

        /// <summary>
        /// Отменяет таймер показа popup, если он активен.
        /// </summary>
        private void CancelShowTimer() {
            if (this.showTimer?.IsEnabled == true) {
                this.showTimer.Stop();
            }
        }

        /// <summary>
        /// Отменяет таймер скрытия popup, если он активен.
        /// </summary>
        private void CancelHideTimer() {
            if (this.hideTimer?.IsEnabled == true) {
                this.hideTimer.Stop();
            }
        }

        /// <summary>
        /// Наведение мыши на сам popup (Border).
        /// Снимает таймер закрытия и делает popup полностью видимым.
        /// </summary>
        private void PopupContent_MouseEnter(object sender, MouseEventArgs e) {
            this.CancelHideTimer();
            this.PopupOpacity = this.maxOpacity;
            this.hasUserInteracted = true;
        }

        /// <summary>
        /// Уход мыши из popup.
        /// Запускает таймер закрытия (если пользователь не взаимодействовал).
        /// </summary>
        private void PopupContent_MouseLeave(object sender, MouseEventArgs e) {
            if (!this.hasUserInteracted) {
                this.PopupOpacity = this.defaultOpacity;
            }

            this.Hide();
        }
    }
}
