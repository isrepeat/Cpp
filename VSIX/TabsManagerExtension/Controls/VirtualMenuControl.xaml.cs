﻿using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Threading;
using System.Collections.ObjectModel;

namespace TabsManagerExtension.Controls {
    public partial class VirtualMenuControl : Helpers.BaseUserControl {
        public ObservableCollection<Helpers.IMenuItem> VirtualMenuItems {
            get { return (ObservableCollection<Helpers.IMenuItem>)this.GetValue(VirtualMenuItemsProperty); }
            set { this.SetValue(VirtualMenuItemsProperty, value); }
        }
        public static readonly DependencyProperty VirtualMenuItemsProperty =
            DependencyProperty.Register(
                nameof(VirtualMenuItems),
                typeof(ObservableCollection<Helpers.IMenuItem>),
                typeof(VirtualMenuControl),
                new PropertyMetadata(null));


        public ICommand OnVirtualMenuOpenCommand {
            get => (ICommand)this.GetValue(OnVirtualMenuOpenCommandProperty);
            set => this.SetValue(OnVirtualMenuOpenCommandProperty, value);
        }
        public static readonly DependencyProperty OnVirtualMenuOpenCommandProperty =
            DependencyProperty.Register(
                nameof(OnVirtualMenuOpenCommand),
                typeof(ICommand),
                typeof(VirtualMenuControl),
                new PropertyMetadata(null));


        public ICommand OnVirtualMenuClosedCommand {
            get => (ICommand)this.GetValue(OnVirtualMenuClosedCommandProperty);
            set => this.SetValue(OnVirtualMenuClosedCommandProperty, value);
        }
        public static readonly DependencyProperty OnVirtualMenuClosedCommandProperty =
            DependencyProperty.Register(
                nameof(OnVirtualMenuClosedCommand),
                typeof(ICommand),
                typeof(VirtualMenuControl),
                new PropertyMetadata(null));


        public DataTemplateSelector? VirtualMenuItemTemplateSelector {
            get => (DataTemplateSelector?)this.GetValue(VirtualMenuItemTemplateSelectorProperty);
            set => this.SetValue(VirtualMenuItemTemplateSelectorProperty, value);
        }
        public static readonly DependencyProperty VirtualMenuItemTemplateSelectorProperty =
            DependencyProperty.Register(
                nameof(VirtualMenuItemTemplateSelector),
                typeof(DataTemplateSelector),
                typeof(VirtualMenuControl),
                new PropertyMetadata(null));


        public object CurrentMenuDataContext { get; private set; }
        
        private DispatcherTimer showTimer;
        private DispatcherTimer hideTimer;

        private object pendingDataContext;
        private Point pendingPosition;

        private double defaultOpacity = 0.1;
        private double maxOpacity = 1.0;

        private bool hasUserInteracted = false;

        public VirtualMenuControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
            this.Unloaded += this.OnUnloaded;
        }


        private void OnLoaded(object sender, RoutedEventArgs e) {
            this.VirtualMenu.MouseEnteredPopup += PopupContent_MouseEnter;
            this.VirtualMenu.MouseLeftPopup += PopupContent_MouseLeave;
        }
        private void OnUnloaded(object sender, RoutedEventArgs e) {
            this.VirtualMenu.MouseLeftPopup -= PopupContent_MouseLeave;
            this.VirtualMenu.MouseEnteredPopup -= PopupContent_MouseEnter;
        }

        /// <summary>
        /// Публичный вызов показа popup.
        /// Если popup уже открыт — обновляет контент и позицию мгновенно.
        /// Если popup закрыт — запускает таймер, и через 300 мс показывает popup, если пользователь всё ещё на элементе.
        /// </summary>
        public void Show(Point position, object dataContext) {
            this.CurrentMenuDataContext = dataContext;

            this.CancelHideTimer(); // предотвращаем закрытие, если вдруг оно было запущено

            if (this.VirtualMenu.MenuPopup.IsOpen) {
                // Popup уже открыт — просто обновим содержимое
                this.UpdateContent(position, dataContext);
                return;
            }

            // Сохраняем параметры до срабатывания таймера
            this.pendingPosition = position;
            this.pendingDataContext = dataContext;

            if (this.showTimer == null) {
                this.showTimer = new DispatcherTimer(DispatcherPriority.Render);
                this.showTimer.Interval = TimeSpan.FromMilliseconds(700);

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
                this.hideTimer = new DispatcherTimer(DispatcherPriority.Render);
                this.hideTimer.Interval = TimeSpan.FromMilliseconds(700);
                this.hideTimer.Tick += (s, e) => {
                    this.hideTimer.Stop();

                    // Если мышь не в popup — закрываем
                    if (!this.VirtualMenu.IsMouseOver) {
                        this.InternalHidePopup();
                    }
                };
            }

            this.hideTimer.Stop(); // защита от повторного вызова
            this.hideTimer.Start();
        }

        public void HideImmediately() {
            this.CancelShowTimer();
            this.InternalHidePopup();
        }

        /// <summary>
        /// Внутренний метод показа popup.
        /// Устанавливает DataContext, позицию, делает видимым.
        /// </summary>
        private void InternalShowPopup(Point position, object dataContext) {
            this.VirtualMenu.ShowMenu(dataContext, PlacementMode.Absolute, isStaysOpen: true, position);
            this.VirtualMenu._Border.Opacity = this.hasUserInteracted 
                ? this.maxOpacity
                : this.defaultOpacity;
        }

        /// <summary>
        /// Внутренний метод скрытия popup и сброса состояния.
        /// </summary>
        private void InternalHidePopup() {
            this.VirtualMenu.MenuPopup.IsOpen = false;
            this.hasUserInteracted = false;
            this.VirtualMenu._Border.Opacity = this.defaultOpacity;
        }

        /// <summary>
        /// Обновление содержимого и позиции popup без его закрытия.
        /// Вызывается при повторном наведении, когда popup уже показан.
        /// </summary>
        private void UpdateContent(Point position, object dataContext) {
            this.VirtualMenu.UpdateMenu(dataContext, position);
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
        private void PopupContent_MouseEnter(object sender, EventArgs e) {
            this.CancelHideTimer();
            this.hasUserInteracted = true;
            this.VirtualMenu._Border.Opacity = this.maxOpacity;
        }

        /// <summary>
        /// Уход мыши из popup.
        /// Запускает таймер закрытия (если пользователь не взаимодействовал).
        /// </summary>
        private void PopupContent_MouseLeave(object sender, EventArgs e) {
            if (!this.hasUserInteracted) {
                this.VirtualMenu._Border.Opacity = this.defaultOpacity;
            }
            this.Hide();
        }
    }
}