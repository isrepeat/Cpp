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


namespace Helpers.Time {
    public class DelayedEventsHandler {
        public System.Action? OnReady { get; set; }

        private readonly DispatcherTimer _timer;
        private readonly object _lock = new();
        private bool _pending;

        public DelayedEventsHandler(TimeSpan interval) {
            _timer = new DispatcherTimer {
                Interval = interval
            };
            _timer.Tick += this.OnTimerTick;
        }

        public void Schedule() {
            lock (_lock) {
                _pending = true;
            }

            _timer.Stop();
            _timer.Start();
        }

        public void Clear() {
            lock (_lock) {
                _pending = false;
            }
        }

        private void OnTimerTick(object? sender, EventArgs e) {
            _timer.Stop();

            bool execute;
            lock (_lock) {
                execute = _pending;
                _pending = false;
            }

            if (execute) {
                this.OnReady?.Invoke();
            }
        }
    }



    public class DelayedEventsHandler<T> {
        public System.Action<List<T>>? OnReady { get; set; }

        private readonly DispatcherTimer _timer;
        private readonly List<T> _pendingItems = new();
        private readonly object _lock = new();

        public DelayedEventsHandler(TimeSpan interval) {
            _timer = new DispatcherTimer {
                Interval = interval
            };
            _timer.Tick += this.OnTimerTick;
        }

        public void Schedule(T item) {
            lock (_lock) {
                _pendingItems.Add(item);
            }

            _timer.Stop();
            _timer.Start();
        }

        public void Clear() {
            lock (_lock) {
                _pendingItems.Clear();
            }
        }

        private void OnTimerTick(object? sender, EventArgs e) {
            _timer.Stop();

            List<T> batch;
            lock (_lock) {
                batch = _pendingItems.ToList();
                _pendingItems.Clear();
            }

            this.OnReady?.Invoke(batch);
        }
    }



    public class DelayedEventsHandler<T, TEnum, TConfig> : IDisposable
        where TEnum : struct, Enum
        where TConfig : ITimerConfig<TEnum>, new() {

        public System.Action<List<T>>? OnEventsReady { get; set; }

        private readonly List<T> _pendingItems = new();
        private readonly object _lock = new();
        private readonly WeakReference<TimerManager<TConfig, TEnum>> _timerManager;
        private readonly TEnum _timeSlot;
        private bool _running;
        private bool _disposed;

        public DelayedEventsHandler(TimerManager<TConfig, TEnum> timerManager, TEnum timeSlot) {
            _timerManager = new WeakReference<TimerManager<TConfig, TEnum>>(timerManager);
            _timeSlot = timeSlot;

            timerManager.Subscribe(_timeSlot, this.OnTimerTick);
        }

        public void Dispose() {
            if (_disposed) {
                return;
            }

            this.Clear();

            if (_timerManager.TryGetTarget(out var timer)) {
                timer.Unsubscribe(_timeSlot, this.OnTimerTick);
            }

            _disposed = true;
        }


        public void Schedule(T item) {
            lock (_lock) {
                _pendingItems.Add(item);
                _running = true;
            }
        }


        public void Clear() {
            lock (_lock) {
                _pendingItems.Clear();
                _running = false;
            }
        }


        private void OnTimerTick() {
            bool execute = false;

            lock (_lock) {
                if (_running) {
                    execute = true;
                    _running = false;
                }
            }

            if (execute) {
                List<T> batch;
                lock (_lock) {
                    batch = _pendingItems.ToList();
                    _pendingItems.Clear();
                }

                this.OnEventsReady?.Invoke(batch);
            }
        }
    }
}