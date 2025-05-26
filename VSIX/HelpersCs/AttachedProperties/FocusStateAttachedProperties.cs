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

namespace Helpers {
    public static class FocusStateAttachedProperties {
        public static readonly DependencyProperty IsParentControlFocusedProperty =
            DependencyProperty.RegisterAttached(
                "IsParentControlFocused",
                typeof(bool),
                typeof(FocusStateAttachedProperties),
                new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.Inherits));

        public static void SetIsParentControlFocused(DependencyObject element, bool value) =>
            element.SetValue(IsParentControlFocusedProperty, value);

        public static bool GetIsParentControlFocused(DependencyObject element) =>
            (bool)element.GetValue(IsParentControlFocusedProperty);
    }
}