using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Helpers.Properties {
    public class InvalidatableProperty<T> : Helpers.IInvalidatable {
        private T _value;
        private T _lastValue;

        private bool _isInvalidated = false;
        private bool _isDisposed = false;

        public InvalidatableProperty(T initialValue) {
            _value = initialValue;
            _lastValue = initialValue;
        }

        public T Value {
            get {
                if (_isDisposed) {
                    Helpers.Diagnostic.Logger.LogError($"[Value.get] Property has been disposed {this.ToStringWithType()}");
                    throw new ObjectDisposedException(nameof(InvalidatableProperty<T>));
                }
                if (_isInvalidated) {
                    Helpers.Diagnostic.Logger.LogError($"[Value.get] Property has been invalidated {this.ToStringWithType()}");
                    throw new InvalidOperationException($"[Value.get] Property has been invalidated {this.ToStringWithType()}");
                }
                return _value;
            }
            set {
                if (_isDisposed) {
                    Helpers.Diagnostic.Logger.LogError($"[Value.set] Property has been disposed {this.ToStringWithType()}");
                    throw new ObjectDisposedException(nameof(InvalidatableProperty<T>));
                }
                if (_isInvalidated) {
                    Helpers.Diagnostic.Logger.LogError($"[Value.set] Property has been invalidated {this.ToStringWithType()}");
                    throw new InvalidOperationException($"[Value.set] Property has been invalidated {this.ToStringWithType()}");
                }

                _value = value;
                _lastValue = value;
            }
        }

        public T LastValue {
            get {
                return _lastValue;
            }
        }

        public void Invalidate() {
            if (_isDisposed) {
                return;
            }
            if (_isInvalidated) {
                return;
            }

            foreach (var invalidatable in Helpers.Reflection.GetPropertiesOf<Helpers.IInvalidatable>(_value!)) {
                invalidatable.Invalidate();
            }
            _value = default!;

            _isInvalidated = true;
        }

        public void Dispose() {
            if (_isDisposed) {
                return;
            }

            if (!_isInvalidated) {
                this.Invalidate();
            }

            if (_lastValue is IDisposable disposable) {
                disposable.Dispose();
            }

            _isDisposed = true;
        }

        public override string ToString() {
            if (_isDisposed) {
                return "<disposed>";
            }
            if (_isInvalidated) {
                return $"{{<invalidated> | lastValue = {_lastValue?.ToString() ?? "null"}}}";
            }
            return _value?.ToString() ?? "null";
        }

        public string ToStringWithType() {
            return $"{nameof(InvalidatableProperty<T>)}({this.ToString()})";
        }

        //public static implicit operator T(InvalidatableProperty<T> tracked) {
        //    return tracked.Value;
        //}

        //public static implicit operator InvalidatableProperty<T>(T value) {
        //    return new InvalidatableProperty<T>(value);
        //}
    }
}