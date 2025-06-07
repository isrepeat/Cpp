using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Helpers {
    public interface IAutoLoadNotifiable {
        void NotifyLoaded();
    }

    /// <summary>
    /// Singleton-класс, отслеживающий инициализацию пользовательских контролов (Loaded).
    /// Позволяет обёрткам типа BoolVisibilityWrapper автоматически "подцепиться" к контролу,
    /// даже если они созданы до вызова конструктора.
    /// </summary>
    public class ControlInitializationObserver {
        public static readonly ControlInitializationObserver Instance = new();

        // Временный буфер для обёрток, созданных до того как доступен their FrameworkElement (this)
        private readonly List<Helpers.IAutoLoadNotifiable> _globalUnattached = new();

        private ControlInitializationObserver() { }

        /// <summary>
        /// Вызывается из конструктора обёртки. Добавляет в глобальный список для последующего подключения.
        /// </summary>
        public void RegisterForOwnerInit(IAutoLoadNotifiable item) {
            _globalUnattached.Add(item);
        }

        /// <summary>
        /// Вызывается в BaseUserControl. Подключает все ранее зарегистрированные обёртки к конкретному контролу.
        /// </summary>
        public void AttachAllPendingTo(FrameworkElement owner) {
            foreach (var item in _globalUnattached) {
                this.Observe(owner, () => item.NotifyLoaded());
            }
            _globalUnattached.Clear();
        }

        /// <summary>
        /// Выполняет колбэк сразу или при Loaded, если элемент ещё не загружен.
        /// </summary>
        public void Observe(FrameworkElement owner, Action callback) {
            if (owner.IsLoaded) {
                callback();
                return;
            }

            owner.Loaded += (_, _) => callback();
        }
    }



    /// <summary>
    /// Базовый пользовательский контрол, предоставляющий:
    /// - RootControl (сам себя)
    /// - HostControl (контейнер-контрол извне)
    /// - интеграцию с ControlInitializationObserver для BoolVisibilityWrapper
    /// </summary>
    public class BaseUserControl : UserControl, INotifyPropertyChanged {
        /// <summary>
        /// Сам контрол — полезно при вложенных шаблонах и передаче в XAML.
        /// </summary>
        public object? RootControl {
            get => (object?)GetValue(RootControlProperty);
            private set => SetValue(RootControlProperty, value);
        }

        public static readonly DependencyProperty RootControlProperty =
            DependencyProperty.Register(
                nameof(RootControl),
                typeof(object),
                typeof(BaseUserControl),
                new PropertyMetadata(null));

        /// <summary>
        /// Контейнер, в который вложен данный контрол (например, TabsManagerToolWindowControl).
        /// Может быть задан извне.
        /// </summary>
        public object? HostControl {
            get => (object?)GetValue(HostControlProperty);
            set => SetValue(HostControlProperty, value);
        }

        public static readonly DependencyProperty HostControlProperty =
            DependencyProperty.Register(
                nameof(HostControl),
                typeof(object),
                typeof(BaseUserControl),
                new PropertyMetadata(null));

        private readonly List<Action> _onLoadedCallbacks = new();
        private bool _loadedHandled = false;

        public BaseUserControl() {
            // RootControl всегда указывает на самого себя
            this.RootControl = this;

            // Автоматически подключаем все зарегистрированные обёртки к этому контролу
            ControlInitializationObserver.Instance.AttachAllPendingTo(this);
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}