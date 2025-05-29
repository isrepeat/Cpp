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


    public class SortedObservableCollection<T> : ObservableCollection<T> {
        public IComparer<T>? Comparer { get; set; } = null;

        public new void Add(T item) {
            if (this.Comparer != null) {
                int index = 0;
                while (index < this.Count && this.Comparer.Compare(this[index], item) < 0) {
                    index++;
                }
                base.Insert(index, item);
            }
            else {
                base.Add(item);
            }
        }

        public new bool Remove(T item) {
            bool removed = base.Remove(item);
            return removed;
        }

        public void AddRange(IEnumerable<T> items) {
            foreach (var item in items) {
                this.Add(item);
            }
        }
    }



    //public interface IRangeModifiableCollection {
    //    void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false);
    //    void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    //    void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    //}

    //public class RangeObservableCollection<T> : ObservableCollection<T>, IRangeModifiableCollection {
    //    public event Action<object, IList<T>, IList<T>, bool>? CollectionChangedExtended;
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



    public interface ISelectableItem {
        bool IsSelected { get; set; }
        void SetSelectedDirectly(bool value);

        IMetadata? Metadata { get; }
    }

    // Мы не можем определить статическое свойство SelectionInterceptor прямо в интерфейсе ISelectableItem,
    // потому что VSIX-проекты используют .NET Framework, который не поддерживает default interface members
    // Чтобы сохранить архитектурную связность и доступ к перехватчику через имя интерфейса мы используем
    // отдельный статический класс ISelectableItemStatics, который служит "псевдо-статическим"
    // контейнером для логики, связанной с интерфейсом.
    public static class ISelectableItemStatics {
        public static Func<ISelectableItem, bool, bool>? SelectionInterceptor { get; set; }
    }


    public abstract class SelectableItemBase : ISelectableItem, INotifyPropertyChanged {
        private bool _isProcessingSelection; // Флаг защиты от рекурсии


        private bool _isSelected;
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
                    bool proposed = ISelectableItemStatics.SelectionInterceptor?.Invoke(this, value) ?? value;

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


        private readonly IMetadata _metadata = new FlaggableMetadata();
        public IMetadata? Metadata => _metadata;


        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }



    public interface ISelectableGroup<TItem> {
        SortedObservableCollection<TItem> Items { get; }
    }



    public class GroupSelectionBinding<TGroup, TItem>
       where TGroup : ISelectableGroup<TItem>
       where TItem : ISelectableItem, INotifyPropertyChanged {

        // Events:
        public event Action? OnGroupStructureChanged;
        public event Action<TGroup, TItem, PropertyChangedEventArgs>? OnGroupItemChanged;

        // Properties:
        private readonly ObservableCollection<TGroup> _groups;
        public IReadOnlyList<TGroup> Groups => _groups;

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
                if (e.NewItems != null) {
                    foreach (TGroup group in e.NewItems) {
                        this.AttachGroup(group);
                    }
                }
                if (e.OldItems != null) {
                    foreach (TGroup group in e.OldItems) {
                        this.DetachGroup(group);
                    }
                }

                this.OnGroupStructureChanged?.Invoke();
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

                this.OnGroupStructureChanged?.Invoke();
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
            get => _selectionState;
            set {
                this.SetPropertyWithNotificationAndGuard(
                    ref _selectionState,
                    value,
                    newVal => this.OnSelectionStateChanged?.Invoke(newVal)
                );
            }
        }

        public (TGroup Group, TItem Item)? ActiveItem {
            get => _anchor;
        }

        // Internal:
        private readonly GroupSelectionBinding<TGroup, TItem> _groupSelectionBinding;
        private readonly HashSet<(TGroup Group, TItem Item)> _selectedItems = new();
        private List<(TGroup Group, TItem Item)> _flatItems = new();
        private (TGroup Group, TItem Item)? _anchor = null;
        
        const string _isActiveFlag = "IsActive";

        public GroupsSelectionCoordinator(ObservableCollection<TGroup> groups) {
            _groupSelectionBinding = new GroupSelectionBinding<TGroup, TItem>(groups);
            _groupSelectionBinding.OnGroupStructureChanged += this.UpdateFlatItems;
            _groupSelectionBinding.OnGroupItemChanged += this.OnGroupItemChanged;

            ISelectableItemStatics.SelectionInterceptor = (item, proposed) => {
                if (item is TItem typed && _groupSelectionBinding.TryGetGroup(typed, out var group)) {
                    return this.InterceptHandler(group, typed, proposed);
                }
                return proposed;
            };

            this.UpdateFlatItems();
        }

        //
        // Handlers
        //
        private bool InterceptHandler(TGroup group, TItem item, bool proposed) {
            var requestedAction = proposed
                ? Enums.SelectionAction.Select
                : Enums.SelectionAction.Unselect;

            // Было ли выбрано текущее выделенное до начала обработки
            bool wasSelectedBefore = item.IsSelected;

            // Снимок: какие другие элементы были выделены (исключая текущий)
            var previouslySelectedOtherItems = _flatItems
                .Where(entry => !ReferenceEquals(entry.Item, item) && entry.Item.IsSelected)
                .ToHashSet();

            var resultAction = this.HandleSelection(group, item, requestedAction);
            var isSelected = resultAction == Enums.SelectionAction.Select;
            if (isSelected) {
                _selectedItems.Add((group, item));
            }
            else {
                _selectedItems.Remove((group, item));
            }

            this.UpdateItemsMetadataFor(_isActiveFlag);

            this.SelectionState = _selectedItems.Count switch {
                0 => Enums.SelectionState.None,
                1 => Enums.SelectionState.Single,
                _ => Enums.SelectionState.Multiple
            };

            // Notify about selection changes.
            var currentlySelectedOtherItems = _flatItems
                .Where(entry => !ReferenceEquals(entry.Item, item) && entry.Item.IsSelected)
                .ToHashSet();

            foreach (var removed in previouslySelectedOtherItems.Except(currentlySelectedOtherItems)) {
                this.OnItemSelectionChanged?.Invoke(removed.Group, removed.Item, false);
            }
            foreach (var added in currentlySelectedOtherItems.Except(previouslySelectedOtherItems)) {
                this.OnItemSelectionChanged?.Invoke(added.Group, added.Item, true);
            }
            if (wasSelectedBefore != isSelected) {
                this.OnItemSelectionChanged?.Invoke(group, item, isSelected);
            }

            return isSelected;
        }

        private Enums.SelectionAction HandleSelection(TGroup group, TItem item, Enums.SelectionAction requestedAction) {
            bool isShift = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
            bool isCtrl = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;

            // Inverted action means do nothing.
            var doNothing = requestedAction == Enums.SelectionAction.Select
                    ? Enums.SelectionAction.Unselect
                    : Enums.SelectionAction.Select;

            if (isShift && isCtrl) { // Such case doesn't supported.
                return doNothing;
            }

            var resultAction = requestedAction;

            if (requestedAction == Enums.SelectionAction.Select) {
                resultAction = Enums.SelectionAction.Select;

                if (isShift) {
                    if (_anchor != null) {
                        this.ApplyShiftRange(_anchor.Value, (group, item));
                    }
                    else {
                        resultAction = doNothing;
                    }
                }
                else {
                    if (isCtrl) {
                    }
                    else {
                        this.ClearAllSelection();
                    }
                    _anchor = (group, item);
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
                        _anchor = (group, item);
                    }
                }
            }

            return resultAction;
        }

        private void OnGroupItemChanged(TGroup group, TItem item, PropertyChangedEventArgs e) {
            if (e.PropertyName != nameof(ISelectableItem.IsSelected)) {
                return;
            }

            // тут ничего не делаем — вся логика в перехватчике
        }


        //
        // Internal logic
        //
        private void ApplyShiftRange((TGroup Group, TItem Item) from, (TGroup Group, TItem Item) to) {
            int i1 = _flatItems.IndexOf(from);
            int i2 = _flatItems.IndexOf(to);
            
            if (i1 == -1 || i2 == -1) {
                return; // Один из элементов не найден — не продолжаем.
            }

            int min = System.Math.Min(i1, i2);
            int max = System.Math.Max(i1, i2);

            _selectedItems.Clear();
            for (int i = 0; i < _flatItems.Count; i++) {
                // Проверяем, входит ли текущий индекс в выделенный диапазон.
                bool inRange = i >= min && i <= max;

                // Устанавливаем выделение напрямую (без перехвата) только тем элементам, которые находятся в диапазоне.
                _flatItems[i].Item.SetSelectedDirectly(inRange);

                // Добавляем элемент в список выбранных, если он входит в диапазон.
                if (inRange) {
                    _selectedItems.Add(_flatItems[i]);
                }
            }

            _anchor = from;
        }

        private void ClearAllSelection() {
            foreach (var (group, item) in _selectedItems.ToList()) {
                item.SetSelectedDirectly(false);
            }
            _selectedItems.Clear();
        }

        private void UpdateItemsMetadataFor(string metadataFlag) {
            if (metadataFlag == _isActiveFlag) {
                foreach (var (group, item) in _flatItems) {
                    item.Metadata?.SetFlag(_isActiveFlag, false);
                }
                if (this.ActiveItem != null) {
                    this.ActiveItem.Value.Item?.Metadata?.SetFlag(_isActiveFlag, true);
                }
            }
        }

        private void UpdateFlatItems() {
            _flatItems = _groupSelectionBinding.Groups
                .SelectMany(g => g.Items.Select(i => (Group: g, Item: i)))
                .ToList();
        }
    }
}