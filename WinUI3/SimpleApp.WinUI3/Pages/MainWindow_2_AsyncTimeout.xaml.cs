using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using SimpleApp.WinUI3.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;


namespace SimpleApp.WinUI3 {

    //public static class TaskExtensions {
    //    //public static async Task<TResult> RunWithTimeoutAsync<TResult>(Func<CancellationToken, Task<TResult>> operation, TimeSpan timeout) {
    //    //    using (var cts = new CancellationTokenSource()) {
    //    //        var task = operation(cts.Token);
    //    //        if (task == await Task.WhenAny(task, Task.Delay(timeout, cts.Token))) {
    //    //            return await task; // Задача завершилась до истечения тайм-аута
    //    //        }
    //    //        else {
    //    //            cts.Cancel(); // Отмена задачи
    //    //            throw new TimeoutException("Операция превысила установленное время ожидания.");
    //    //        }
    //    //    }
    //    //}

    //    public static async Task<TResult> TimeoutAfter<TResult>(this Task<TResult> task, TimeSpan timeout) {
    //        if (task == await Task.WhenAny(task, Task.Delay(timeout))) {
    //            return await task; // Метод завершился до истечения тайм-аута
    //        }
    //        else {
    //            throw new TimeoutException("Операция превысила установленное время ожидания.");
    //        }
    //    }
    //}


    public static class TaskExtensions {
        public static async Task TimeoutAfter(this Task task, TimeSpan timeout) {
            if (task == await Task.WhenAny(task, Task.Delay(timeout))) {
                await task; // Метод завершился до истечения тайм-аута
                return;
            }
            else {
                throw new TimeoutException("Операция превысила установленное время ожидания.");
            }
        }
    }

    public sealed partial class MainWindow_2_AsyncTimeout : Window {
        public MainWindow_2_AsyncTimeout() {
            this.InitializeComponent();
            _ = this.InitializeAsync();
        }

        private async Task InitializeAsync() {
            //try {
            //    await LongtimeTaskRoutineA().TimeoutAfter(TimeSpan.FromSeconds(2));
            //}
            //catch (TimeoutException ex) {
            //    System.Diagnostics.Debugger.Break();
            //}

            //await LongtimeTaskRoutineA();
            //await LongtimeTaskRoutineB();

            Diagnostic.Logger.LogDebug("InitializeAsync.1");
            await Task.WhenAll(LongtimeTaskRoutineA(), LongtimeTaskRoutineB());
            Diagnostic.Logger.LogDebug("InitializeAsync.2");
        }

        private async Task LongtimeTaskRoutineA() {
            using var __logFunctionScoped = Diagnostic.Logger.LogFunctionScope("LongtimeTaskRoutineA()");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineA.1");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineA.2");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineA.3");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineA.4");
        }

        private async Task LongtimeTaskRoutineB() {
            using var __logFunctionScoped = Diagnostic.Logger.LogFunctionScope("LongtimeTaskRoutineB()");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineB.1");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineB.2");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineB.3");
            await Task.Delay(1000);
            Diagnostic.Logger.LogDebug("RoutineB.4");
        }

        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}