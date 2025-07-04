using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    namespace Events {
        public class TriggeredAction {
            private bool _isTriggered = false;
            private Action? _eventHandler;
            private Action? _lastAddedHandler;

            public void Add(Action handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;

                if (_isTriggered) {
                    handler.Invoke();
                }
            }

            public void Remove(Action handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }

            public void Invoke() {
                _isTriggered = true;
                _eventHandler?.Invoke();
            }

            public static TriggeredAction operator +(TriggeredAction handlerContainer, Action handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static TriggeredAction operator -(TriggeredAction handlerContainer, Action handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }
        }




        /// <summary>
        /// Все "TriggeredAction<T1, T2, ...>" основаны на логике TriggeredAction<T1>.
        /// При изменении алгоритма в TriggeredAction<T1>(например флагов _isTriggered или _isInvoked,
        /// а также TryInvokeInternal) необходимо аналогично адаптировать все остальные варианты.
        /// </summary>
        public class TriggeredAction<T1> {
            private bool _isTriggered = false;
            private T1? _lastArg;

            private Action<T1>? _eventHandler;
            private Action<T1>? _lastAddedHandler;

            public void Add(Action<T1> handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;

                if (_isTriggered) {
                    handler.Invoke(_lastArg!);
                }
            }

            public void Remove(Action<T1> handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }

            public void Invoke(T1 arg1) {
                _isTriggered = true;
                _lastArg = arg1;
                _eventHandler?.Invoke(arg1);
            }

            public static TriggeredAction<T1> operator +(TriggeredAction<T1> handlerContainer, Action<T1> handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static TriggeredAction<T1> operator -(TriggeredAction<T1> handlerContainer, Action<T1> handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }
        }



        // <T1, T2>
        public class TriggeredAction<T1, T2> {
            private bool _isTriggered = false;
            private (T1, T2)? _lastArgs;

            private Action<T1, T2>? _eventHandler;
            private Action<T1, T2>? _lastAddedHandler;

            public void Add(Action<T1, T2> handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;

                if (_isTriggered) {
                    handler.Invoke(_lastArgs!.Value.Item1, _lastArgs!.Value.Item2);
                }
            }

            public void Remove(Action<T1, T2> handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }

            public void Invoke(T1 arg1, T2 arg2) {
                _isTriggered = true;
                _lastArgs = (arg1, arg2);
                _eventHandler?.Invoke(arg1, arg2);
            }

            public static TriggeredAction<T1, T2> operator +(TriggeredAction<T1, T2> handlerContainer, Action<T1, T2> handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static TriggeredAction<T1, T2> operator -(TriggeredAction<T1, T2> handlerContainer, Action<T1, T2> handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }
        }


        // <T1, T2, T3>
        public class TriggeredAction<T1, T2, T3> {
            private bool _isTriggered = false;
            private (T1, T2, T3)? _lastArgs;

            private Action<T1, T2, T3>? _eventHandler;
            private Action<T1, T2, T3>? _lastAddedHandler;

            public void Add(Action<T1, T2, T3> handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;

                if (_isTriggered) {
                    handler.Invoke(_lastArgs!.Value.Item1, _lastArgs!.Value.Item2, _lastArgs!.Value.Item3);
                }
            }

            public void Remove(Action<T1, T2, T3> handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }

            public void Invoke(T1 arg1, T2 arg2, T3 arg3) {
                _isTriggered = true;
                _lastArgs = (arg1, arg2, arg3);
                _eventHandler?.Invoke(arg1, arg2, arg3);
            }

            public static TriggeredAction<T1, T2, T3> operator +(TriggeredAction<T1, T2, T3> handlerContainer, Action<T1, T2, T3> handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static TriggeredAction<T1, T2, T3> operator -(TriggeredAction<T1, T2, T3> handlerContainer, Action<T1, T2, T3> handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }
        }
    }
}