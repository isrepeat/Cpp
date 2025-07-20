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
    public sealed class Variant<T1, T2>
        where T1 : class
        where T2 : class {

        public object? CurrentValue {
            get {
                if (_value1 != null) {
                    return _value1;
                }
                if (_value2 != null) {
                    return _value2;
                }
                return null;
            }
        }

        private T1? _value1;
        private T2? _value2;

        public void Set(T1 value) {
            _value1 = value ?? throw new ArgumentNullException(nameof(value));
            _value2 = null;
        }

        public void Set(T2 value) {
            _value2 = value ?? throw new ArgumentNullException(nameof(value));
            _value1 = null;
        }

        public bool Is<T>() where T : class {
            if (typeof(T) == typeof(T1)) {
                return _value1 != null;
            }
            if (typeof(T) == typeof(T2)) {
                return _value2 != null;
            }
            return false;
        }

        public T Get<T>() where T : class {
            if (typeof(T) == typeof(T1)) {
                return _value1 as T
                    ?? throw new InvalidOperationException($"Stored value for {typeof(T1).Name} is null.");
            }
            if (typeof(T) == typeof(T2)) {
                return _value2 as T
                    ?? throw new InvalidOperationException($"Stored value for {typeof(T2).Name} is null.");
            }
            throw new InvalidOperationException($"Type {typeof(T).Name} is not supported by this variant.");
        }
    }
}