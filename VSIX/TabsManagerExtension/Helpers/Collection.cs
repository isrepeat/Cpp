using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Windows.Input;
using System.Windows;
using System.Collections.Specialized;
using System.Linq;
using System;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace TabsManagerExtension.Helpers {
    public interface ISelectableGroup<TItem> {
        ObservableCollection<TItem> SelectedItems { get; }
    }


    public class SelectionCoordinator<TGroup, TItem> where TGroup : ISelectableGroup<TItem> {
        public Action<TGroup, TItem, bool>? OnItemSelectionChanged;

        private readonly ObservableCollection<TGroup> _groups;
        private bool _isSyncing;

        public SelectionCoordinator(ObservableCollection<TGroup> groups) {
            _groups = groups;

            // Подписка на уже существующие
            foreach (var group in _groups) {
                SubscribeToGroup(group);
            }

            // Динамически обрабатываем добавление/удаление
            _groups.CollectionChanged += OnGroupsChanged;
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
            group.SelectedItems.CollectionChanged -= HandleGroupSelectionChanged;
            group.SelectedItems.CollectionChanged += HandleGroupSelectionChanged;
        }
        private void UnsubscribeFromGroup(TGroup group) {
            group.SelectedItems.CollectionChanged -= HandleGroupSelectionChanged;
        }

        private void HandleGroupSelectionChanged(object sender, NotifyCollectionChangedEventArgs e) {
            if (sender is not ObservableCollection<TItem> selectedItems)
                return;

            var group = _groups.FirstOrDefault(g => ReferenceEquals(g.SelectedItems, selectedItems));
            if (group == null)
                return;

            if (e.Action == NotifyCollectionChangedAction.Add && e.NewItems != null) {
                foreach (TItem item in e.NewItems) {
                    OnItemSelectionChanged?.Invoke(group, item, true);
                }
            }
            else if (e.Action == NotifyCollectionChangedAction.Remove && e.OldItems != null) {
                foreach (TItem item in e.OldItems) {
                    OnItemSelectionChanged?.Invoke(group, item, false);
                }
            }
            else if (e.Action == NotifyCollectionChangedAction.Reset) {
                // Reset не обрабатываем — мы сами отлавливаем очистку вручную при OnGroupSelectionChanged.
            }

            OnGroupSelectionChanged(group);
        }



        private void OnGroupSelectionChanged(TGroup changedGroup) {
            // Защита от повторного входа, если метод уже выполняется
            if (_isSyncing) return;

            // Если зажат Ctrl — разрешаем множественный выбор, не сбрасываем другие группы
            if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
                return;

            // Устанавливаем флаг, чтобы предотвратить повторные вызовы из-за CollectionChanged
            _isSyncing = true;

            // Выполняем логику асинхронно в UI-диспетчере
            // Это защищает от исключений и конфликтов в WPF/COM, например, при выборе вкладок
            Application.Current.Dispatcher.InvokeAsync(() => {

                // Проходим по всем группам, кроме той, где произошло текущее выделение
                foreach (var group in _groups) {
                    if (!ReferenceEquals(group, changedGroup)) {
                        // Сохраняем текущее состояние выделения перед очисткой
                        var removedItems = group.SelectedItems.ToList();

                        // Снимаем выделение в этой группе
                        group.SelectedItems.Clear(); // вызовет CollectionChanged (Reset)

                        // Явно уведомляем, что эти элементы были сняты
                        foreach (var item in removedItems) {
                            OnItemSelectionChanged?.Invoke(group, item, false);
                        }
                    }
                }
                _isSyncing = false;
            });
        }

        public IEnumerable<(TItem Item, TGroup Group)> GetAllSelectedItems() {
            foreach (var group in _groups) {
                foreach (var item in group.SelectedItems) {
                    yield return (item, group);
                }
            }
        }
    }


    namespace Ex {
        public static class CollectionExtensions {
            public static IEnumerable<(TGroup Group, TItem Item)> GetAllSelectedItems<TGroup, TItem>(
                this IEnumerable<TGroup> groups) where TGroup : ISelectableGroup<TItem> {
                foreach (var group in groups) {
                    foreach (var item in group.SelectedItems) {
                        yield return (group, item);
                    }
                }
            }
        }
    }
}