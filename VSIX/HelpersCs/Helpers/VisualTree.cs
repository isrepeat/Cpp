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


        public static FrameworkElement? FindParentByName(DependencyObject child, string targetName) {
            var current = VisualTreeHelper.GetParent(child);
            while (current != null) {
                if (current is FrameworkElement fe && fe.Name == targetName) {
                    return fe;
                }
                current = VisualTreeHelper.GetParent(current);
            }
            return null;
        }


        public static T? FindParentByType<T>(DependencyObject root) where T : DependencyObject {
            DependencyObject? current = VisualTreeHelper.GetParent(root);

            while (current != null) {
                if (current is T match) {
                    return match;
                }
                current = VisualTreeHelper.GetParent(current);
            }
            return null;
        }


        public static DependencyObject? FindChildByType(DependencyObject root, string typeName) {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++) {
                var child = VisualTreeHelper.GetChild(root, i);
                if (child.GetType().Name == typeName) {
                    return child;
                }

                var result = FindChildByType(child, typeName);
                if (result != null) {
                    return result;
                }
            }

            return null;
        }


        public static T? FindChildByType<T>(DependencyObject root) where T : DependencyObject {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++) {
                var child = VisualTreeHelper.GetChild(root, i);

                if (child is T match) {
                    return match;
                }

                var result = FindChildByType<T>(child);
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
            public static IEnumerable<T> ex_GetVisualDescendants<T>(this DependencyObject parent) where T : DependencyObject {
                if (parent == null) yield break;

                for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++) {
                    var child = VisualTreeHelper.GetChild(parent, i);

                    if (child is T typedChild) {
                        yield return typedChild;
                    }

                    foreach (var descendant in ex_GetVisualDescendants<T>(child)) {
                        yield return descendant;
                    }
                }
            }
        }


        public static class UserControlExtensions {
            /// <summary>
            /// Проверяет, находится ли на пути от заданного дочернего элемента до текущего UserControl
            /// хотя бы один интерактивный элемент управления (Focusable и Enabled Control).
            ///
            /// Используется для фильтрации событий ввода: например, чтобы отличить клики по кнопкам, чекбоксам и другим
            /// активным UI-элементам от кликов по фоновым областям.
            /// </summary>
            /// <param name="control">UserControl, выступающий верхней границей поиска.</param>
            /// <param name="startingElement">Исходный элемент, от которого начинается восходящий поиск.</param>
            /// <returns>True, если хотя бы один интерактивный элемент найден на пути вверх до UserControl включительно.</returns>
            public static bool ex_HasInteractiveElementOnPathFrom(this UserControl control, DependencyObject startingElement) {
                var current = startingElement;

                while (current != null) {
                    // Любой UIElement с фокусом или с поддержкой ввода
                    if (current is UIElement uiElement) {
                        // Control уже покрыт (Control наследуется от UIElement),
                        // но также сюда попадёт TextBlock и т.п.
                        if (uiElement.Focusable && uiElement.IsEnabled) {
                            return true;
                        }
                    }

                    // Иногда ContentElement не является UIElement'ом (например, Run)
                    if (current is ContentElement contentElement && contentElement.Focusable) {
                        return true;
                    }

                    if (current == control) {
                        break;
                    }

                    current = VisualTreeHelper.GetParent(current);
                }

                return false;
            }
        }
    }
}