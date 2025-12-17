using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace Helpers.Properties {
    public class ObservableProperty<T> : Helpers.ObservableObject {
        private T _value;
        public T Value {
            get => _value;
            set {
                if (!object.Equals(_value, value)) {
                    _value = value;
                    base.OnPropertyChanged();
                }
            }
        }

        public ObservableProperty(T initialValue) {
            _value = initialValue;
        }

        public T Get() {
            return this.Value;
        }
        public void Set(T value) {
            this.Value = value;
        }
    }


    public sealed class BoolProperty : ObservableProperty<bool> {
        public BoolProperty(bool initialValue) : base(initialValue) {
        }
    }
}