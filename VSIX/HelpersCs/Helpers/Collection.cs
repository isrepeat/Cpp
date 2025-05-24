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
    }

    public interface ISelectableGroup<TItem> {
        ObservableCollection<TItem> Items { get; }
        ObservableCollection<TItem> SelectedItems { get; }
    }


    public class SelectionCoordinator<TGroup, TItem> where TGroup : ISelectableGroup<TItem> {
        // Events:
        public Action<TGroup, TItem, bool>? OnItemSelectionChanged;
        public Action<Enums.SelectionState>? OnSelectionStateChanged;


        // Properties:
        private Enums.SelectionState _currentSelectionState = Enums.SelectionState.Single;
        public Enums.SelectionState SelectionState => _currentSelectionState;

        
        private (TItem Item, TGroup Group)? _primarySelection;
        public (TItem Item, TGroup Group)? PrimarySelection => _primarySelection;


        // Internal:
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


        public IEnumerable<(TItem Item, TGroup Group)> GetAllSelectedItems() {
            foreach (var group in _groups) {
                foreach (var item in group.SelectedItems) {
                    yield return (item, group);
                }
            }
        }

        //public void SelectItem((TItem Item, TGroup Group) pair, bool additive = false) {
        //    var (item, group) = pair;

        //    if (!_groups.Contains(group)) {
        //        return;
        //    }

        //    if (!additive) {
        //        foreach (var otherGroup in _groups) {
        //            if (!ReferenceEquals(otherGroup, group)) {
        //                var removedItems = otherGroup.SelectedItems.ToList();
        //                otherGroup.SelectedItems.Clear();
        //                foreach (var removed in removedItems) {
        //                    OnItemSelectionChanged?.Invoke(otherGroup, removed, false);
        //                }
        //            }
        //        }

        //        // Внутри группы тоже убираем старые
        //        var toUnselect = group.SelectedItems.Where(i => !Equals(i, item)).ToList();
        //        group.SelectedItems.Clear();
        //        foreach (var removed in toUnselect) {
        //            OnItemSelectionChanged?.Invoke(group, removed, false);
        //        }
        //    }

        //    if (!group.SelectedItems.Contains(item)) {
        //        group.SelectedItems.Add(item);
        //        OnItemSelectionChanged?.Invoke(group, item, true);
        //    }

        //    UpdateSelectionState();
        //}


        public void SelectItem__(TItem item, bool additive = false) {
            // Находим группу, содержащую этот item в SelectedItems
            TGroup targetGroup = default;
            foreach (var group in _groups) {
                if (group.SelectedItems.Contains(item)) {
                    targetGroup = group;
                    break;
                }
            }

            if (targetGroup == null) {
                // Не удалось найти группу — ничего не делаем
                return;
            }

            // Если не additive — снимаем выделение с других групп
            if (!additive) {
                foreach (var otherGroup in _groups) {
                    if (!ReferenceEquals(otherGroup, targetGroup)) {
                        var toUnselect = otherGroup.SelectedItems.ToList();
                        otherGroup.SelectedItems.Clear();

                        foreach (var removed in toUnselect) {
                            OnItemSelectionChanged?.Invoke(otherGroup, removed, false);
                        }
                    }
                }

                // И внутри группы — убираем все, кроме текущего
                var toUnselectFromSame = targetGroup.SelectedItems
                    .Where(i => !Equals(i, item))
                    .ToList();

                targetGroup.SelectedItems.Clear();

                foreach (var removed in toUnselectFromSame) {
                    OnItemSelectionChanged?.Invoke(targetGroup, removed, false);
                }
            }

            // Добавляем item, если его ещё нет
            if (!targetGroup.SelectedItems.Contains(item)) {
                targetGroup.SelectedItems.Add(item);
                OnItemSelectionChanged?.Invoke(targetGroup, item, true);
            }

            UpdateSelectionState();
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



        public void SelectItem(TItem item, bool additive = false) {
            TGroup targetGroup = default;

            foreach (var group in _groups) {
                if (group.Items.Contains(item)) {
                    targetGroup = group;
                    break;
                }
            }

            if (targetGroup == null)
                return;

            ApplySelection(item, targetGroup, additive);
        }

        private void HandleGroupSelectionChanged(object sender, NotifyCollectionChangedEventArgs e) {
            if (sender is not ObservableCollection<TItem> selectedItems) {
                return;
            }

            // Ищем группу в которой произошли изменения
            var group = _groups.FirstOrDefault(g => ReferenceEquals(g.SelectedItems, selectedItems));
            if (group == null) {
                return;
            }

            UpdateSelectionState();

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
            if (_isSyncing) {
                return;
            }

            // Если зажат Ctrl — разрешаем множественный выбор, не сбрасываем другие группы
            if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control) {
                return;
            }

            Application.Current.Dispatcher.InvokeAsync(() => {
                ApplySelection(changedGroup.SelectedItems.FirstOrDefault(), changedGroup, false);
            });
        }


        private void ApplySelection(TItem itemToSelect, TGroup targetGroup, bool additive) {
            if (_isSyncing) {
                return;
            }
            _isSyncing = true;

            if (!additive) {
                // Снимаем выделение со всех остальных групп
                foreach (var group in _groups) {
                    if (!ReferenceEquals(group, targetGroup)) {
                        var toUnselect = group.SelectedItems.ToList();
                        group.SelectedItems.Clear();

                        foreach (var removed in toUnselect) {
                            OnItemSelectionChanged?.Invoke(group, removed, false);
                        }
                    }
                }

                // Снимаем выделение в самой группе (всё, кроме item)
                var toUnselectFromSame = targetGroup.SelectedItems
                    .Where(i => !Equals(i, itemToSelect))
                    .ToList();

                targetGroup.SelectedItems.Clear();

                foreach (var removed in toUnselectFromSame) {
                    OnItemSelectionChanged?.Invoke(targetGroup, removed, false);
                }
            }

            // Добавляем item, если он ещё не выбран
            if (!targetGroup.SelectedItems.Contains(itemToSelect)) {
                targetGroup.SelectedItems.Add(itemToSelect);
                OnItemSelectionChanged?.Invoke(targetGroup, itemToSelect, true);
            }

            UpdateSelectionState();
            _isSyncing = false;
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