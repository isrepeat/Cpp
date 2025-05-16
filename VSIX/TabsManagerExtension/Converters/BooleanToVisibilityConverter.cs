using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TabsManagerExtension.Converters {
    public class BooleanToVisibilityConverter : IValueConverter {
        public bool IsInverted { get; set; } = false;
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture) {
            if (value is bool isVisible) {
                if (IsInverted) {
                    return isVisible ? Visibility.Collapsed : Visibility.Visible;
                }
                return isVisible ? Visibility.Visible : Visibility.Collapsed;
            }
            return Visibility.Collapsed; // By default
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}