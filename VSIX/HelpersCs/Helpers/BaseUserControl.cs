using System;
using System.Linq;
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



    public static class BaseUserControlResourceHelper {
        /// <summary>
        /// Обновляем все найденные словари, даже если ключ дублируется.
        /// В WPF ResourceDictionary создаёт отдельный экземпляр при каждом Source = "...".
        ///
        /// Это означает, что даже если разные контролы подключают один и тот же XAML-файл ресурсов (например, BrushResources.xaml),
        /// они получают отдельные, несвязанные словари.
        ///
        /// Поэтому мы намеренно обновляем все локальные словари, где есть нужный ключ,
        /// чтобы DynamicResource-привязки внутри контролов начали использовать новое значение.
        ///
        /// Это гарантирует синхронизацию внешне идентичных, но фактически независимых ресурсов.
        /// Обновляем все найденные словари, даже если ключ дублируется.
        /// </summary>
        public static void UpdateDynamicResource(DependencyObject root, string key, object value) {
            var dictionaries = FindBaseUserControlDictionariesWithKey(root, key);
            foreach (var dict in dictionaries) {
                dict[key] = value;
            }
        }

        /// <summary>
        /// Находит все словари ресурсов внутри BaseUserControl, которые содержат указанный ключ.
        /// Обход выполняется как вниз по визуальному дереву (VisualTree), так и вверх по логическому дереву (LogicalTree),
        /// начиная с заданного корневого элемента.
        ///
        /// Это необходимо потому, что:
        /// - <see cref="DynamicResource"/> может разрешиться как в локальном словаре, так и в родительском;
        /// - Даже если словари загружают один и тот же XAML-файл, каждый <see cref="ResourceDictionary"/> — отдельный экземпляр;
        /// - Чтобы синхронизировать ресурс по всей иерархии контролов, нужно обновить все словари, где найден ключ.
        ///
        /// Метод гарантирует, что каждый словарь будет возвращён только один раз (без дублирования).
        /// </summary>
        public static List<ResourceDictionary> FindBaseUserControlDictionariesWithKey(DependencyObject root, string key) {
            var result = new List<ResourceDictionary>();
            var visited = new HashSet<ResourceDictionary>();

            // Вниз по дереву
            var queue = new Queue<DependencyObject>();
            queue.Enqueue(root);

            while (queue.Count > 0) {
                var current = queue.Dequeue();

                if (current is Helpers.BaseUserControl uc && uc.Resources.Contains(key)) {
                    if (visited.Add(uc.Resources)) {
                        result.Add(uc.Resources);
                    }
                }

                int childCount = VisualTreeHelper.GetChildrenCount(current);
                for (int i = 0; i < childCount; i++) {
                    queue.Enqueue(VisualTreeHelper.GetChild(current, i));
                }
            }

            // Вверх по дереву (логическому)
            var parent = LogicalTreeHelper.GetParent(root);
            while (parent != null) {
                if (parent is Helpers.BaseUserControl uc && uc.Resources.Contains(key)) {
                    if (visited.Add(uc.Resources)) {
                        result.Add(uc.Resources);
                    }
                }

                parent = LogicalTreeHelper.GetParent(parent);
            }

            return result;
        }

    }
}