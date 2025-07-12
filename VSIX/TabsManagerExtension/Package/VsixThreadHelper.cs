using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.ComponentModel.Design;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.TextManager.Interop;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension {
    public static class VsixThreadHelper {
        /// <summary>
        /// Выполняет action на STA COM UI потоке Visual Studio без ожидания завершения (fire-and-forget).
        /// Можно безопасно использовать DTE, IVsSolution и т.д.
        /// </summary>
        public static void RunOnVsThread(Action action) {
            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                try {
                    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                    action();
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"[RunOnVsThread] action exception: {ex}");
                    System.Diagnostics.Debugger.Break();
                    throw;
                }
            });
        }

        public static void RunOnVsThread(Func<Task> asyncAction) {
            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                try {
                    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                    await asyncAction();
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"[RunOnVsThread] asyncAction exception: {ex}");
                    System.Diagnostics.Debugger.Break();
                    throw;
                }
            });
        }

        /// <summary>
        /// Выполняет action на WPF UI Dispatcher потоке. Можно безопасно обновлять визуальные элементы.
        /// </summary>
        public static void RunOnUiThread(Action action, DispatcherPriority priority = DispatcherPriority.Background) {
            Application.Current.Dispatcher.BeginInvoke(action, priority);
        }

        public static void RunOnUiThread(Func<Task> asyncAction, DispatcherPriority priority = DispatcherPriority.Background) {
            Application.Current.Dispatcher.InvokeAsync(async () => {
                try {
                    await asyncAction();
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"[RunOnUiThread] asyncAction exception: {ex}");
                    System.Diagnostics.Debugger.Break();
                    throw;
                }
            }, priority);
        }
    }
}