using System.Windows;
using System.Windows.Media;

namespace Helpers {
    /// <summary>
    /// Универсальный Adorner, позволяющий наложить любой UIElement поверх целевого элемента.
    /// Используется для визуальных наложений (рамка, подсказка, индикатор и т.п.), без вмешательства в Visual Tree.
    /// </summary>
    public class AdornerWithChild<T> : System.Windows.Documents.Adorner where T : UIElement {
        private readonly T _child;
        /// <summary>
        /// Получает UIElement, который был добавлен в качестве оверлея.
        /// </summary>
        public T Child => _child;

        /// <summary>
        /// Создаёт наложение с указанным элементом поверх заданного элемента.
        /// </summary>
        /// <param name="adornedElement">Элемент, поверх которого будет размещён оверлей.</param>
        /// <param name="childElement">UIElement, который будет наложен.</param>
        public AdornerWithChild(UIElement adornedElement, T childElement) : base(adornedElement) {
            _child = childElement;
            _child.IsHitTestVisible = false;
            this.AddVisualChild(_child);

            // Обновляем позиционирование при изменении размера
            if (adornedElement is FrameworkElement fe) {
                fe.SizeChanged += (_, _) => this.InvalidateMeasure();
            }
        }

        /// <summary>
        /// Указывает количество визуальных дочерних элементов у адорнера. В нашем случае — всегда один.
        /// </summary>
        protected override int VisualChildrenCount => 1;

        /// <summary>
        /// Возвращает визуальный дочерний элемент по индексу. Используется системой WPF при отрисовке.
        /// </summary>
        /// <param name="index">Индекс визуального потомка (всегда 0, так как потомок один).</param>
        /// <returns>Дочерний UIElement, наложенный поверх.</returns>
        protected override Visual GetVisualChild(int index) {
            return _child;
        }

        /// <summary>
        /// Вызывает измерение размера дочернего элемента, чтобы он соответствовал заданному ограничению.
        /// </summary>
        /// <param name="constraint">Максимально допустимый размер, который может занять дочерний элемент.</param>
        /// <returns>Фактически занимаемый размер (возвращается как есть).</returns>
        protected override Size MeasureOverride(Size constraint) {
            _child.Measure(constraint);
            return constraint;
        }

        /// <summary>
        /// Размещает дочерний элемент в пределах полного доступного размера адорнера.
        /// </summary>
        /// <param name="finalSize">Итоговый прямоугольник, в который нужно вписать содержимое.</param>
        /// <returns>Размер, который будет использоваться для отрисовки.</returns>
        protected override Size ArrangeOverride(Size finalSize) {
            _child.Arrange(new Rect(finalSize));
            return finalSize;
        }
    }



    /// <summary>
    /// Вспомогательный сервис для управления наложениями поверх UIElement через AdornerLayer.
    /// Позволяет добавить и удалить AdornerWithChild с произвольным содержимым.
    /// </summary>
    public static class AdornerService {
        /// <summary>
        /// Добавляет произвольный UIElement в качестве наложения поверх целевого элемента.
        /// </summary>
        /// <typeparam name="T">Тип UIElement для наложения.</typeparam>
        /// <param name="target">Целевой элемент, поверх которого добавляется наложение.</param>
        /// <param name="element">UIElement, который нужно отобразить поверх.</param>
        /// <returns>Созданный Adorner или null, если слой не найден.</returns>
        public static AdornerWithChild<T>? AddAdorner<T>(UIElement target, T element) where T : UIElement {
            var layer = System.Windows.Documents.AdornerLayer.GetAdornerLayer(target);
            if (layer == null) {
                return null;
            }

            var adorner = new AdornerWithChild<T>(target, element);
            layer.Add(adorner);
            return adorner;
        }

        /// <summary>
        /// Удаляет все AdornerWithChild заданного типа, наложенные на целевой элемент.
        /// </summary>
        /// <typeparam name="T">Тип UIElement, который был добавлен в качестве оверлея.</typeparam>
        /// <param name="target">Целевой элемент, с которого нужно удалить наложение.</param>
        public static void RemoveAdorners<T>(UIElement target) where T : UIElement {
            var layer = System.Windows.Documents.AdornerLayer.GetAdornerLayer(target);
            if (layer == null) {
                return;
            }

            var adorners = layer.GetAdorners(target);
            if (adorners == null) {
                return;
            }

            foreach (var adorner in adorners) {
                if (adorner is AdornerWithChild<T>) {
                    layer.Remove(adorner);
                }
            }
        }
    }



    /// <summary>
    /// Управляет наложением оверлея поверх FrameworkElement через AdornerLayer.
    /// Синхронизирует размеры и автоматически удаляет оверлей при выгрузке.
    /// </summary>
    public class AdornerOverlayManager<T> where T : FrameworkElement {
        public bool IsAttached { get; private set; } = false;
        public T? Overlay {
            get {
                _overlayRef.TryGetTarget(out var overlay);
                return overlay;
            }
        }

        private readonly System.WeakReference<FrameworkElement> _targetRef;
        private readonly System.WeakReference<T> _overlayRef;
        private AdornerWithChild<T>? _adorner;

        /// <summary>
        /// Создаёт менеджер наложения оверлея и автоматически добавляет его к AdornerLayer.
        /// </summary>
        public AdornerOverlayManager(FrameworkElement target, T overlay) {
            if (target == null) {
                throw new System.ArgumentNullException(nameof(target));
            }

            if (overlay == null) {
                throw new System.ArgumentNullException(nameof(overlay));
            }

            _targetRef = new System.WeakReference<FrameworkElement>(target);
            _overlayRef = new System.WeakReference<T>(overlay);

            // Установка начального размера
            overlay.Width = target.ActualWidth;
            overlay.Height = target.ActualHeight;

            // Подписка на изменение размеров и выгрузку
            target.SizeChanged += OnTargetSizeChanged;
            target.Unloaded += OnTargetUnloaded;

            // Добавление в слой наложений
            _adorner = AdornerService.AddAdorner(target, overlay);
            this.IsAttached = true;
        }

        /// <summary>
        /// Обрабатывает изменение размеров таргета, обновляя размеры оверлея.
        /// Если один из элементов недоступен — вызывает очистку.
        /// </summary>
        private void OnTargetSizeChanged(object? sender, SizeChangedEventArgs e) {
            if (!_targetRef.TryGetTarget(out var target) ||
                !_overlayRef.TryGetTarget(out var overlay)) {
                Cleanup();
                return;
            }

            overlay.Width = target.ActualWidth;
            overlay.Height = target.ActualHeight;
        }

        /// <summary>
        /// Обрабатывает событие выгрузки целевого элемента — вызывает удаление оверлея.
        /// </summary>
        private void OnTargetUnloaded(object? sender, RoutedEventArgs e) {
            this.IsAttached = false;
            Cleanup();
        }

        /// <summary>
        /// Удаляет оверлей и отписывается от событий.
        /// </summary>
        private void Cleanup() {
            if (_targetRef.TryGetTarget(out var target)) {
                target.SizeChanged -= OnTargetSizeChanged;
                target.Unloaded -= OnTargetUnloaded;
                AdornerService.RemoveAdorners<T>(target);
            }

            _adorner = null;
        }

        /// <summary>
        /// Принудительное удаление оверлея.
        /// </summary>
        public void Remove() {
            Cleanup();
        }
    }
}