using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TabsManagerExtension.Converters {
    public class MultiConditionVisibilityConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null || parameter is not string conditions)
                return Visibility.Collapsed;

            var parts = conditions.Split(';');
            foreach (var part in parts) {
                var conditionResult = part.Split(':');
                if (conditionResult.Length != 2) continue;

                var condition = conditionResult[0].Trim();
                var result = conditionResult[1].Trim();

                if (CheckCondition(value, condition)) {
                    return result.Equals("Visible", StringComparison.OrdinalIgnoreCase)
                        ? Visibility.Visible
                        : Visibility.Collapsed;
                }
            }

            return Visibility.Collapsed;
        }

        private bool CheckCondition(object value, string condition) {
            if (condition == "*") return true;

            // Попробуем сначала сравнение как числа
            if (value is IComparable comparableValue && int.TryParse(value.ToString(), out var valAsInt)) {
                if (condition.StartsWith(">=") && int.TryParse(condition.Substring(2), out var minEq))
                    return valAsInt >= minEq;
                if (condition.StartsWith("<=") && int.TryParse(condition.Substring(2), out var maxEq))
                    return valAsInt <= maxEq;
                if (condition.StartsWith(">") && int.TryParse(condition.Substring(1), out var min))
                    return valAsInt > min;
                if (condition.StartsWith("<") && int.TryParse(condition.Substring(1), out var max))
                    return valAsInt < max;
                if (condition.StartsWith("==") && int.TryParse(condition.Substring(1), out var eq))
                    return valAsInt == eq;
            }

            // Сравнение как строк
            var str = value.ToString();
            if (condition.StartsWith("=="))
                return string.Equals(str, condition.Substring(2), StringComparison.OrdinalIgnoreCase);
            if (condition.StartsWith("!="))
                return !string.Equals(str, condition.Substring(2), StringComparison.OrdinalIgnoreCase);

            return false;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}