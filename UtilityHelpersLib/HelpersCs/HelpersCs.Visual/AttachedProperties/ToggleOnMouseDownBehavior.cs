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
using System.Windows.Controls;
using System.Windows.Input;
using System.Reflection;

namespace Helpers {
    // TODO: make universal (now supports only check box)
    public static class ToggleOnMouseDownBehavior {
        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.RegisterAttached(
                "IsEnabled",
                typeof(bool),
                typeof(ToggleOnMouseDownBehavior),
                new PropertyMetadata(false, OnIsEnabledChanged));

        public static void SetIsEnabled(DependencyObject element, bool value) {
            element.SetValue(IsEnabledProperty, value);
        }

        public static bool GetIsEnabled(DependencyObject element) {
            return (bool)element.GetValue(IsEnabledProperty);
        }

        private static void OnIsEnabledChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            if (d is UIElement uiElement) {
                if ((bool)e.NewValue) {
                    uiElement.PreviewMouseLeftButtonDown += OnPreviewMouseLeftButtonDown;
                }
                else {
                    uiElement.PreviewMouseLeftButtonDown -= OnPreviewMouseLeftButtonDown;
                }
            }
        }

        private static void OnPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e) {
            var fe = sender as FrameworkElement;
            if (fe == null) {
                return;
            }

            if (fe is CheckBox checkBox) {
                var prevValue = checkBox.IsChecked;
                checkBox.IsChecked = !prevValue;
            }
        }
    }
}