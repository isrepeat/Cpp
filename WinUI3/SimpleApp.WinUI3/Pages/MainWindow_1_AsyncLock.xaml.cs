#define __LOG_ENABLE_LOGGING_LOCKS

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
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
    public interface ILockable {
        SemaphoreSlim GetLockableObj();
    }

    public class LockGuard<T> where T : ILockable {
        public static async Task<LockedObj> LockAsync(T instance) {
            System.Diagnostics.Debug.WriteLine($"Lock");
            var semaphore = instance.GetLockableObj();
            await semaphore.WaitAsync();
            return new LockedObj(instance, semaphore);
        }

        public class LockedObj : IDisposable {
            private readonly SemaphoreSlim semaphore;
            private readonly T instance;
            public T Instance {
                get {
                    return this.instance;
                }
            }
            public LockedObj(T instance, SemaphoreSlim semaphore) {
                this.instance = instance;
                this.semaphore = semaphore;
            }
            public void Dispose() {
                this.semaphore.Release();
                System.Diagnostics.Debug.WriteLine($"Release");
            }
        }
    }


    public class LockContext {
        public string Name { get; set; }
        public int RecursionCount { get; set; } = 0;
        public bool SemaphoreAcquired { get; set; } = false;
    }


    public class AsyncManager<T> where T : ILockable {
        private static readonly AsyncManager<T> _instance = new AsyncManager<T>();
        public static AsyncManager<T> Instance => _instance;
        public static AsyncRecursiveLock<T> AsyncRecursiveLock { get; } = new AsyncRecursiveLock<T>();
    }

    public class AsyncRecursiveLock<T> where T : ILockable {
        private AsyncLocal<LockContext> rootContext = new AsyncLocal<LockContext>();

        private AsyncLocal<string> currentContextName = new AsyncLocal<string>();

        public void SetRootContext(string rootContextName) {
            this.rootContext.Value = new LockContext {
                Name = rootContextName
            };
        }

        public string GetRootContextName() {
            if (this.rootContext.Value == null) {
                return "null ctx";
            }
            return this.rootContext.Value.Name;
        }

        public AsyncRecursiveLock<T> AutoCtx() {
            if (this.rootContext.Value == null) {
                this.rootContext.Value = new LockContext {
                    Name = "context Default"
                };
            }
            return this;
        }

        public async Task<LockedObj> LockAsync(T instance) {
            var semaphore = instance.GetLockableObj();

            if (this.rootContext.Value == null) {
#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Lock acquire ... [null ctx]");
#endif
                await semaphore.WaitAsync();
#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Lock acquired [null ctx]");
#endif
                return new LockedObj(instance, semaphore, null);
            }

            if (String.IsNullOrEmpty(this.currentContextName.Value) || this.currentContextName.Value != this.rootContext.Value.Name) {
#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Acquire lock ... [{this.rootContext.Value.Name}]");
#endif
                await semaphore.WaitAsync();

#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Lock acquired [{this.rootContext.Value.Name}]");
#endif
                this.rootContext.Value.RecursionCount = 1;
                this.rootContext.Value.SemaphoreAcquired = true;

                return new LockedObj(instance, semaphore, this.rootContext.Value);

            }
            else {
#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Lock ignore acquisition [{this.rootContext.Value.Name}]");
#endif
                this.rootContext.Value.RecursionCount++;
                return new LockedObj(instance, semaphore, this.rootContext.Value);
            }
        }
        public bool Release(SemaphoreSlim semaphore) {
            // NOTE: In LockAsync() we wait semaphore and if this.rootContext.Value == null,
            //       so call semaphore.Release() is safe.
            if (this.rootContext.Value == null) {
#if __LOG_ENABLE_LOGGING_LOCKS
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Release [null ctx]");
#endif
                semaphore.Release();
                return true;
            }

            this.rootContext.Value.RecursionCount--;

            if (this.rootContext.Value.RecursionCount == 0) {
                if (this.rootContext.Value.SemaphoreAcquired) {
#if __LOG_ENABLE_LOGGING_LOCKS
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] Release [{this.rootContext.Value.Name}]");
#endif
                    semaphore.Release();
                    this.rootContext.Value.SemaphoreAcquired = false;
                    return true;
                }
            }
            return false;
        }


        public class LockedObj : IDisposable {
            private readonly T instance;
            public T Instance {
                get {
                    return this.instance;
                }
            }
            private readonly SemaphoreSlim semaphore;
            public LockContext lockContext;

            private bool saveCurrentContextCalled = false;
            private bool disposed = false;

            public LockedObj(T instance, SemaphoreSlim semaphore, LockContext lockContext) {
                this.instance = instance;
                this.semaphore = semaphore;
                this.lockContext = lockContext;
            }

            public void SaveCurrentContext() {
                this.saveCurrentContextCalled = true;
                if (this.lockContext == null) {
                    return;
                }
                AsyncManager<T>.AsyncRecursiveLock.currentContextName.Value = this.lockContext.Name;
            }

            public void Dispose() {
                if (!this.saveCurrentContextCalled) {
#if __LOG_ENABLE_LOGGING_LOCKS
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss}] SaveCurrentContext() was not called!");
#endif
                    System.Diagnostics.Debugger.Break();
                }
                if (!this.disposed) {
                    if (AsyncManager<T>.AsyncRecursiveLock.Release(this.semaphore)) {
                        AsyncManager<T>.AsyncRecursiveLock.currentContextName.Value = null;
                    }
                    this.disposed = true;
                }
            }
        }
    }





    public class AppSettings : ILockable {
        private readonly SemaphoreSlim semaphore = new SemaphoreSlim(1, 1);

        public SemaphoreSlim GetLockableObj() {
            return this.semaphore;
        }
    }


    
    public sealed partial class MainWindow_1_AsyncLock : Window {
        private readonly CancellationTokenSource cts = new CancellationTokenSource();
        private readonly PeriodicTimer timerA = new PeriodicTimer(TimeSpan.FromSeconds(3));
        private readonly PeriodicTimer timerB = new PeriodicTimer(TimeSpan.FromSeconds(4));

        private AppSettings appSettings = new AppSettings();

        public MainWindow_1_AsyncLock() {
            this.InitializeComponent();
            this.InitializeTimerAAsync();
            this.InitializeTimerBAsync();
        }

        private async void InitializeTimerAAsync() {
            await this.StartTimerAAsync();
        }
        private async void InitializeTimerBAsync() {
            await this.StartTimerBAsync();
        }

        private async Task StartTimerAAsync() {
            AsyncManager<AppSettings>.AsyncRecursiveLock.SetRootContext("context A");
            try {
                while (await this.timerA.WaitForNextTickAsync(this.cts.Token)) {
                    using (var appSettingsLockedObj = await AsyncManager<AppSettings>.AsyncRecursiveLock.AutoCtx().LockAsync(this.appSettings)) {
                        appSettingsLockedObj.SaveCurrentContext();

                        System.Diagnostics.Debug.WriteLine($"TimerA handler [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: start");
                        await UpdateGalleryItems("A");
                        System.Diagnostics.Debug.WriteLine($"TimerA handler [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: end");
                    }
                }
            }
            catch (OperationCanceledException ex) {
                System.Diagnostics.Debugger.Break();
            }
            catch (System.Exception ex) {
                System.Diagnostics.Debugger.Break();
            }
        }

        private async Task StartTimerBAsync() {
            try {
                while (await this.timerB.WaitForNextTickAsync(this.cts.Token)) {
                    using (var appSettingsLockedObj = await AsyncManager<AppSettings>.AsyncRecursiveLock.AutoCtx().LockAsync(this.appSettings)) {
                        appSettingsLockedObj.SaveCurrentContext();

                        System.Diagnostics.Debug.WriteLine($"TimerB handler [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: start");
                        await UpdateGalleryItems("B");
                        System.Diagnostics.Debug.WriteLine($"TimerB handler [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: end");
                    }
                }
            }
            catch (OperationCanceledException ex) {
                System.Diagnostics.Debugger.Break();
            }
            catch (System.Exception ex) {
                System.Diagnostics.Debugger.Break();
            }
        }

        private void StopTimer() {
            this.cts.Cancel();
        }


        public async Task UpdateGalleryItems(string timerName) {
            using (var appSettingsLockedObj = await AsyncManager<AppSettings>.AsyncRecursiveLock.AutoCtx().LockAsync(this.appSettings)) {
                appSettingsLockedObj.SaveCurrentContext();

                System.Diagnostics.Debug.WriteLine($"UpdateGalleryItems [{timerName}] [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: wait 2s ...");
                await Task.Delay(2000);
                System.Diagnostics.Debug.WriteLine($"UpdateGalleryItems [{timerName}] [{AsyncManager<AppSettings>.AsyncRecursiveLock.GetRootContextName()}]: finish");
            }
        }

        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}