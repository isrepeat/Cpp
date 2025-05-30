using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Helpers {
    public interface IMetadata : INotifyPropertyChanged {
        void SetFlag(string key, bool value);
        bool GetFlag(string key);
        bool this[string key] { get; }
    }

    public class FlaggableMetadata : IMetadata {
        private readonly Dictionary<string, bool> _flags = new();

        public bool this[string key] => this.GetFlag(key);

        public bool GetFlag(string key) {
            return _flags.TryGetValue(key, out var value) && value;
        }

        public void SetFlag(string key, bool value) {
            if (!_flags.TryGetValue(key, out var oldValue) || oldValue != value) {
                _flags[key] = value;
                this.OnPropertyChanged($"Item[]");
            }
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}