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
            // ���������� � ������������
            throw new InvalidOperationException("���������� � ������������.");
        }
    }

    public class MainWindow_4_CrashHandling_ViewModel : Helpers.ObservableObject {
        public MainWindow_4_CrashHandling_ViewModel() {

            try {
                // ������� ������ � �������������, ������������� ����������
                var obj = new FaultyClass();
            }
            catch (Exception ex) {
                Console.WriteLine($"���������� ����������� � ������ Main: {ex.Message}");
            }

            //// �������������� ���� ������ ��� ������ ������������
            //GC.Collect();
            //GC.WaitForPendingFinalizers();

            //ThrowExceptionAsync();           
        }

        private async Task ThrowExceptionAsync() {
            // ���������� ����� ��������� ������ ������������ ������
            throw new InvalidOperationException("���������� � ����������� ������.");
        }
    }

    public sealed partial class MainWindow_4_CrashHandling : Window {
        private MainWindow_4_CrashHandling_ViewModel viewModel;

        public MainWindow_4_CrashHandling() {
            this.InitializeComponent();
            this.viewModel = new MainWindow_4_CrashHandling_ViewModel();
            this.MainGrid.DataContext = this.viewModel;

            // �������������� ���� ������ ��� ������������ ��������������� ����������
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        //public MainWindow_4_CrashHandling() {
        //    this.InitializeComponent();

        //    // �������� �� ������� �������������� ���������� �����
        //    TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;

        //    // ����� ������������ ������ ��� await ��� ������������� ��������������� ����������
        //    ThrowExceptionAsync();

        //    // �������������� ���� ������ ��� ������������ ��������������� ����������
        //    GC.Collect();
        //    GC.WaitForPendingFinalizers();
        //    //GC.Collect();
        //}

        //private async Task ThrowExceptionAsync() {
        //    // ���������� ����� ��������� ������ ������������ ������
        //    throw new InvalidOperationException("���������� � ����������� ������.");
        //}

        //private void TaskScheduler_UnobservedTaskException(object? sender, UnobservedTaskExceptionEventArgs e) {
        //    // ��������� ��������������� ���������� ������
        //    Console.WriteLine($"�������������� ���������� ������: {e.Exception}");
        //    e.SetObserved(); // �������� ���������� ��� ������������
        //}

        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}