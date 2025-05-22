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

// TODO: Сделай такую же структуру проекта как и у SimpleApp.WinUI3
namespace WpfTestApp {
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
                foreach (var removed in e.RemovedItems) {
                    boundList.Remove(removed);
                }
                foreach (var added in e.AddedItems) {
                    if (!boundList.Contains(added)) {
                        boundList.Add(added);
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
        ObservableCollection<TItem> SelectedItems { get; }
    }

    public static class SelectedItemsHelper {
        public static IEnumerable<(TGroup Group, TItem Item)> GetAllSelectedItems<TGroup, TItem>(
            this IEnumerable<TGroup> groups) where TGroup : ISelectableGroup<TItem> {
            foreach (var group in groups) {
                foreach (var item in group.SelectedItems) {
                    yield return (group, item);
                }
            }
        }
    }


    public class MyItem {
        public string Name { get; }

        public MyItem(string name) {
            Name = name;
        }

        public override string ToString() => Name;
    }


    public class GroupModel<TItem> : ISelectableGroup<TItem> {
        public string Name { get; }
        public ObservableCollection<TItem> Items { get; }
        public ObservableCollection<TItem> SelectedItems { get; set; } = new();

        public GroupModel(string name, IEnumerable<TItem> items) {
            Name = name;
            Items = new ObservableCollection<TItem>(items);
        }
    }


    public class SelectionCoordinator<TGroup, TItem> where TGroup : ISelectableGroup<TItem> {
        private readonly IEnumerable<TGroup> _groups;
        private bool _isSyncing;

        public SelectionCoordinator(IEnumerable<TGroup> groups) {
            _groups = groups;

            foreach (var group in _groups) {
                group.SelectedItems.CollectionChanged += (_, __) => OnGroupSelectionChanged(group);
            }
        }

        private void OnGroupSelectionChanged(TGroup changedGroup) {
            if (_isSyncing) return;
            if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
                return;

            _isSyncing = true;

            Application.Current.Dispatcher.InvokeAsync(() => {
                foreach (var group in _groups) {
                    if (!ReferenceEquals(group, changedGroup))
                        group.SelectedItems.Clear();
                }

                _isSyncing = false;
            });
        }

        public IEnumerable<(TGroup Group, TItem Item)> GetAllSelectedItems() {
            foreach (var group in _groups) {
                foreach (var item in group.SelectedItems) {
                    yield return (group, item);
                }
            }
        }
    }



    public partial class MainWindow : Window, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ObservableCollection<GroupModel<MyItem>> Groups { get; }
        private SelectionCoordinator<GroupModel<MyItem>, MyItem> _selectionCoordinator;

        public MainWindow() {
            InitializeComponent();

            Groups = new ObservableCollection<GroupModel<MyItem>> {
                new ("Group A", new[] { new MyItem("A1"), new MyItem("A2"), new MyItem("A3") }),
                new ("Group B", new[] { new MyItem("B1"), new MyItem("B2"), new MyItem("B3") }),
                new ("Group C", new[] { new MyItem("C1"), new MyItem("C2") })
            };

            _selectionCoordinator = new SelectionCoordinator<GroupModel<MyItem>, MyItem>(Groups);
            DataContext = this;
        }

        private void ShowSelectedItems_Click(object sender, RoutedEventArgs e) {
            foreach (var (group, item) in _selectionCoordinator.GetAllSelectedItems()) {
                Debug.WriteLine($"Группа: {(group).Name}, Элемент: {item}");
            }
        }

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}