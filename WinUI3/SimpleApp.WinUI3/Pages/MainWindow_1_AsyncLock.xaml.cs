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


    public class AppSettings : ILockable {
        private readonly SemaphoreSlim semaphore = new SemaphoreSlim(1, 1);

        public AppSettings() {
        }

        public SemaphoreSlim GetLockableObj() {
            return this.semaphore;
        }
    }



    public sealed partial class MainWindow_1_AsyncLock : Window {
        private readonly CancellationTokenSource cts = new CancellationTokenSource();
        private readonly PeriodicTimer timerOne = new PeriodicTimer(TimeSpan.FromSeconds(3));
        private readonly PeriodicTimer timerTwo = new PeriodicTimer(TimeSpan.FromSeconds(4));

        private AppSettings appSettings = new AppSettings();

        public MainWindow_1_AsyncLock() {
            this.InitializeComponent();
            this.InitializeTimerOneAsync();
            this.InitializeTimerTwoAsync();
        }

        private async void InitializeTimerOneAsync() {
            await this.StartTimerOneAsync();
        }
        private async void InitializeTimerTwoAsync() {
            await this.StartTimerTwoAsync();
        }

        private async Task StartTimerOneAsync() {
            try {
                while (await this.timerOne.WaitForNextTickAsync(this.cts.Token)) {
                    System.Diagnostics.Debug.WriteLine($"TimerOneHandler: start");
                    await UpdateGalleryItems("one");
                    System.Diagnostics.Debug.WriteLine($"TimerOneHandler: end");
                }
            }
            catch (OperationCanceledException ex) {
                System.Diagnostics.Debugger.Break();
            }
            catch (System.Exception ex) {
                System.Diagnostics.Debugger.Break();
            }
        }

        private async Task StartTimerTwoAsync() {
            try {
                while (await this.timerTwo.WaitForNextTickAsync(this.cts.Token)) {
                    System.Diagnostics.Debug.WriteLine($"TimerTwoHandler: start");
                    await UpdateGalleryItems("two");
                    System.Diagnostics.Debug.WriteLine($"TimerTwoHandler: end");
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
            using (var appSettingsLockedObj = await LockGuard<AppSettings>.LockAsync(this.appSettings)) {
                System.Diagnostics.Debug.WriteLine($"UpdateGalleryItems [{timerName}]: wait 2s ...");
                await Task.Delay(2000);
                System.Diagnostics.Debug.WriteLine($"UpdateGalleryItems [{timerName}]: finish");
            }
        }

        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}