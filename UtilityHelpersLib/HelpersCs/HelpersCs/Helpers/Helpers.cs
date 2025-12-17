using System;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;


namespace Helpers {
    public class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        private readonly Dictionary<string, bool> _propertyGuards = new();

        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null) {
            if (EqualityComparer<T>.Default.Equals(field, value)) {
                return false;
            }
            field = value;
            this.OnPropertyChanged(propertyName);
            return true;
        }

        protected void SetPropertyWithNotification<T>(
            ref T refProperty,
            T newValue,
            System.Action<T>? onChanged = null,
            [CallerMemberName] string? propertyName = null) {

            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }

            if (EqualityComparer<T>.Default.Equals(refProperty, newValue)) {
                return;
            }

            if (_propertyGuards.TryGetValue(propertyName, out var inProgress) && inProgress) {
                Helpers.Diagnostic.Logger.LogWarning($"[Property: {propertyName}] Recursive update attempt detected. Operation aborted.");
                return;
            }

            _propertyGuards[propertyName] = true;

            try {
                refProperty = newValue;
                this.OnPropertyChanged(propertyName);

                onChanged?.Invoke(newValue);

                _propertyGuards[propertyName] = false;
            }
            catch {
                Helpers.Diagnostic.Logger.LogError($"[Property: {propertyName}] Exception occurred during property update.");
                throw;
            }
        }

        /// <summary>
        /// Обновляет свойство, уведомляет об изменении и откладывает новое значение в список для дальнейшей обработки.
        /// </summary>
        protected void SetPropertyWithDeferredNotificationValues<T>(
            ref T refProperty,
            T newValue,
            List<T> pendingValues,
            [CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }

            if (EqualityComparer<T>.Default.Equals(refProperty, newValue)) {
                return;
            }

            if (_propertyGuards.TryGetValue(propertyName, out var inProgress) && inProgress) {
                Helpers.Diagnostic.Logger.LogWarning($"[Property: {propertyName}] Recursive update attempt detected. Operation aborted.");
                return;
            }

            _propertyGuards[propertyName] = true;

            try {
                refProperty = newValue;
                this.OnPropertyChanged(propertyName);

                pendingValues.Add(newValue); // сохраняем значение для последующей обработки

                _propertyGuards[propertyName] = false;
            }
            catch {
                Helpers.Diagnostic.Logger.LogError($"[Property: {propertyName}] Exception occurred during property update.");
                throw;
            }
        }
    }


    public static class Reflection {
        public static IEnumerable<TInterface> GetPropertiesOf<TInterface>(object instance) {
            if (instance == null) {
                throw new ArgumentNullException(nameof(instance));
            }

            var type = instance.GetType();

            var properties = type.GetProperties(
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.Public |
                System.Reflection.BindingFlags.NonPublic);

            foreach (var prop in properties) {
                if (!typeof(TInterface).IsAssignableFrom(prop.PropertyType)) {
                    continue;
                }

                var value = prop.GetValue(instance);
                if (value is TInterface casted) {
                    yield return casted;
                }
            }
        }
    }
}