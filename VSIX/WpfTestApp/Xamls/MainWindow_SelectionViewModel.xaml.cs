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
    public interface IVisualRefreshNotifiable {
        event Action? ForceVisualRefreshRequested;
    }
    public interface IRangeModifiableCollection {
        void AddRange(IEnumerable<object> toAddItems, bool multiselectionMode = false);
        void RemoveRange(IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
        void AddRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    }

    public class RangeObservableCollection<T> : ObservableCollection<T>, IRangeModifiableCollection, IVisualRefreshNotifiable {
        public event Action<object, IList<T>, IList<T>, bool>? CollectionChangedExtended;
        public event Action? ForceVisualRefreshRequested;
        public IComparer<T>? Comparer { get; set; } = null;

        private bool _suppressNotification = false;
        public RangeObservableCollection() : base() { }

        public RangeObservableCollection(IEnumerable<T> collection) : base(collection) { }


        public void RaiseVisualRefresh() {
            this.ForceVisualRefreshRequested?.Invoke();
        }

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




    public interface ISelectableGroup<TItem> {
        RangeObservableCollection<TItem> Items { get; }
        RangeObservableCollection<TItem> SelectedItems { get; }
    }




    public class MyItem {
        public string Name { get; }

        public MyItem(string name) {
            Name = name;
        }

        public override string ToString() => Name;
    }


    public class GroupModel : ISelectableGroup<MyItem> {
        public string Name { get; }
        public RangeObservableCollection<MyItem> Items { get; } = new();
        public RangeObservableCollection<MyItem> SelectedItems { get; set; } = new();

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
    }

    public class GroupsSelectionCoordinator<TGroup, TItem> where TGroup : ISelectableGroup<TItem> {
        // Events:
        public Action<TGroup, TItem, bool>? OnItemSelectionChanged;
        public Action<Enums.SelectionState>? OnSelectionStateChanged;


        // Properties:
        private Enums.SelectionState _currentSelectionState = Enums.SelectionState.None;
        public Enums.SelectionState SelectionState => _currentSelectionState;


        private (TItem Item, TGroup Group)? _primarySelection;
        public (TItem Item, TGroup Group)? PrimarySelection => _primarySelection;


        // Internal:
        private readonly ObservableCollection<TGroup> _groups;
        private bool _isSyncing;

        public GroupsSelectionCoordinator(ObservableCollection<TGroup> groups) {
            _groups = groups;

            // Подписка на уже существующие
            foreach (var group in _groups) {
                SubscribeToGroup(group);
            }

            _groups.CollectionChanged += OnGroupsChanged;
        }


        public IEnumerable<(TItem Item, TGroup Group)> GetAllSelectedItems() {
            foreach (var group in _groups) {
                foreach (var item in group.SelectedItems) {
                    yield return (item, group);
                }
            }
        }

        private void OnGroupsChanged(object sender, NotifyCollectionChangedEventArgs e) {
            if (e.NewItems != null) {
                foreach (TGroup group in e.NewItems) {
                    SubscribeToGroup(group);
                }
            }

            if (e.OldItems != null) {
                foreach (TGroup group in e.OldItems) {
                    UnsubscribeFromGroup(group);
                }
            }
        }

        private void SubscribeToGroup(TGroup group) {
            // Отписываемся на всякий случай, даже если раньше не подписывались
            group.SelectedItems.CollectionChangedExtended -= HandleGroupSelectionChanged;
            group.SelectedItems.CollectionChangedExtended += HandleGroupSelectionChanged;
        }
        private void UnsubscribeFromGroup(TGroup group) {
            group.SelectedItems.CollectionChangedExtended -= HandleGroupSelectionChanged;
        }



        public void SelectItem(TItem item, bool multiselectionMode = false) {
#if ENABLE_VERBOSE_LOGGING
            using var __logFunctionScope = Helpers.Diagnostic.Logger.LogFunctionScope("SelectItem()");
            Helpers.Diagnostic.Logger.LogParam($"item: {item}");
#endif
            TGroup? targetGroup = default;
            foreach (var group in _groups) {
                if (group.Items.Contains(item)) {
                    targetGroup = group;
                    break;
                }
            }

            if (targetGroup == null) {
                Helpers.Diagnostic.Logger.LogWarning($"Item not found in any group: {item}");
                return;
            }

            if (targetGroup.SelectedItems.Contains(item)) {
                Helpers.Diagnostic.Logger.LogWarning($"Skip, targetGroup.SelectedItems already contain the item = {item}");
                return;
            }

            var addedItems = new List<TItem>() { item };
            var removedItems = multiselectionMode
                ? new List<TItem>()
                : targetGroup.SelectedItems.ToList();

            targetGroup.SelectedItems.AddRemoveRange(
                addedItems.Cast<object>(),
                removedItems.Cast<object>(),
                multiselectionMode
                );
        }

        public void UnselectItem(TItem item, bool multiselectionMode = false) {
#if ENABLE_VERBOSE_LOGGING
            using var __logFunctionScope = Helpers.Diagnostic.Logger.LogFunctionScope("UnselectItem()");
            Helpers.Diagnostic.Logger.LogParam($"item: {item}");
#endif
            TGroup? targetGroup = default;
            foreach (var group in _groups) {
                if (group.Items.Contains(item)) {
                    targetGroup = group;
                    break;
                }
            }

            if (targetGroup == null) {
                Helpers.Diagnostic.Logger.LogWarning($"Item not found in any group: {item}");
                return;
            }

            if (!targetGroup.SelectedItems.Contains(item)) {
                Helpers.Diagnostic.Logger.LogWarning($"Skip, item is not selected: {item}");
                return;
            }

            var addedItems = new List<TItem>();
            var removedItems = multiselectionMode
                ? new List<TItem> { item }
                : targetGroup.SelectedItems.ToList();

            targetGroup.SelectedItems.AddRemoveRange(
                addedItems.Cast<object>(),
                removedItems.Cast<object>(),
                multiselectionMode
                );
        }


        private (TItem Item, TGroup Group)? _selectionAnchor = null;





        private void HandleGroupSelectionChanged(object sender, IList<TItem> addedItems, IList<TItem> removedItems, bool multiselectionMode = false) {
#if ENABLE_VERBOSE_LOGGING
            using var __log = Helpers.Diagnostic.Logger.LogFunctionScope("HandleGroupSelectionChanged()");
            Helpers.Diagnostic.Logger.LogParam($"addedItems: {{{string.Join(", ", addedItems)}}}");
            Helpers.Diagnostic.Logger.LogParam($"removedItems: {{{string.Join(", ", removedItems)}}}");
#endif

            if (sender is not ObservableCollection<TItem> selectedItems) {
                return;
            }

            var group = _groups.FirstOrDefault(g => ReferenceEquals(g.SelectedItems, selectedItems));
            if (group == null) {
                return;
            }

            var finalAdded = new List<(TGroup Group, TItem Item)>();
            var finalRemoved = new List<(TGroup Group, TItem Item)>();


            bool isShiftPressed = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
            if (!isShiftPressed) {
                if (addedItems.Count == 1) {
                    _selectionAnchor = (addedItems[0], group);
                    Helpers.Diagnostic.Logger.LogDebug($"set _selectionAnchor = {_selectionAnchor.Value.Item}");
                }
                else if (addedItems.Count == 0 && removedItems.Count > 0 && group.SelectedItems.Count == 1) {
                    // Сняли выделение со всех и остался 1 выбранный — считаем его якорем
                    var remaining = group.SelectedItems[0];
                    _selectionAnchor = (remaining, group);
                    Helpers.Diagnostic.Logger.LogDebug($"set _selectionAnchor (based on final selection) = {_selectionAnchor.Value.Item}");
                }
            }
            if (isShiftPressed) {
                if (_selectionAnchor.HasValue) {
                    var current = addedItems.LastOrDefault();
                    if (current != null) {
                        Helpers.Diagnostic.Logger.LogDebug($"_selectionAnchor: {_selectionAnchor.Value.Item}");
                        Helpers.Diagnostic.Logger.LogDebug($"         current: {current}");

                        // 1. Построить линейный список
                        var flat = _groups
                            .SelectMany(g => g.Items.Select(i => (Group: g, Item: i)))
                            .ToList();

                        // 2. Найти границы
                        var anchor = _selectionAnchor ?? (current, group);
                        int i1 = flat.FindIndex(x => EqualityComparer<TItem>.Default.Equals(x.Item, anchor.Item));
                        int i2 = flat.FindIndex(x => EqualityComparer<TItem>.Default.Equals(x.Item, current));

                        if (i1 != -1 && i2 != -1) {
                            int from = Math.Min(i1, i2);
                            int to = Math.Max(i1, i2);

                            var range = flat.GetRange(from, to - from + 1);
                            Helpers.Diagnostic.Logger.LogDebug($"select range: {{{string.Join(", ", range.Select(i => i.Item))}}}");


                            foreach (var g in _groups) {
                                var target = range.Where(x => EqualityComparer<TGroup>.Default.Equals(x.Group, g)).Select(x => x.Item).ToList();

                                // Удаляем всё, что не входит в целевой диапазон
                                foreach (var item in g.SelectedItems.ToList()) {
                                    if (!target.Contains(item)) {
                                        g.SelectedItems.Remove(item);
                                        finalRemoved.Add((g, item));
                                    }
                                }

                                // Добавляем всё, чего ещё нет
                                foreach (var item in target) {
                                    if (!g.SelectedItems.Contains(item)) {
                                        g.SelectedItems.Add(item);
                                        finalAdded.Add((g, item));
                                    }
                                }
                            }

                            this.NotifySelection(finalAdded, finalRemoved);
                            return;
                        }
                    }
                }
            }

            foreach (var item in addedItems) {
                finalAdded.Add((group, item));
            }
            foreach (var item in removedItems) {
                finalRemoved.Add((group, item));
            }

            if (!multiselectionMode) {
                // ❌ Снимаем выделение со всех других групп
                foreach (var otherGroup in _groups) {
                    if (!ReferenceEquals(otherGroup, group)) {
                        foreach (var item in otherGroup.SelectedItems.ToList()) {
                            otherGroup.SelectedItems.Remove(item);
                            finalRemoved.Add((otherGroup, item));
                        }
                    }
                }
            }

            this.NotifySelection(finalAdded, finalRemoved);
        }

        private void NotifySelection(
            List<(TGroup Group, TItem Item)> addedEntries,
            List<(TGroup Group, TItem Item)> removedEntries
            ) {
            this.UpdateSelectionState();

#if ENABLE_VERBOSE_LOGGING
            Helpers.Diagnostic.Logger.LogDebug($"entriesAdded: {{{string.Join(", ", addedEntries.Select(i => i.Item))}}}");
            Helpers.Diagnostic.Logger.LogDebug($"entriesRemoved: {{{string.Join(", ", removedEntries.Select(i => i.Item))}}}");
#endif

            foreach (var (g, item) in addedEntries) {
                this.OnItemSelectionChanged?.Invoke(g, item, true);
            }
            foreach (var (g, item) in removedEntries) {
                this.OnItemSelectionChanged?.Invoke(g, item, false);
            }
        }


        private void UpdateSelectionState() {
            (TItem Item, TGroup Group)? newPrimary = null;
            int totalCount = 0;

            foreach (var group in _groups) {
                foreach (var item in group.SelectedItems) {
                    totalCount++;
                    if (totalCount == 1) {
                        newPrimary = (item, group);
                    }
                    else {
                        newPrimary = null;
                        break;
                    }
                }
                if (totalCount > 1)
                    break;
            }

            var newState = totalCount switch {
                0 => Enums.SelectionState.None,
                1 => Enums.SelectionState.Single,
                _ => Enums.SelectionState.Multiple
            };

            _primarySelection = newPrimary;

            if (_currentSelectionState != newState) {
                _currentSelectionState = newState;
                OnSelectionStateChanged?.Invoke(_currentSelectionState);
            }
        }
    }







    public partial class MainWindow_SelectionViewModel : Window, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ObservableCollection<GroupModel> Groups { get; }
        private GroupsSelectionCoordinator<GroupModel, MyItem> _groupsSelectionCoordinator;

        public string SelectionState {
            get => _groupsSelectionCoordinator.SelectionState.ToString();
        }

        public MainWindow_SelectionViewModel() {
            InitializeComponent();

            Groups = new ObservableCollection<GroupModel> {
                new ("Group A", new[] { new MyItem("A3"), new MyItem("A2"), new MyItem("A1"), new MyItem("A4") }),
                new ("Group B", new[] { new MyItem("B1"), new MyItem("B2"), new MyItem("B5") }),
                new ("Group C", new[] { new MyItem("C1"), new MyItem("C2") })
            };

            _groupsSelectionCoordinator = new GroupsSelectionCoordinator<GroupModel, MyItem>(Groups);
            _groupsSelectionCoordinator.OnItemSelectionChanged = (group, item, isSelected) => {
                Helpers.Diagnostic.Logger.LogDebug($"[{(isSelected ? "Selected" : "Unselected")}] {item.Name} in group {group.Name}");
            };
            _groupsSelectionCoordinator.OnSelectionStateChanged = (Enums.SelectionState newSelectionState) => {
                OnPropertyChanged(nameof(SelectionState));
            };
            DataContext = this;
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
    }
}