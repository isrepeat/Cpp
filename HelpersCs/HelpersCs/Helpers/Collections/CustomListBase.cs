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
    public abstract class CustomListBase<T> : IList<T>, IReadOnlyList<T> {
        public bool IsReadOnly => false;
        public int Count => _items.Count;

        protected readonly List<T> _items = new();


        public T this[int index] {
            get => this.GetItem(index);
            set => this.SetItem(index, value);
        }


        public virtual void Add(T item) {
            _items.Add(item);
        }

        public virtual void Insert(int index, T item) {
            _items.Insert(index, item);
        }

        public virtual bool Remove(T item) {
            return _items.Remove(item);
        }

        public virtual void RemoveAt(int index) {
            _items.RemoveAt(index);
        }

        public virtual void Clear() {
            _items.Clear();
        }


        public bool Contains(T item) {
            return _items.Contains(item);
        }

        public int IndexOf(T item) {
            return _items.IndexOf(item);
        }

        public void CopyTo(T[] array, int arrayIndex) {
            _items.CopyTo(array, arrayIndex);
        }

        public IEnumerator<T> GetEnumerator() {
            return _items.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return this.GetEnumerator();
        }

        public IReadOnlyList<T> AsReadOnly() {
            return _items.AsReadOnly();
        }

       
        protected virtual T GetItem(int index) {
            return _items[index];
        }

        protected virtual void SetItem(int index, T value) {
            _items[index] = value;
        }
    }
}