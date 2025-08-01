using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;


namespace Helpers.Collections {
    public sealed class DisposableList<T> : CustomListBase<T>, INotifyCollectionChanged
        where T : IDisposable {

        public event NotifyCollectionChangedEventHandler? CollectionChanged;
        
        public override void Add(T item) {
            base.Add(item);

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Add,
                item,
                _items.Count - 1));
        }

        public override void Insert(int index, T item) {
            base.Insert(index, item);

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Add,
                item,
                index));
        }


        [Obsolete("Use RemoveAndDispose instead.")]
        public override bool Remove(T item) {
            return this.RemoveAndDispose(item);
        }


        [Obsolete("Use RemoveAtAndDispose instead.")]
        public override void RemoveAt(int index) {
            this.RemoveAtAndDispose(index);
        }


        [Obsolete("Use ClearAndDispose instead.")]
        public override void Clear() {
            this.ClearAndDispose();
        }


        public void AddRange(IEnumerable<T> items) {
            var startIndex = _items.Count;
            var addedItems = new List<T>(items);
            _items.AddRange(addedItems);

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Add,
                changedItems: addedItems,
                startingIndex: startIndex));
        }


        public bool RemoveAndDispose(T item) {
            int index = _items.IndexOf(item);
            if (index < 0) {
                return false;
            }

            base.RemoveAt(index);
            item.Dispose();

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Remove,
                item,
                index));

            return true;
        }

        public void RemoveAtAndDispose(int index) {
            var item = _items[index];
            base.RemoveAt(index);
            item.Dispose();

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Remove,
                item,
                index));
        }


        public void ClearAndDispose() {
            foreach (var item in _items) {
                item.Dispose();
            }

            _items.Clear();

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Reset));
        }


        protected override void SetItem(int index, T value) {
            var oldItem = _items[index];
            _items[index] = value;

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Remove,
                oldItem,
                index));

            this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction.Add,
                value,
                index));
        }


        private void OnCollectionChanged(NotifyCollectionChangedEventArgs args) {
            this.CollectionChanged?.Invoke(this, args);
        }
    }


    namespace Ex {
        public static class ListExtensions {
            public static void ex_ClearAndDispose<T>(this List<T> list) where T : IDisposable {
                foreach (var item in list) {
                    item.Dispose();
                }
                list.Clear();
            }
        }
    }
}