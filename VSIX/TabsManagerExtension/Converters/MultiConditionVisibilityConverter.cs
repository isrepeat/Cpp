using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TabsManagerExtension.Converters {
    public class MultiConditionVisibilityConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (!(value is IComparable comparableValue) || !(parameter is string conditions))
                return Visibility.Collapsed;

            var parts = conditions.Split(';');
            foreach (var part in parts) {
                var conditionResult = part.Split(':');
                if (conditionResult.Length != 2) continue;

                var condition = conditionResult[0].Trim();
                var result = conditionResult[1].Trim();

                if (CheckCondition(comparableValue, condition)) {
                    return result.Equals("Visible", StringComparison.OrdinalIgnoreCase) ?
                           Visibility.Visible :
                           Visibility.Collapsed;
                }
            }

            return Visibility.Collapsed;
        }

        private bool CheckCondition(IComparable value, string condition) {
            if (condition == "*") return true;

            if (condition.StartsWith(">=") && int.TryParse(condition.Substring(2), out var minEqual))
                return value.CompareTo(minEqual) >= 0;
            if (condition.StartsWith("<=") && int.TryParse(condition.Substring(2), out var maxEqual))
                return value.CompareTo(maxEqual) <= 0;
            if (condition.StartsWith(">") && int.TryParse(condition.Substring(1), out var min))
                return value.CompareTo(min) > 0;
            if (condition.StartsWith("<") && int.TryParse(condition.Substring(1), out var max))
                return value.CompareTo(max) < 0;
            if (condition.StartsWith("=") && int.TryParse(condition.Substring(1), out var equal))
                return value.CompareTo(equal) == 0;

            return false;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }

    public class MultiConditionBoolConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (!(value is IComparable comparableValue) || !(parameter is string conditions))
                return false;

            var parts = conditions.Split(';');
            foreach (var part in parts) {
                var conditionResult = part.Split(':');
                if (conditionResult.Length != 2) continue;

                var condition = conditionResult[0].Trim();
                var result = conditionResult[1].Trim();

                if (CheckCondition(comparableValue, condition)) {
                    return result.Equals("true", StringComparison.OrdinalIgnoreCase);
                }
            }

            return false;
        }

        private bool CheckCondition(IComparable value, string condition) {
            if (condition == "*") return true;

            if (condition.StartsWith(">=") && int.TryParse(condition.Substring(2), out var minEqual))
                return value.CompareTo(minEqual) >= 0;
            if (condition.StartsWith("<=") && int.TryParse(condition.Substring(2), out var maxEqual))
                return value.CompareTo(maxEqual) <= 0;
            if (condition.StartsWith(">") && int.TryParse(condition.Substring(1), out var min))
                return value.CompareTo(min) > 0;
            if (condition.StartsWith("<") && int.TryParse(condition.Substring(1), out var max))
                return value.CompareTo(max) < 0;
            if (condition.StartsWith("=") && int.TryParse(condition.Substring(1), out var equal))
                return value.CompareTo(equal) == 0;

            return false;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}
