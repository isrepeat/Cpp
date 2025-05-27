using System.Collections.Generic;
using System.Windows.Media;
using System.Windows;

namespace TabsManagerExtension.Helpers {
    public static class VisualTree {
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

        // Универсальный метод поиска родителя
        public static T FindParent<T>(DependencyObject child) where T : DependencyObject {
            while (child != null) {
                if (child is T parent) {
                    return parent;
                }
                child = VisualTreeHelper.GetParent(child);
            }
            return null;
        }
    }


    namespace Ex {
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
}