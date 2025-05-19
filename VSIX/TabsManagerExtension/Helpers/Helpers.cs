using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows;

namespace TabsManagerExtension.Helpers {
    public class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null) {
            //if (Equals(field, value)) {
            //    return false;
            //}
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }

    public static class UI {
        // Вспомогательный метод для поиска элементов Visual Tree (рекурсивно)
        public static List<T> FindVisualChildren<T>(DependencyObject depObj) where T : DependencyObject {
            List<T> result = new List<T>();

            if (depObj == null) return result;

            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(depObj); i++) {
                var child = VisualTreeHelper.GetChild(depObj, i);
                if (child is T matchingChild) {
                    result.Add(matchingChild);
                }

                // Рекурсивный вызов для вложенных детей
                result.AddRange(FindVisualChildren<T>(child));
            }

            return result;
        }
    }
}

namespace TabsManagerExtension.Helpers.Ex {
    // TODO: move to extensions file.
    public static class VisualTreeExtensions {
        // Метод расширения для поиска всех потомков указанного типа (универсальный)
        public static IEnumerable<T> GetVisualDescendants<T>(this DependencyObject parent) where T : DependencyObject {
            if (parent == null) yield break;

            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++) {
                var child = VisualTreeHelper.GetChild(parent, i);

                if (child is T typedChild) {
                    yield return typedChild;
                }

                foreach (var descendant in GetVisualDescendants<T>(child)) {
                    yield return descendant;
                }
            }
        }
    }
}