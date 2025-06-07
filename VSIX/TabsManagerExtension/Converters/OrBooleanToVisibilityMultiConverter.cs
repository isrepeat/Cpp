using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TabsManagerExtension.Converters {
    public class OrBooleanToVisibilityMultiConverter : IMultiValueConverter {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture) {
            foreach (var value in values) {
                if (value is bool b && b) {
                    return Visibility.Visible;
                }
            }
            return Visibility.Collapsed;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture) {
            throw new NotSupportedException();
        }
    }
}