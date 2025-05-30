using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    public static class GlobalFlags {
        // Properties:
        private static readonly FlaggableMetadata _globalMetadata = new();
        public static IMetadata Metadata => _globalMetadata;

        // Internal:
        private class HandlerEntry {
            public WeakReference<object> Owner;
            public Action Callback;

            public HandlerEntry(object owner, Action callback) {
                this.Owner = new WeakReference<object>(owner);
                this.Callback = callback;
            }
        }

        private static readonly Dictionary<string, List<HandlerEntry>> _setHandlers = new();
        private static readonly Dictionary<string, List<HandlerEntry>> _unsetHandlers = new();
        private static readonly Dictionary<string, bool> _flagStates = new();


        public static void RegisterFlagSet(string key, object owner, Action callback) {
            AddHandler(_setHandlers, key, owner, callback);
        }

        public static void RegisterFlagUnset(string key, object owner, Action callback) {
            AddHandler(_unsetHandlers, key, owner, callback);
        }

        public static void SetFlag(string key, bool value) {
            _flagStates[key] = value;
            _globalMetadata.SetFlag(key, value);

            if (value) {
                Notify(_setHandlers, key);
            }
            else {
                Notify(_unsetHandlers, key);
            }
        }

        public static bool IsSet(string key) {
            return _flagStates.TryGetValue(key, out bool value) && value;
        }


        private static void AddHandler(Dictionary<string, List<HandlerEntry>> dict, string key, object owner, Action callback) {
            if (!dict.TryGetValue(key, out var list)) {
                list = new List<HandlerEntry>();
                dict[key] = list;
            }
            list.Add(new HandlerEntry(owner, callback));
        }

        private static void Notify(Dictionary<string, List<HandlerEntry>> dict, string key) {
            CleanupDeadHandlers(dict, key);

            if (dict.TryGetValue(key, out var list)) {
                foreach (var handler in list) {
                    if (handler.Owner.TryGetTarget(out _)) {
                        handler.Callback?.Invoke();
                    }
                }
            }
        }

        private static void CleanupDeadHandlers(Dictionary<string, List<HandlerEntry>> dict, string key) {
            if (dict.TryGetValue(key, out var list)) {
                list.RemoveAll(entry => !entry.Owner.TryGetTarget(out _));
            }
        }
    }
}