using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Media;

namespace Helpers {
    public static class AttachedProperties {

        public static readonly DependencyProperty HostControlProperty =
            DependencyProperty.RegisterAttached(
                "HostControl",
                typeof(object),
                typeof(AttachedProperties),
                new PropertyMetadata(null));

        public static void SetHostControl(DependencyObject element, object value) {
            element.SetValue(HostControlProperty, value);
        }

        public static object? GetHostControl(DependencyObject element) {
            return element.GetValue(HostControlProperty);
        }

        // 🌟 Автоматическая привязка
        public static readonly DependencyProperty AutoAttachRootControlProperty =
            DependencyProperty.RegisterAttached(
                "AutoAttachRootControl",
                typeof(bool),
                typeof(AttachedProperties),
                new PropertyMetadata(false, OnAutoAttachRootControlChanged));

        public static void SetAutoAttachRootControl(DependencyObject element, bool value) {
            element.SetValue(AutoAttachRootControlProperty, value);
        }

        public static bool GetAutoAttachRootControl(DependencyObject element) {
            return (bool)element.GetValue(AutoAttachRootControlProperty);
        }

        private static void OnAutoAttachRootControlChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            if (d is FrameworkElement fe && (bool)e.NewValue) {
                fe.Loaded += (_, __) => {
                    if (GetHostControl(fe) != null) {
                        return; // уже задан
                    }

                    var root = FindNearestRootControl(fe);
                    if (root != null) {
                        SetHostControl(fe, root);
                    }
                };
            }
        }

        private static object? FindNearestRootControl(DependencyObject start) {
            var current = start;
            while (current != null) {
                var val = GetHostControl(current);
                if (val != null) {
                    return val;
                }

                current = LogicalTreeHelper.GetParent(current) ?? VisualTreeHelper.GetParent(current);
            }
            return null;
        }
    }
}