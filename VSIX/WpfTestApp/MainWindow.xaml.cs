#define ENABLE_VERBOSE_LOGGING
using System;
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


// TODO: Сделай такую же структуру проекта как и у SimpleApp.WinUI3
namespace WpfTestApp {
    public interface IRangeModifiableCollection {
        void AddRange(IEnumerable<object> items);
        void RemoveRange(IEnumerable<object> items);
        void AddAndRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false);
    }

    public class RangeObservableCollection<T> : ObservableCollection<T>, IRangeModifiableCollection {
        private bool _suppressNotification = false;

        public RangeObservableCollection() : base() { }

        public RangeObservableCollection(IEnumerable<T> collection) : base(collection) { }

        public event Action<object, IList<T>, IList<T>, bool>? CollectionChangedExtended;

        public void AddRange(IEnumerable<object> items) {
            //if (items == null) {
            //    return;
            //}

            //var addedItems = items.OfType<T>().ToList();
            //if (addedItems.Count == 0) {
            //    return;
            //}

            //_suppressNotification = true;
            //foreach (var item in addedItems) {
            //    Add(item);
            //}
            //_suppressNotification = false;

            //OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, addedItems));
        }

        public void RemoveRange(IEnumerable<object> items) {
            //if (items == null) {
            //    return;
            //}

            //var removedItems = items.OfType<T>().ToList();
            //if (removedItems.Count == 0) {
            //    return;
            //}

            //_suppressNotification = true;
            //foreach (var item in removedItems) {
            //    Remove(item);
            //}
            //_suppressNotification = false;

            //OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, removedItems));
        }

        public void AddAndRemoveRange(IEnumerable<object> toAddItems, IEnumerable<object> toRemoveItems, bool multiselectionMode = false) {
            var addedItems = toAddItems?.OfType<T>().ToList() ?? new();
            var removedItems = toRemoveItems?.OfType<T>().ToList() ?? new();

            _suppressNotification = true;
            foreach (var item in addedItems) {
                Add(item);
            }
            foreach (var item in removedItems) {
                Remove(item);
            }
            _suppressNotification = false;

            // Для UI уведомлений
            OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, addedItems));
            OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, removedItems));

            // Для внутренней логики
            CollectionChangedExtended?.Invoke(this, addedItems, removedItems, multiselectionMode);
        }

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e) {
            if (!_suppressNotification)
                base.OnCollectionChanged(e);
        }
    }


    public static class ListBoxSelectedItemsBehavior {
        private static readonly Dictionary<ListBox, bool> SyncFlags = new();

        public static readonly DependencyProperty BindableSelectedItemsProperty =
            DependencyProperty.RegisterAttached(
                "BindableSelectedItems",
                typeof(System.Collections.IList),
                typeof(ListBoxSelectedItemsBehavior),
                new PropertyMetadata(null, OnBindableSelectedItemsChanged));

        public static void SetBindableSelectedItems(DependencyObject element, System.Collections.IList value) {
            element.SetValue(BindableSelectedItemsProperty, value);
        }

        public static System.Collections.IList GetBindableSelectedItems(DependencyObject element) {
            return (System.Collections.IList)element.GetValue(BindableSelectedItemsProperty);
        }

        private static void OnBindableSelectedItemsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            if (d is not ListBox listBox) {
                return;
            }

            listBox.SelectionChanged -= ListBox_SelectionChanged;
            listBox.SelectionChanged += ListBox_SelectionChanged;

            if (e.OldValue is INotifyCollectionChanged oldCollection) {
                oldCollection.CollectionChanged -= (s, args) => SyncListBoxSelection(listBox);
            }
            if (e.NewValue is INotifyCollectionChanged newCollection) {
                newCollection.CollectionChanged += (s, args) => SyncListBoxSelection(listBox);
            }
            SyncListBoxSelection(listBox);
        }

        private static void ListBox_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            if (sender is not ListBox listBox) {
                return;
            }

            var boundList = GetBindableSelectedItems(listBox);
            if (boundList == null) {
                return;
            }

            if (IsSyncing(listBox)) {
                return;
            }
            SetSyncing(listBox, true);

            listBox.Dispatcher.InvokeAsync(() => {
                if (boundList is IRangeModifiableCollection rangeList) {
                    bool multiselectionMode = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;
                    rangeList.AddAndRemoveRange(
                        e.AddedItems.Cast<object>().Where(item => !boundList.Contains(item)),
                        e.RemovedItems.Cast<object>(),
                        multiselectionMode
                        );

                }
                else {
                    foreach (var added in e.AddedItems) {
                        if (!boundList.Contains(added)) {
                            boundList.Add(added);
                        }
                    }
                    foreach (var removed in e.RemovedItems) {
                        boundList.Remove(removed);
                    }
                }

                SetSyncing(listBox, false);
            });
        }

        private static void SyncListBoxSelection(ListBox listBox) {
            if (IsSyncing(listBox)) {
                return;
            }
            SetSyncing(listBox, true);

            var boundList = GetBindableSelectedItems(listBox);
            if (boundList == null) {
                return;
            }

            listBox.SelectedItems.Clear();

            foreach (var item in boundList) {
                listBox.SelectedItems.Add(item);
            }
            SetSyncing(listBox, false);
        }

        private static bool IsSyncing(ListBox listBox) {
            return SyncFlags.TryGetValue(listBox, out var syncing) && syncing;
        }

        private static void SetSyncing(ListBox listBox, bool value) {
            SyncFlags[listBox] = value;
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
            Items = new RangeObservableCollection<MyItem>(items);
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

            targetGroup.SelectedItems.AddAndRemoveRange(
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

            targetGroup.SelectedItems.AddAndRemoveRange(
                addedItems.Cast<object>(),
                removedItems.Cast<object>(),
                multiselectionMode
                );
        }

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

            UpdateSelectionState();

            foreach (var item in addedItems) {
                finalAdded.Add((group, item));
            }
            foreach (var item in removedItems) {
                finalRemoved.Add((group, item));
            }

#if ENABLE_VERBOSE_LOGGING
            Helpers.Diagnostic.Logger.LogDebug($"finalAdded: {{{string.Join(", ", finalAdded.Select(i => i.Item))}}}");
            Helpers.Diagnostic.Logger.LogDebug($"finalRemoved: {{{string.Join(", ", finalRemoved.Select(i => i.Item))}}}");
#endif

            // 🔔 Вызов событий централизованно
            foreach (var (g, item) in finalAdded) {
                OnItemSelectionChanged?.Invoke(g, item, true);
            }
            foreach (var (g, item) in finalRemoved) {
                OnItemSelectionChanged?.Invoke(g, item, false);
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







    public partial class MainWindow : Window, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;


        public ObservableCollection<GroupModel> Groups { get; }
        private GroupsSelectionCoordinator<GroupModel, MyItem> _groupsSelectionCoordinator;

        public string SelectionState {
            get => _groupsSelectionCoordinator.SelectionState.ToString();
        }


        public MainWindow() {
            InitializeComponent();

            Groups = new ObservableCollection<GroupModel> {
                new ("Group A", new[] { new MyItem("A1"), new MyItem("A2"), new MyItem("A3"), new MyItem("A4") }),
                new ("Group B", new[] { new MyItem("B1"), new MyItem("B2"), new MyItem("B3") }),
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

        private void TestButton_Click(object sender, RoutedEventArgs e) {
            using var __logFunctionScope = Helpers.Diagnostic.Logger.LogFunctionScope("TestButton_Click()");

            var group = Groups.FirstOrDefault(g => g.Name == "Group B");
            //_groupsSelectionCoordinator.SelectItem(group.Items[0]);
            //_groupsSelectionCoordinator.SelectItem(group.Items[1]);
            
            //_groupsSelectionCoordinator.SelectItem(group.Items[1], false);
            //_groupsSelectionCoordinator.SelectItem(group.Items[1], true);
            
            //_groupsSelectionCoordinator.UnselectItem(group.Items[1], false);
            _groupsSelectionCoordinator.UnselectItem(group.Items[1], true);
        }

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}