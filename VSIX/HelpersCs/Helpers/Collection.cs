using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    namespace Enums {
        public enum SelectionState {
            None,
            Single,
            Multiple
        }

        public enum SelectionAction {
            Select,
            Unselect
        }
    }


    public interface IRangeModifiableCollection {
        void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false);
        void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
        void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    }

    public class RangeObservableCollection<T> : ObservableCollection<T>, IRangeModifiableCollection {
        public event Action<object, IList<T>, IList<T>, bool>? CollectionChangedExtended;
        public IComparer<T>? Comparer { get; set; } = null;

        private bool _suppressNotification = false;

        public RangeObservableCollection() : base() { }

        public RangeObservableCollection(IEnumerable<T> collection) : base(collection) { }


        public void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false) {
            this.AddRemoveRange(toAddItems, Enumerable.Empty<object>(), multiselectionMode);
        }

        public void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false) {
            this.AddRemoveRange(Enumerable.Empty<object>(), toRemoveItems, multiselectionMode);
        }

        public void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false) {
            var addedItems = toAddItems?.OfType<T>().ToList() ?? new();
            var removedItems = toRemoveItems?.OfType<T>().ToList() ?? new();

            _suppressNotification = true;
            // NOTE: Because we use sorting, we must first remove items before inserting new ones.
            //       Inserting before removing could cause duplicates or incorrect positions due to existing items affecting sort order.
            foreach (var item in removedItems) {
                base.Remove(item);
            }
            foreach (var item in addedItems) {
                if (Comparer != null) {
                    int index = 0;
                    while (index < Count && Comparer.Compare(this[index], item) < 0) {
                        index++;
                    }
                    base.Insert(index, item);
                }
                else {
                    base.Add(item);
                }
            }
            _suppressNotification = false;

            // Для UI уведомлений
            if (addedItems.Count > 0) {
                base.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, addedItems));
            }
            if (removedItems.Count > 0) {
                base.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, removedItems));
            }

            // Для внутренней логики
            if (addedItems.Count > 0 || removedItems.Count > 0) {
                CollectionChangedExtended?.Invoke(this, addedItems, removedItems, multiselectionMode);
            }
        }

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e) {
            if (!_suppressNotification) {
                base.OnCollectionChanged(e);
            }
        }
    }



    public interface ISelectableItem {
        bool IsSelected { get; set; }
        void SetSelectedDirectly(bool value);

        static Func<ISelectableItem, bool, bool>? SelectionInterceptor { get; set; }
    }

    public abstract class SelectableItemBase : ISelectableItem, INotifyPropertyChanged {
        private bool _isSelected;

        // Флаг защиты от рекурсии
        private bool _isProcessingSelection;

        public bool IsSelected {
            get => _isSelected;
            set {
                if (_isProcessingSelection) {
                    Helpers.Diagnostic.Logger.LogWarning("You call setter from SelectableItem.SelectionInterceptor");
                    System.Diagnostics.Debugger.Break();
                    return;
                }

                _isProcessingSelection = true;
                try {
                    bool proposed = ISelectableItem.SelectionInterceptor?.Invoke(this, value) ?? value;

                    if (_isSelected != proposed) {
                        _isSelected = proposed;
                        OnPropertyChanged();
                    }
                }
                finally {
                    _isProcessingSelection = false;
                }
            }
        }

        // Для прямого обновления без перехвата
        public void SetSelectedDirectly(bool value) {
            if (_isSelected != value) {
                _isSelected = value;
                OnPropertyChanged(nameof(IsSelected));
            }
        }


        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }



    public interface ISelectableGroup<TItem> {
        RangeObservableCollection<TItem> Items { get; }
    }



    public class GroupSelectionBinding<TGroup, TItem>
       where TGroup : ISelectableGroup<TItem>
       where TItem : ISelectableItem, INotifyPropertyChanged {

        // Events:
        public event Action<TGroup, TItem, PropertyChangedEventArgs>? OnGroupItemChanged;

        // Properties:
        private readonly ObservableCollection<TGroup> _groups;
        public IReadOnlyList<TGroup> Groups => this._groups;

        // Internal:
        private readonly Dictionary<TGroup, NotifyCollectionChangedEventHandler> _collectionChangedHandlers = new();
        private readonly Dictionary<TItem, PropertyChangedEventHandler> _itemHandlers = new();
        private readonly Dictionary<TItem, TGroup> _itemToGroupMap = new();

        public GroupSelectionBinding(ObservableCollection<TGroup> groups) {
            _groups = groups;

            foreach (var group in _groups) {
                this.AttachGroup(group);
            }

            _groups.CollectionChanged += (_, e) => {
                if (e.NewItems is IEnumerable<TGroup> addedGroups) {
                    foreach (var group in addedGroups) {
                        this.AttachGroup(group);
                    }
                }
                if (e.OldItems is IEnumerable<TGroup> removedGroups) {
                    foreach (var group in removedGroups) {
                        this.DetachGroup(group);
                    }
                }
            };
        }

        public bool TryGetGroup(TItem item, out TGroup group) {
            return _itemToGroupMap.TryGetValue(item, out group);
        }

        private void AttachGroup(TGroup group) {
            foreach (var item in group.Items) {
                _itemToGroupMap[item] = group;

                var handler = new PropertyChangedEventHandler((_, e) => this.OnGroupItemChangedHandler(group, item, e));
                _itemHandlers[item] = handler;
                item.PropertyChanged += handler;
            }

            NotifyCollectionChangedEventHandler collectionHandler = (s, e) => {
                if (e.NewItems != null) {
                    foreach (var item in e.NewItems.Cast<TItem>()) {
                        _itemToGroupMap[item] = group;

                        var handler = new PropertyChangedEventHandler((_, ev) => this.OnGroupItemChangedHandler(group, item, ev));
                        _itemHandlers[item] = handler;
                        item.PropertyChanged += handler;
                    }
                }

                if (e.OldItems != null) {
                    foreach (var item in e.OldItems.Cast<TItem>()) {
                        _itemToGroupMap.Remove(item);

                        if (_itemHandlers.TryGetValue(item, out var handler)) {
                            item.PropertyChanged -= handler;
                            _itemHandlers.Remove(item);
                        }
                    }
                }
            };

            _collectionChangedHandlers[group] = collectionHandler;
            group.Items.CollectionChanged += collectionHandler;
        }

        private void DetachGroup(TGroup group) {
            if (_collectionChangedHandlers.TryGetValue(group, out var collectionHandler)) {
                group.Items.CollectionChanged -= collectionHandler;
                _collectionChangedHandlers.Remove(group);
            }

            foreach (var item in group.Items) {
                _itemToGroupMap.Remove(item);

                if (_itemHandlers.TryGetValue(item, out var handler)) {
                    item.PropertyChanged -= handler;
                    _itemHandlers.Remove(item);
                }
            }
        }

        private void OnGroupItemChangedHandler(TGroup group, TItem item, PropertyChangedEventArgs e) {
            this.OnGroupItemChanged?.Invoke(group, item, e);
        }
    }



    public class GroupsSelectionCoordinator<TGroup, TItem> : Helpers.ObservableObject
        where TGroup : ISelectableGroup<TItem>
        where TItem : ISelectableItem, INotifyPropertyChanged {

        // Events:
        public Action<TGroup, TItem, bool>? OnItemSelectionChanged;
        public Action<Enums.SelectionState>? OnSelectionStateChanged;

        // Properties:
        private Enums.SelectionState _selectionState = Enums.SelectionState.None;
        public Enums.SelectionState SelectionState {
            get => this._selectionState;
            set {
                this.SetPropertyWithNotificationAndGuard(
                    ref this._selectionState,
                    value,
                    newVal => this.OnSelectionStateChanged?.Invoke(newVal)
                );
            }
        }

        // Internal:
        private readonly GroupSelectionBinding<TGroup, TItem> _groupSelectionBinding;
        private readonly HashSet<(TGroup Group, TItem Item)> _selectedItems = new();
        private (TGroup Group, TItem Item)? _anchor = null;

        public GroupsSelectionCoordinator(ObservableCollection<TGroup> groups) {
            _groupSelectionBinding = new GroupSelectionBinding<TGroup, TItem>(groups);
            _groupSelectionBinding.OnGroupItemChanged += this.OnGroupItemChanged;

            ISelectableItem.SelectionInterceptor = (item, proposed) => {
                if (item is TItem typed && _groupSelectionBinding.TryGetGroup(typed, out var group)) {
                    return this.InterceptHandler(group, typed, proposed);
                }
                return proposed;
            };
        }

        private bool InterceptHandler(TGroup group, TItem item, bool proposed) {
            var requestedAction = proposed
                ? Enums.SelectionAction.Select
                : Enums.SelectionAction.Unselect;

            var resultAction = this.HandleSelection(group, item, requestedAction);
            var isSelected = resultAction == Enums.SelectionAction.Select;

            if (isSelected) {
                _selectedItems.Add((group, item));
            }
            else {
                _selectedItems.Remove((group, item));
            }

            this.SelectionState = _selectedItems.Count switch {
                0 => Enums.SelectionState.None,
                1 => Enums.SelectionState.Single,
                _ => Enums.SelectionState.Multiple
            };

            this.OnItemSelectionChanged?.Invoke(group, item, isSelected);
            return isSelected;
        }

        public Enums.SelectionAction HandleSelection(TGroup group, TItem item, Enums.SelectionAction requestedAction) {
            bool isShift = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
            bool isCtrl = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;

            if (isShift && isCtrl) {
                // Such case doesn't supported. Return inverted action (it's mean do nothing).
                return requestedAction == Enums.SelectionAction.Select
                    ? Enums.SelectionAction.Unselect
                    : Enums.SelectionAction.Select;
            }

            var resultAction = requestedAction;

            if (requestedAction == Enums.SelectionAction.Select) {
                resultAction = Enums.SelectionAction.Select;

                if (isShift) {
                    if (_anchor != null) {
                        this.ApplyShiftRange(_anchor.Value, (group, item));
                    }
                }
                else {
                    _anchor = (group, item);

                    if (isCtrl) {
                    }
                    else {
                        this.ClearAllSelection();
                    }
                }
            }
            else if (requestedAction == Enums.SelectionAction.Unselect) {
                resultAction = Enums.SelectionAction.Select; // Unselect allowable only for CTRL

                if (isShift) {
                    if (_anchor != null) {
                        this.ApplyShiftRange(_anchor.Value, (group, item));
                    }
                }
                else {
                    if (isCtrl) {
                        if (_selectedItems.Count > 1) {
                            resultAction = Enums.SelectionAction.Unselect;
                        }
                    }
                    else {
                        this.ClearAllSelection();
                        resultAction = Enums.SelectionAction.Select;
                    }
                }
            }

            return resultAction;
        }

        private void ApplyShiftRange((TGroup Group, TItem Item) from, (TGroup Group, TItem Item) to) {
            // Преобразуем все элементы всех групп в линейный список.
            var flat = _groupSelectionBinding.Groups
                .SelectMany(g => g.Items.Select(i => (Group: g, Item: i)))
                .ToList();

            int i1 = flat.IndexOf(from);
            int i2 = flat.IndexOf(to);
            if (i1 == -1 || i2 == -1) {
                return; // Один из элементов не найден — не продолжаем.
            }

            int min = System.Math.Min(i1, i2);
            int max = System.Math.Max(i1, i2);

            _selectedItems.Clear();
            for (int i = 0; i < flat.Count; i++) {
                // Проверяем, входит ли текущий индекс в выделенный диапазон.
                bool inRange = i >= min && i <= max;

                // Устанавливаем выделение напрямую (без перехвата) только тем элементам, которые находятся в диапазоне.
                flat[i].Item.SetSelectedDirectly(inRange);

                // Добавляем элемент в список выбранных, если он входит в диапазон.
                if (inRange) {
                    _selectedItems.Add(flat[i]);
                }
            }
            _anchor = from;
        }


        public void ClearAllSelection() {
            foreach (var (group, item) in _selectedItems.ToList()) {
                item.SetSelectedDirectly(false);
            }
            _selectedItems.Clear();
        }


        private void OnGroupItemChanged(TGroup group, TItem item, PropertyChangedEventArgs e) {
            if (e.PropertyName != nameof(ISelectableItem.IsSelected)) {
                return;
            }
            // тут ничего не делаем — вся логика в перехватчике
        }
    }
}