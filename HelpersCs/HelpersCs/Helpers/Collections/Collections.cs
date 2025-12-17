using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;


namespace Helpers.Collections {
    public enum ItemPosition {
        Top = 0,
        Middle = 100,
        Bottom = 200
    }

    public enum ItemInsertMode {
        Default,                    // обычное поведение: любое количество элементов
        Single,                     // разрешён только один элемент — остальные игнорируются
        SingleWithReplaceExisting   // разрешён только один — при добавлении предыдущий удаляется
    }

    /// <summary>
    /// Представляет группу элементов с определённым приоритетом и сортировкой внутри группы.
    /// </summary>
    public class PriorityGroup<T> {
        public ItemPosition Position {
            get => (ItemPosition)this.Priority;
            set => this.Priority = (int)value;
        }
        public ItemInsertMode InsertMode { get; set; }
        public Func<T, bool> Predicate { get; set; }
        public IComparer<T> Comparer { get; set; }

        internal int Priority { get; private set; }

        public PriorityGroup() {
            this.Position = ItemPosition.Middle;
            this.InsertMode = ItemInsertMode.Default;
            this.Predicate = _ => false;
            this.Comparer = Comparer<T>.Default;
        }
    }

    /// <summary>
    /// Расширенная ObservableCollection, поддерживающая приоритетные группы и сортировку при вставке.
    /// </summary>
    public class SortedObservableCollection<T> : ObservableCollection<T> {
        private PriorityGroup<T> _defaultPriorityGroup;
        private List<PriorityGroup<T>> _priorityGroups;

        public SortedObservableCollection()
            : this(Comparer<T>.Default, Enumerable.Empty<PriorityGroup<T>>()) {
        }
        public SortedObservableCollection(IComparer<T> defaultComparer)
            : this(defaultComparer, Enumerable.Empty<PriorityGroup<T>>()) {
        }
        public SortedObservableCollection(
            IComparer<T> defaultComparer,
            IEnumerable<PriorityGroup<T>> priorityGroups) {

            _defaultPriorityGroup = new PriorityGroup<T> {
                Predicate = _ => true,
                Comparer = defaultComparer,
                Position = ItemPosition.Middle
            };
            
            _priorityGroups = priorityGroups?.ToList() ?? throw new ArgumentNullException(nameof(priorityGroups));
        }


        /// <summary>
        /// Добавляет элемент в коллекцию с учётом приоритетной группы и сортировки.
        /// </summary>
        public new void Add(T item) {
            var itemGroup = this.GetPriorityGroup(item);

            switch (itemGroup.InsertMode) {
                case ItemInsertMode.Single:
                case ItemInsertMode.SingleWithReplaceExisting:
                    var existing = this.FirstOrDefault(existingItem => itemGroup.Predicate(existingItem));

                    if (existing != null) {
                        if (itemGroup.InsertMode == ItemInsertMode.SingleWithReplaceExisting) {
                            base.Remove(existing); // заменяем
                        }
                        else {
                            return; // игнорируем вставку
                        }
                    }
                    break;
            }

            int index = this.GetInsertIndex(item);
            base.Insert(index, item);
        }

        [Obsolete("Direct Insert is not supported in SortedObservableCollection. Use Add() instead.", true)]
        public new void Insert(int index, T item) {
            throw new InvalidOperationException("Use of Insert is not allowed. Use Add() for sorted insertion.");
        }

        /// <summary>
        /// Удаляет элемент из коллекции.
        /// </summary>
        public new bool Remove(T item) {
            bool removed = base.Remove(item);
            return removed;
        }

        /// <summary>
        /// Добавляет диапазон элементов в коллекцию с правильной сортировкой каждого.
        /// </summary>
        public void AddRange(IEnumerable<T> items) {
            foreach (var item in items) {
                this.Add(item);
            }
        }

        /// <summary>
        /// Вычисляет индекс, в который должен быть вставлен элемент,
        /// основываясь на приоритетной группе и сортировке внутри неё.
        /// </summary>
        private int GetInsertIndex(T item) {
            var itemGroup = this.GetPriorityGroup(item);

            for (int i = 0; i < this.Count; i++) {
                var current = this[i];
                var currentGroup = this.GetPriorityGroup(current);

                // Сравнение приоритетов групп
                if (itemGroup.Priority < currentGroup.Priority) {
                    return i;
                }

                if (itemGroup.Priority == currentGroup.Priority) {
                    var comparer = itemGroup.Comparer ?? _defaultPriorityGroup.Comparer;

                    // Вставка до текущего, если элемент "меньше"
                    if (comparer.Compare(item, current) < 0) {
                        return i;
                    }
                }
            }

            // Вставка в конец
            return this.Count;
        }

        /// <summary>
        /// Определяет, к какой приоритетной группе принадлежит элемент.
        /// Если ни к одной — возвращается "дефолтная" группа с максимальным приоритетом.
        /// </summary>
        private PriorityGroup<T> GetPriorityGroup(T item) {
            foreach (var group in _priorityGroups.OrderBy(g => g.Priority)) {
                if (group.Predicate(item)) {
                    return group;
                }
            }

            return _defaultPriorityGroup;
        }
    }



    //public interface IRangeModifiableCollection {
    //    void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false);
    //    void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    //    void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    //}

    //public class RangeObservableCollection<T> : ObservableCollection<T>, IRangeModifiableCollection {
    //    public event System.Action<object, IList<T>, IList<T>, bool>? CollectionChangedExtended;
    //    public IComparer<T>? Comparer { get; set; } = null;

    //    private bool _suppressNotification = false;

    //    public RangeObservableCollection() : base() { }

    //    public RangeObservableCollection(IEnumerable<T> collection) : base(collection) { }


    //    public void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false) {
    //        this.AddRemoveRange(toAddItems, Enumerable.Empty<object>(), multiselectionMode);
    //    }

    //    public void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false) {
    //        this.AddRemoveRange(Enumerable.Empty<object>(), toRemoveItems, multiselectionMode);
    //    }

    //    public void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false) {
    //        var addedItems = toAddItems?.OfType<T>().ToList() ?? new();
    //        var removedItems = toRemoveItems?.OfType<T>().ToList() ?? new();

    //        _suppressNotification = true;
    //        // NOTE: Because we use sorting, we must first remove items before inserting new ones.
    //        //       Inserting before removing could cause duplicates or incorrect positions due to existing items affecting sort order.
    //        foreach (var item in removedItems) {
    //            base.Remove(item);
    //        }
    //        foreach (var item in addedItems) {
    //            if (Comparer != null) {
    //                int index = 0;
    //                while (index < Count && Comparer.Compare(this[index], item) < 0) {
    //                    index++;
    //                }
    //                base.Insert(index, item);
    //            }
    //            else {
    //                base.Add(item);
    //            }
    //        }
    //        _suppressNotification = false;

    //        // Для UI уведомлений
    //        if (addedItems.Count > 0) {
    //            base.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, addedItems));
    //        }
    //        if (removedItems.Count > 0) {
    //            base.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, removedItems));
    //        }

    //        // Для внутренней логики
    //        if (addedItems.Count > 0 || removedItems.Count > 0) {
    //            CollectionChangedExtended?.Invoke(this, addedItems, removedItems, multiselectionMode);
    //        }
    //    }

    //    protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e) {
    //        if (!_suppressNotification) {
    //            base.OnCollectionChanged(e);
    //        }
    //    }
    //}
}