using System;
using System.Collections.Generic;

namespace Helpers {
    public static class FocusWatcher {
        // Properties:
        private static readonly FlaggableMetadata _focusMetadata = new();
        public static IMetadata FocusMetadata => _focusMetadata;

        // Internal:
        private class HandlerEntry {
            public WeakReference<object> Owner;
            public Action Callback;

            public HandlerEntry(object owner, Action callback) {
                this.Owner = new WeakReference<object>(owner);
                this.Callback = callback;
            }
        }

        private static readonly Dictionary<string, List<HandlerEntry>> _gotHandlers = new();
        private static readonly Dictionary<string, List<HandlerEntry>> _lostHandlers = new();
        private static readonly Dictionary<string, bool> _focusStates = new();


        public static void RegisterFocusGot(string key, object owner, Action callback) {
            AddHandler(_gotHandlers, key, owner, callback);
        }

        public static void RegisterFocusLost(string key, object owner, Action callback) {
            AddHandler(_lostHandlers, key, owner, callback);
        }

        public static void NotifyFocusGot(string key) {
            _focusStates[key] = true;
            _focusMetadata.SetFlag(key, true);
            CleanupDeadHandlers(_gotHandlers, key);

            if (_gotHandlers.TryGetValue(key, out var list)) {
                foreach (var handler in list) {
                    if (handler.Owner.TryGetTarget(out _)) {
                        handler.Callback?.Invoke();
                    }
                }
            }
        }

        public static void NotifyFocusLost(string key) {
            _focusStates[key] = false;
            _focusMetadata.SetFlag(key, false);
            CleanupDeadHandlers(_lostHandlers, key);

            if (_lostHandlers.TryGetValue(key, out var list)) {
                foreach (var handler in list) {
                    if (handler.Owner.TryGetTarget(out _)) {
                        handler.Callback?.Invoke();
                    }
                }
            }
        }

        public static bool HasFocus(string key) {
            return _focusStates.TryGetValue(key, out bool value) && value;
        }


        private static void AddHandler(Dictionary<string, List<HandlerEntry>> dict, string key, object owner, Action callback) {
            if (!dict.TryGetValue(key, out var list)) {
                list = new List<HandlerEntry>();
                dict[key] = list;
            }
            list.Add(new HandlerEntry(owner, callback));
        }

        private static void CleanupDeadHandlers(Dictionary<string, List<HandlerEntry>> dict, string key) {
            if (dict.TryGetValue(key, out var list)) {
                list.RemoveAll(entry => !entry.Owner.TryGetTarget(out _));
            }
        }
    }
}