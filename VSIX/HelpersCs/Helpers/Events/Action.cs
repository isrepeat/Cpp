namespace Helpers {
    namespace Events {
        public class Action {
            private bool _isTriggered = false;

            private System.Action? _eventHandler;
            private System.Action? _lastAddedHandler;

            public void Invoke() {
                _isTriggered = true;
                _eventHandler?.Invoke();
            }

            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    _lastAddedHandler.Invoke();
                }
            }

            public static Action operator +(Action handlerContainer, System.Action handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static Action operator -(Action handlerContainer, System.Action handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }

            private void Add(System.Action handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;
            }

            private void Remove(System.Action handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }




        public class Action<T> {
            private bool _isTriggered = false;

            private System.Action<T>? _eventHandler;
            private System.Action<T>? _lastAddedHandler;

            private T? _lastArg;

            public void Invoke(T arg) {
                _isTriggered = true;
                _lastArg = arg;
                _eventHandler?.Invoke(arg);
            }

            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    _lastAddedHandler.Invoke(_lastArg!);
                }
            }

            public static Action<T> operator +(Action<T> handlerContainer, System.Action<T> handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static Action<T> operator -(Action<T> handlerContainer, System.Action<T> handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }

            private void Add(System.Action<T> handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;
            }

            private void Remove(System.Action<T> handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }




        public class Action<T1, T2> {
            private bool _isTriggered = false;

            private System.Action<T1, T2>? _eventHandler;
            private System.Action<T1, T2>? _lastAddedHandler;

            private (T1? arg1, T2? arg2) _lastArgs;

            public void Invoke(T1 arg1, T2 arg2) {
                _isTriggered = true;
                _lastArgs = (arg1, arg2);
                _eventHandler?.Invoke(arg1, arg2);
            }

            public void InvokeForLastHandlerIfTriggered() {
                if (_lastAddedHandler != null && _isTriggered) {
                    _lastAddedHandler.Invoke(_lastArgs.arg1!, _lastArgs.arg2!);
                }
            }

            public static Action<T1, T2> operator +(Action<T1, T2> handlerContainer, System.Action<T1, T2> handler) {
                handlerContainer.Add(handler);
                return handlerContainer;
            }

            public static Action<T1, T2> operator -(Action<T1, T2> handlerContainer, System.Action<T1, T2> handler) {
                handlerContainer.Remove(handler);
                return handlerContainer;
            }

            private void Add(System.Action<T1, T2> handler) {
                _eventHandler += handler;
                _lastAddedHandler = handler;
            }

            private void Remove(System.Action<T1, T2> handler) {
                _eventHandler -= handler;
                if (_lastAddedHandler == handler) {
                    _lastAddedHandler = null;
                }
            }
        }


    }
}