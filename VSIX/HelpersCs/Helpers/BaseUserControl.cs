using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace Helpers {
    public class BaseUserControl : UserControl {
        public object? RootControl {
            get => (object?)GetValue(RootControlProperty);
            private set => SetValue(RootControlProperty, value);
        }

        public static readonly DependencyProperty RootControlProperty =
            DependencyProperty.Register(
                nameof(RootControl),
                typeof(object),
                typeof(BaseUserControl),
                new PropertyMetadata(null));


        public object? HostControl {
            get => (object?)GetValue(HostControlProperty);
            set => SetValue(HostControlProperty, value);
        }

        public static readonly DependencyProperty HostControlProperty =
            DependencyProperty.Register(
                nameof(HostControl),
                typeof(object),
                typeof(BaseUserControl),
                new PropertyMetadata(null));


        public BaseUserControl() {
            this.RootControl = this;
        }
    }
}