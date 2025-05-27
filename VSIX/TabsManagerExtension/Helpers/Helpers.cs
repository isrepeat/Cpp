using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows;
using System.Windows.Threading;
using System.Collections.Specialized;
using System.Windows.Controls;
using Microsoft.VisualStudio.Shell;

namespace TabsManagerExtension.Helpers {
    public class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null) {
            //if (Equals(field, value)) {
            //    return false;
            //}
            field = value;
            OnPropertyChanged(propertyName);
            return true;
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