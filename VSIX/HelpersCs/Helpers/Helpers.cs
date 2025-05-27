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
    public class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;

        private readonly Dictionary<string, bool> _propertyGuards = new();

        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null) {
            if (EqualityComparer<T>.Default.Equals(field, value)) {
                return false;
            }
            field = value;
            this.OnPropertyChanged(propertyName);
            return true;
        }

        protected void SetPropertyWithNotificationAndGuard<T>(
            ref T refProperty,
            T newValue,
            Action<T>? onChanged = null,
            [CallerMemberName] string? propertyName = null
        ) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }

            if (EqualityComparer<T>.Default.Equals(refProperty, newValue)) {
                return;
            }

            if (_propertyGuards.TryGetValue(propertyName, out var inProgress) && inProgress) {
                Helpers.Diagnostic.Logger.LogWarning($"[Property: {propertyName}] Recursive update attempt detected. Operation aborted.");
                return;
            }

            _propertyGuards[propertyName] = true;

            try {
                refProperty = newValue;
                this.OnPropertyChanged(propertyName);
                onChanged?.Invoke(newValue);
                _propertyGuards[propertyName] = false;
            }
            catch {
                Helpers.Diagnostic.Logger.LogError($"[Property: {propertyName}] Exception occurred during property update.");
                throw;
            }
        }
    }


    public static class Time {
        public static void RunWithDelay(TimeSpan delay, Action action) {
            var timer = new DispatcherTimer { Interval = delay };
            timer.Tick += (s, e) => {
                timer.Stop();
                action();
            };
            timer.Start();
        }
    }

    //public static class UIDispatcher {
    //    public static void Run(Action action) {
    //        if (IsVsixEnvironment()) {
    //            ThreadHelper.JoinableTaskFactory.RunAsync(async () =>
    //            {
    //                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
    //                action();
    //            });
    //        }
    //        else {
    //            Application.Current?.Dispatcher.InvokeAsync(action);
    //        }
    //    }

    //    public static Task RunAsync(Action action) {
    //        if (IsVsixEnvironment()) {
    //            return ThreadHelper.JoinableTaskFactory.RunAsync(async () =>
    //            {
    //                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
    //                action();
    //            }).Task;
    //        }
    //        else {
    //            return Application.Current?.Dispatcher.InvokeAsync(action).Task
    //                   ?? Task.Run(action);
    //        }
    //    }

    //    private static bool IsVsixEnvironment() {
    //        // ThreadHelper works only in VS context, safe test:
    //        return ThreadHelper.JoinableTaskFactory?.Context != null;
    //    }
    //}
}