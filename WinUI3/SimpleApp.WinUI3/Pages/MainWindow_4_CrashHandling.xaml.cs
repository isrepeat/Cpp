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
using System.ComponentModel;


namespace SimpleApp.WinUI3 {
    class FaultyClass {
        ~FaultyClass() {
            // Исключение в финализаторе
            throw new InvalidOperationException("Исключение в финализаторе.");
        }
    }

    public class MainWindow_4_CrashHandling_ViewModel : Helpers.ObservableObject {
        public MainWindow_4_CrashHandling_ViewModel() {

            try {
                // Создаем объект с финализатором, выбрасывающим исключение
                var obj = new FaultyClass();
            }
            catch (Exception ex) {
                Console.WriteLine($"Исключение перехвачено в методе Main: {ex.Message}");
            }

            //// Принудительный сбор мусора для вызова финализатора
            //GC.Collect();
            //GC.WaitForPendingFinalizers();

            //ThrowExceptionAsync();           
        }

        private async Task ThrowExceptionAsync() {
            // Исключение будет выброшено внутри асинхронного метода
            throw new InvalidOperationException("Исключение в асинхронном методе.");
        }
    }

    public sealed partial class MainWindow_4_CrashHandling : Window {
        private MainWindow_4_CrashHandling_ViewModel viewModel;

        public MainWindow_4_CrashHandling() {
            this.InitializeComponent();
            this.viewModel = new MainWindow_4_CrashHandling_ViewModel();
            this.MainGrid.DataContext = this.viewModel;

            // Принудительный сбор мусора для демонстрации необработанного исключения
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        //public MainWindow_4_CrashHandling() {
        //    this.InitializeComponent();

        //    // Подписка на событие необработанных исключений задач
        //    TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;

        //    // Вызов асинхронного метода без await для моделирования необработанного исключения
        //    ThrowExceptionAsync();

        //    // Принудительный сбор мусора для демонстрации необработанного исключения
        //    GC.Collect();
        //    GC.WaitForPendingFinalizers();
        //    //GC.Collect();
        //}

        //private async Task ThrowExceptionAsync() {
        //    // Исключение будет выброшено внутри асинхронного метода
        //    throw new InvalidOperationException("Исключение в асинхронном методе.");
        //}

        //private void TaskScheduler_UnobservedTaskException(object? sender, UnobservedTaskExceptionEventArgs e) {
        //    // Обработка необработанного исключения задачи
        //    Console.WriteLine($"Необработанное исключение задачи: {e.Exception}");
        //    e.SetObserved(); // Помечаем исключение как обработанное
        //}

        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}