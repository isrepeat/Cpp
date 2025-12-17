using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Helpers.Properties {
    /// <summary>
    /// Обёртка для логического значения, автоматически возвращающая true (или Visible),
    /// пока связанный контрол не пройдёт стадию Loaded.
    /// После Loaded работает как обычное bindable-свойство с поддержкой INotifyPropertyChanged.
    ///
    /// Используется для решения распространённой проблемы WPF: если обычное свойство Visibility
    /// изначально установлено в Collapsed, то связанный элемент <b>не будет добавлен в визуальное дерево</b>,
    /// и его привязки <b>не сработают корректно</b>.Эта обёртка всегда возвращает Visible до Loaded, 
    /// чтобы гарантировать, что элемент хотя бы один раз попадёт в дерево и успеет создать свои привязки.
    /// </summary>
    public class VisibilityProperty : Helpers.ObservableObject, IAutoLoadNotifiable {
        private bool _actualValue = false;

        /// <summary>
        /// Значение, возвращаемое до Loaded всегда как true.
        /// После Loaded — фактическое значение.
        /// </summary>
        public bool Value {
            get {
                return !_isLoaded ? true : _actualValue;
            }
            set {
                if (_actualValue != value) {
                    _actualValue = value;
                    if (_isLoaded) {
                        OnPropertyChanged(nameof(this.Value));
                        OnPropertyChanged(nameof(this.Visibility));
                    }
                }
            }
        }

        /// <summary>
        /// Преобразование логического значения в Visibility.
        /// </summary>
        public Visibility Visibility => this.Value ? Visibility.Visible : Visibility.Collapsed;

        private bool _isLoaded = false;

        public VisibilityProperty() {
            // Регистрируемся в глобальном наблюдателе при создании
            ControlInitializationObserver.Instance.RegisterForOwnerInit(this);
        }

        /// <summary>
        /// Вызывается, когда связанный контрол загрузился.
        /// Переключает обёртку в "реальный" режим.
        /// </summary>
        public void NotifyLoaded() {
            _isLoaded = true;
            OnPropertyChanged(nameof(this.Value));
            OnPropertyChanged(nameof(this.Visibility));
        }
    }
}