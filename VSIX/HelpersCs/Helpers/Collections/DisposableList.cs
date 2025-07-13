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


namespace Helpers.Collections {
    public sealed class DisposableList<T> : List<T> where T : IDisposable {
        public new void Clear() {
            foreach (var item in this) {
                item.Dispose();
            }
            base.Clear();
        }

        public new bool Remove(T item) {
            if (base.Remove(item)) {
                item.Dispose();
                return true;
            }
            return false;
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