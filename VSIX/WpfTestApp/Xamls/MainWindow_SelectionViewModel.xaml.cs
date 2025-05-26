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
    }
    public interface ISelectableGroup<TItem> {
        RangeObservableCollection<TItem> Items { get; }
    }




    public class MyItem : Helpers.ObservableObject, ISelectableItem {
        private bool _isSelected;
        public bool IsSelected {
            get => _isSelected;
            set {
                if (_isSelected != value) {
                    _isSelected = value;
                    this.OnPropertyChanged();
                }
            }
        }

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
    }


    public class GroupsSelectionCoordinator<TGroup, TItem>
        where TGroup : ISelectableGroup<TItem>
        where TItem : ISelectableItem, INotifyPropertyChanged {

        // Events:
        public Action<TGroup, TItem, bool>? OnItemSelectionChanged;
        public Action<Enums.SelectionState>? OnSelectionStateChanged;


        private readonly ObservableCollection<TGroup> _groups;
        private readonly Dictionary<TGroup, NotifyCollectionChangedEventHandler> _collectionChangedHandlers = new();
        private readonly Dictionary<TItem, PropertyChangedEventHandler> _itemHandlers = new();


        public GroupsSelectionCoordinator(ObservableCollection<TGroup> groups) {
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

        private void AttachGroup(TGroup group) {
            foreach (var item in group.Items) {
                var handler = new PropertyChangedEventHandler((_, e) => this.OnItemChanged(group, item, e));
                _itemHandlers[item] = handler;
                item.PropertyChanged += handler;
            }

            NotifyCollectionChangedEventHandler collectionHandler = (s, e) => {
                if (e.NewItems != null) {
                    foreach (var item in e.NewItems.Cast<TItem>()) {
                        var handler = new PropertyChangedEventHandler((_, ev) => this.OnItemChanged(group, item, ev));
                        _itemHandlers[item] = handler;
                        item.PropertyChanged += handler;
                    }
                }

                if (e.OldItems != null) {
                    foreach (var item in e.OldItems.Cast<TItem>()) {
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
                if (_itemHandlers.TryGetValue(item, out var handler)) {
                    item.PropertyChanged -= handler;
                    _itemHandlers.Remove(item);
                }
            }
        }

        private void OnItemChanged(TGroup group, TItem item, PropertyChangedEventArgs e) {
            if (e.PropertyName == nameof(ISelectableItem.IsSelected)) {
                this.OnItemSelectionChanged?.Invoke(group, item, item.IsSelected);
            }
        }
    }




    public partial class MainWindow_SelectionViewModel : Window, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ObservableCollection<GroupModel> Groups { get; }
        private GroupsSelectionCoordinator<GroupModel, MyItem> _groupsSelectionCoordinator;

        public string SelectionState {
            //get => _groupsSelectionCoordinator.SelectionState.ToString();
            get => "...";
        }

        public MainWindow_SelectionViewModel() {
            InitializeComponent();

            Groups = new ObservableCollection<GroupModel> {
                new ("Group A", new[] { new MyItem("A3"), new MyItem("A2"), new MyItem("A1"), new MyItem("A4") }),
                new ("Group B", new[] { new MyItem("B1"), new MyItem("B2"), new MyItem("B5") }),
                new ("Group C", new[] { new MyItem("C1"), new MyItem("C2") })
            };

            Groups[0].Items[2].IsSelected = true;

            _groupsSelectionCoordinator = new GroupsSelectionCoordinator<GroupModel, MyItem>(Groups);
            _groupsSelectionCoordinator.OnItemSelectionChanged = (group, item, isSelected) => {
                Helpers.Diagnostic.Logger.LogDebug($"[{(isSelected ? "Selected" : "Unselected")}] {item.Name} in group {group.Name}");
            };
            //_groupsSelectionCoordinator.OnSelectionStateChanged = (Enums.SelectionState newSelectionState) => {
            //    OnPropertyChanged(nameof(SelectionState));
            //};
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

        private void ListView_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            Helpers.Diagnostic.Logger.LogWarning("ListView_SelectionChanged should never be called.");
            System.Diagnostics.Debugger.Break();

            var listView = (ListView)sender;
            var count = listView.SelectedItems.Count;
        }
    }
}