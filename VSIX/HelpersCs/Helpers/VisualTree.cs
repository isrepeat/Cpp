using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Controls;

namespace Helpers {
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

        public static T? FindParentOfType<T>(DependencyObject root) where T : DependencyObject {
            DependencyObject? current = VisualTreeHelper.GetParent(root);

            while (current != null) {
                if (current is T match) {
                    return match;
                }
                current = VisualTreeHelper.GetParent(current);
            }
            return null;
        }


        public static FrameworkElement? FindElementByName(DependencyObject root, string name) {
            if (root is FrameworkElement fe && fe.Name == name) {
                return fe;
            }

            int count = VisualTreeHelper.GetChildrenCount(root);
            for (int i = 0; i < count; i++) {
                var child = VisualTreeHelper.GetChild(root, i);
                var result = FindElementByName(child, name);
                if (result != null) {
                    return result;
                }
            }

            return null;
        }


        public static DependencyObject? FindDescendantByType(DependencyObject root, string typeName) {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++) {
                var child = VisualTreeHelper.GetChild(root, i);
                if (child.GetType().Name == typeName) {
                    return child;
                }

                var result = FindDescendantByType(child, typeName);
                if (result != null) {
                    return result;
                }
            }

            return null;
        }

        public static T? FindDescendantByType<T>(DependencyObject root) where T : DependencyObject {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++) {
                var child = VisualTreeHelper.GetChild(root, i);

                if (child is T match) {
                    return match;
                }

                var result = FindDescendantByType<T>(child);
                if (result != null) {
                    return result;
                }
            }

            return null;
        }





        public static void LogDescendantsRecursive(DependencyObject element, int depth, Action<FrameworkElement, int>? extraLogger) {
            if (element is FrameworkElement fe) {
                string indent = new string(' ', depth * 2);
                string typeName = fe.GetType().Name;
                string name = !string.IsNullOrEmpty(fe.Name) ? $" Name='{fe.Name}'" : "";
                string visibility = $" Visibility={fe.Visibility}";

                Helpers.Diagnostic.Logger.LogDebug($"{indent}- [{depth}] {typeName}{name}{visibility}");

                extraLogger?.Invoke(fe, depth);
            }

            int count = VisualTreeHelper.GetChildrenCount(element);
            for (int i = 0; i < count; i++) {
                var child = VisualTreeHelper.GetChild(element, i);
                LogDescendantsRecursive(child, depth + 1, extraLogger);
            }
        }

        public static void DumpVisualTree(DependencyObject parent, int indent = 0) {
            if (parent == null) return;

            string prefix = new string(' ', indent * 2);
            string name = (parent as FrameworkElement)?.Name ?? "(no name)";
            string type = parent.GetType().Name;

            Helpers.Diagnostic.Logger.LogDebug($"{prefix}{type}  [{name}]");

            int count = VisualTreeHelper.GetChildrenCount(parent);
            for (int i = 0; i < count; i++) {
                DumpVisualTree(VisualTreeHelper.GetChild(parent, i), indent + 1);
            }
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