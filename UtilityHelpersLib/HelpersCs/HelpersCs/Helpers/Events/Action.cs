using System.Linq;
using System.Collections.Generic;


namespace Helpers {
    namespace Events {
        // interface marker for Reflection
        public interface IDisposableEvent : System.IDisposable {
        }


        public class Action {
            [System.Flags]
            public enum Options {
                None = 0,
                UnsubscribeAfterInvoked = 1 << 0
            }

            private readonly List<System.Action> _handlers = new();
            private readonly HashSet<System.Action> _invokedHandlers = new();
            private readonly HashSet<System.Action> _autoUnsubscribeHandlers = new();

            private System.Action? _lastAddedHandler;
            private bool _isTriggered = false;

            public void Invoke() {
                _isTriggered = true;

                foreach (var handler in _handlers.ToArray()) {
                    handler();
                    _invokedHandlers.Add(handler);

                    if (_autoUnsubscribeHandlers.Contains(handler)) {
                        this.Remove(handler);
                    }
                }
            }

            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    if (!_invokedHandlers.Contains(_lastAddedHandler)) {
                        _lastAddedHandler.Invoke();
                        _invokedHandlers.Add(_lastAddedHandler);

                        if (_autoUnsubscribeHandlers.Contains(_lastAddedHandler)) {
                            this.Remove(_lastAddedHandler);
                        }
                    }
                    else {
                        System.Diagnostics.Debugger.Break();
                    }
                }
            }

            public void Add(System.Action handler) {
                this.Add(Options.None, handler);
            }

            public void Add(Options options, System.Action handler) {
                _handlers.Add(handler);
                _lastAddedHandler = handler;

                if ((options.HasFlag(Action.Options.UnsubscribeAfterInvoked))) {
                    _autoUnsubscribeHandlers.Add(handler);
                }
            }

            public void Remove(System.Action handler) {
                _handlers.Remove(handler);
                _invokedHandlers.Remove(handler);
                _autoUnsubscribeHandlers.Remove(handler);

                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }



        //
        // Action<T>
        //
        // NOTE: '_invokedHandlers' хранит информацию о том, какие хендлеры уже вызывались через Invoke(...).
        //       Это нужно для того чтобы случайно не вызывать повторно хендлер с тем же _lastArg,
        //       если этот хендлер уже был вызван через обычный Invoke. Иначе могут быть побочные еффекты,
        //       если 'T? _lastArg' содержит не примитивные типы.
        public class Action<T> {
            private readonly List<System.Action<T>> _handlers = new();
            private readonly HashSet<System.Action<T>> _invokedHandlers = new();
            private readonly HashSet<System.Action<T>> _autoUnsubscribeHandlers = new();

            private System.Action<T>? _lastAddedHandler;
            private T? _lastArg;
            private bool _isTriggered = false;

            /// <summary>
            /// Вызывает всех подписчиков (включая lastAddedHandler, если он зарегистрирован),
            /// сохраняет последний аргумент и помечает, что событие было вызвано.
            /// </summary>
            public void Invoke(T arg) {
                _isTriggered = true;
                _lastArg = arg;

                foreach (var handler in _handlers.ToArray()) {
                    handler(arg);
                    _invokedHandlers.Add(handler);

                    if (_autoUnsubscribeHandlers.Contains(handler)) {
                        this.Remove(handler);
                    }
                }
            }

            /// <summary>
            /// Если событие уже было вызвано, а последний добавленный хендлер ещё не вызывался —
            /// вызывает только его. После вызова сбрасывает флаг и аргумент.
            /// </summary>
            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    if (!_invokedHandlers.Contains(_lastAddedHandler)) {
                        _lastAddedHandler.Invoke(_lastArg!);
                        _invokedHandlers.Add(_lastAddedHandler);

                        if (_autoUnsubscribeHandlers.Contains(_lastAddedHandler)) {
                            this.Remove(_lastAddedHandler);
                        }
                    }
                    else {
                        System.Diagnostics.Debugger.Break();
                    }
                }
            }


            public void Add(System.Action<T> handler) {
                this.Add(Action.Options.None, handler);
            }

            public void Add(Action.Options options, System.Action<T> handler) {
                _handlers.Add(handler);
                _lastAddedHandler = handler;

                if ((options.HasFlag(Action.Options.UnsubscribeAfterInvoked))) {
                    _autoUnsubscribeHandlers.Add(handler);
                }
            }

            public void Remove(System.Action<T> handler) {
                _handlers.Remove(handler);
                _invokedHandlers.Remove(handler);
                _autoUnsubscribeHandlers.Remove(handler);

                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }


        //
        // Action<T1, T2>
        //
        public class Action<T1, T2> {
            private readonly List<System.Action<T1, T2>> _handlers = new();
            private readonly HashSet<System.Action<T1, T2>> _invokedHandlers = new();
            private readonly HashSet<System.Action<T1, T2>> _autoUnsubscribeHandlers = new();

            private System.Action<T1, T2>? _lastAddedHandler;
            private (T1?, T2?) _lastArgs;
            private bool _isTriggered = false;

            public void Invoke(T1 arg1, T2 arg2) {
                _isTriggered = true;
                _lastArgs = (arg1, arg2);

                foreach (var handler in _handlers.ToArray()) {
                    handler(arg1, arg2);
                    _invokedHandlers.Add(handler);

                    if (_autoUnsubscribeHandlers.Contains(handler)) {
                        this.Remove(handler);
                    }
                }
            }

            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    if (!_invokedHandlers.Contains(_lastAddedHandler)) {
                        _lastAddedHandler.Invoke(_lastArgs.Item1!, _lastArgs.Item2!);
                        _invokedHandlers.Add(_lastAddedHandler);

                        if (_autoUnsubscribeHandlers.Contains(_lastAddedHandler)) {
                            this.Remove(_lastAddedHandler);
                        }
                    }
                    else {
                        System.Diagnostics.Debugger.Break();
                    }
                }
            }


            public void Add(System.Action<T1, T2> handler) {
                this.Add(Action.Options.None, handler);
            }

            public void Add(Action.Options options, System.Action<T1, T2> handler) {
                _handlers.Add(handler);
                _lastAddedHandler = handler;

                if ((options.HasFlag(Action.Options.UnsubscribeAfterInvoked))) {
                    _autoUnsubscribeHandlers.Add(handler);
                }
            }

            public void Remove(System.Action<T1, T2> handler) {
                _handlers.Remove(handler);
                _invokedHandlers.Remove(handler);
                _autoUnsubscribeHandlers.Remove(handler);

                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }
    }
}