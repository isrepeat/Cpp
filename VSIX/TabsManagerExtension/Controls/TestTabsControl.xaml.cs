using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows.Controls;
using Microsoft.VisualStudio.Shell;
using Task = System.Threading.Tasks.Task;

namespace TabsManagerExtension.Controls {
    public partial class TestTabsControl : Helpers.BaseUserControl {
        //private TextEditorFrameActivationTracker? _textEditorFrameActivationTracker;
        public TestTabsControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e) {
            ThreadHelper.JoinableTaskFactory.Run(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                //_textEditorFrameActivationTracker = new TextEditorFrameActivationTracker();
                //_textEditorFrameActivationTracker.TextEditorFrameActivated += isActive => {
                //    this.Dispatcher.Invoke(() => {
                //        this.Background = isActive ? Brushes.Purple : Brushes.Gray;
                //    });
                //};
            });
        }

        private void UserControl_PreviewMouseDown(object sender, MouseButtonEventArgs e) {
            if (e.OriginalSource is not Button && e.OriginalSource is DependencyObject d) {
                var parentButton = Helpers.VisualTree.FindParentByType<Button>(d);
                if (parentButton == null) {
                    // Клик по пустому месту → перехватываем фокус
                    TextEditorControlHelper.FocusEditor();
                    this.FocusStealer.Focus();
                    //Keyboard.ClearFocus(); // глобально сбрасываем клавишный фокус со всего.
                    e.Handled = true;
                }
            }
        }

        private void OnLockEditorClick(object sender, RoutedEventArgs e) {
            //ThreadHelper.JoinableTaskFactory.Run(async () => {
            //    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            //    TextEditorControlHelper.SetEditorEditable(false);
            //    TextEditorControlHelper.SetCaretVisible(false);
            //});
        }

        private void OnUnlockEditorClick(object sender, RoutedEventArgs e) {
            //ThreadHelper.JoinableTaskFactory.Run(async () => {
            //    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            //    TextEditorControlHelper.SetEditorEditable(true);
            //    TextEditorControlHelper.SetCaretVisible(true);
            //    TextEditorControlHelper.FocusEditor();
            //});
        }
    }
}