#define ENABLE_VERBOSE_LOGGING
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace WpfTestApp {
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

        // Глобальный перехватчик установки значения
        static Func<ISelectableItem, bool, bool>? SelectionInterceptor { get; set; }
    }

    public abstract class SelectableItemBase : ISelectableItem, INotifyPropertyChanged {
        private bool _isSelected;

        // 🔒 Флаг защиты от рекурсии
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




    public class MyItem : SelectableItemBase {
        public string Name { get; }

        public MyItem(string name) {
            Name = name;
        }

        public override string ToString() => Name;
    }


    public class GroupModel : ISelectableGroup<MyItem> {
        public string Name { get; }
        public RangeObservableCollection<MyItem> Items { get; } = new();

        public GroupModel(string name, IEnumerable<MyItem> items) {
            Name = name;
            Items.Comparer = Comparer<MyItem>.Create((a, b) => string.Compare(a.Name, b.Name, StringComparison.OrdinalIgnoreCase));
            Items.AddRange(items.Cast<object>());
        }
    }





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
                if (_selectionState != value) {
                    _selectionState = value;
                    OnPropertyChanged();
                }
            }
        }

        // Internal:
        private readonly ObservableCollection<TGroup> _groups;
        private readonly Dictionary<TGroup, NotifyCollectionChangedEventHandler> _collectionChangedHandlers = new();
        private readonly Dictionary<TItem, PropertyChangedEventHandler> _itemHandlers = new();
        private readonly Dictionary<TItem, TGroup> _itemToGroupMap = new();
        private readonly HashSet<(TGroup Group, TItem Item)> _selectedItems = new();
        private (TGroup Group, TItem Item)? _anchor = null;

        public GroupsSelectionCoordinator(ObservableCollection<TGroup> groups) {
            _groups = groups;

            // Централизованный перехват выбора
            ISelectableItem.SelectionInterceptor = (item, proposed) => {
                if (item is TItem typed && _itemToGroupMap.TryGetValue(typed, out var group)) {
                    var requestedAction = proposed
                        ? Enums.SelectionAction.Select
                        : Enums.SelectionAction.Unselect;

                    var resultAction = this.HandleSelection(group, typed, requestedAction);
                    var isSelected = resultAction == Enums.SelectionAction.Select;

                    if (isSelected) {
                        _selectedItems.Add((group, typed));
                    }
                    else {
                        _selectedItems.Remove((group, typed));
                    }

                    var oldSelectionState = this.SelectionState;
                    this.SelectionState = _selectedItems.Count switch {
                        0 => Enums.SelectionState.None,
                        1 => Enums.SelectionState.Single,
                        _ => Enums.SelectionState.Multiple
                    };

                    if (this.SelectionState != oldSelectionState) {
                        OnSelectionStateChanged?.Invoke(this.SelectionState);
                    }

                    this.OnItemSelectionChanged?.Invoke(group, typed, isSelected);
                    return isSelected;
                }
                return proposed;
            };

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

        // TODO: add comments
        private void ApplyShiftRange((TGroup Group, TItem Item) from, (TGroup Group, TItem Item) to) {
            var flat = _groups.SelectMany(g => g.Items.Select(i => (Group: g, Item: i))).ToList();

            int i1 = flat.IndexOf(from);
            int i2 = flat.IndexOf(to);
            if (i1 == -1 || i2 == -1) {
                return;
            }

            int min = Math.Min(i1, i2);
            int max = Math.Max(i1, i2);

            _selectedItems.Clear();

            for (int i = 0; i < flat.Count; i++) {
                bool inRange = i >= min && i <= max;
                flat[i].Item.SetSelectedDirectly(inRange);

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



        private void AttachGroup(TGroup group) {
            foreach (var item in group.Items) {
                _itemToGroupMap[item] = group;

                var handler = new PropertyChangedEventHandler((_, e) => this.OnItemChanged(group, item, e));
                _itemHandlers[item] = handler;
                item.PropertyChanged += handler;
            }

            NotifyCollectionChangedEventHandler collectionHandler = (s, e) => {
                if (e.NewItems != null) {
                    foreach (var item in e.NewItems.Cast<TItem>()) {
                        _itemToGroupMap[item] = group;

                        var handler = new PropertyChangedEventHandler((_, ev) => this.OnItemChanged(group, item, ev));
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

        private void OnItemChanged(TGroup group, TItem item, PropertyChangedEventArgs e) {
            if (e.PropertyName != nameof(ISelectableItem.IsSelected))
                return;

            // тут ничего не делаем — вся логика в перехватчике
        }
    }




    public partial class MainWindow_SelectionViewModel : Window, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ObservableCollection<GroupModel> Groups { get; }

        private GroupsSelectionCoordinator<GroupModel, MyItem> _groupsSelectionCoordinator;
        public GroupsSelectionCoordinator<GroupModel, MyItem> GroupsSelectionCoordinator {
            get => _groupsSelectionCoordinator;
        }

        public MainWindow_SelectionViewModel() {
            this.InitializeComponent();

            Groups = new ObservableCollection<GroupModel> {
                new ("Group A", new[] { new MyItem("A3"), new MyItem("A2"), new MyItem("A1"), new MyItem("A4") }),
                new ("Group B", new[] { new MyItem("B1"), new MyItem("B2"), new MyItem("B5") }),
                new ("Group C", new[] { new MyItem("C1"), new MyItem("C2") })
            };


            _groupsSelectionCoordinator = new GroupsSelectionCoordinator<GroupModel, MyItem>(Groups);
            _groupsSelectionCoordinator.OnItemSelectionChanged = (group, item, isSelected) => {
                Helpers.Diagnostic.Logger.LogDebug($"[{(isSelected ? "Selected" : "Unselected")}] {item.Name} in group {group.Name}");
            };
            //_groupsSelectionCoordinator.OnSelectionStateChanged = (Enums.SelectionState newSelectionState) => {
            //    OnPropertyChanged(nameof(SelectionState));
            //};
            DataContext = this;

            Groups[0].Items[2].IsSelected = true;
        }

        private async void TestButton_Click(object sender, RoutedEventArgs e) {
            using var __logFunctionScope = Helpers.Diagnostic.Logger.LogFunctionScope("TestButton_Click()");

            var group = Groups.FirstOrDefault(g => g.Name == "Group B");
            //_groupsSelectionCoordinator.SelectItem(group.Items[0]);
            //_groupsSelectionCoordinator.SelectItem(group.Items[1]);

            //_groupsSelectionCoordinator.SelectItem(group.Items[1], false);
            //_groupsSelectionCoordinator.SelectItem(group.Items[1], true);

            //_groupsSelectionCoordinator.UnselectItem(group.Items[1], false);
            //_groupsSelectionCoordinator.UnselectItem(group.Items[1], true);

            group.Items.AddRange(new List<MyItem> { new MyItem("B4") });
            await Task.Delay(TimeSpan.FromSeconds(2));
            group.Items.AddRange(new List<MyItem> { new MyItem("B3") });
            await Task.Delay(TimeSpan.FromSeconds(2));
            group.Items.AddRange(new List<MyItem> { new MyItem("B7") });
        }

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void ListView_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            Helpers.Diagnostic.Logger.LogWarning("ListView_SelectionChanged should never be called.");
            System.Diagnostics.Debugger.Break();

            var listView = (ListView)sender;
            var count = listView.SelectedItems.Count;
        }
    }
}