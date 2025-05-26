using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace WpfTestApp.Controls {
    public partial class CustomTitleBar : UserControl {
        public CustomTitleBar() {
            InitializeComponent();
        }

        private void Minimize_Click(object sender, RoutedEventArgs e) {
            Window.GetWindow(this).WindowState = WindowState.Minimized;
        }

        private void Maximize_Click(object sender, RoutedEventArgs e) {
            var win = Window.GetWindow(this);
            win.WindowState = win.WindowState == WindowState.Maximized
                ? WindowState.Normal
                : WindowState.Maximized;
        }

        private void Close_Click(object sender, RoutedEventArgs e) {
            Window.GetWindow(this).Close();
        }

        private void Grid_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) {
            var win = Window.GetWindow(this);
            if (e.ClickCount == 2) {
                Maximize_Click(sender, e);
            }
            else {
                win?.DragMove();
            }
        }
    }
}