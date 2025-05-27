using System.Collections.Generic;
using System.Collections.Specialized;
using System.Windows.Controls;
using System.Windows;

namespace TabsManagerExtension.Behaviours {
    public static class ListBoxSelectedItemsBehavior {
        private static readonly Dictionary<ListBox, bool> SyncFlags = new();
        private static readonly Dictionary<ListBox, NotifyCollectionChangedEventHandler> CollectionChangedHandlers = new();

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
            if (d is not ListBox listBox)
                return;

            // Подписка на SelectionChanged UI
            listBox.SelectionChanged -= ListBox_SelectionChanged;
            listBox.SelectionChanged += ListBox_SelectionChanged;

            // Отписка от предыдущей коллекции, если есть
            if (e.OldValue is INotifyCollectionChanged oldCollection && CollectionChangedHandlers.TryGetValue(listBox, out var oldHandler)) {
                oldCollection.CollectionChanged -= oldHandler;
                CollectionChangedHandlers.Remove(listBox);
            }

            // Подписка к новой коллекции
            if (e.NewValue is INotifyCollectionChanged newCollection) {
                NotifyCollectionChangedEventHandler newHandler = (_, __) => SyncListBoxSelection(listBox);
                newCollection.CollectionChanged += newHandler;
                CollectionChangedHandlers[listBox] = newHandler;
            }

            // Синхронизируем состояние
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

        private static void CollectionChangedHandler(object? sender, NotifyCollectionChangedEventArgs e) {
            if (sender is ListBox listBox) {
                SyncListBoxSelection(listBox);
            }
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
}