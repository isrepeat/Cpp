using System.Windows;
using System.Windows.Controls;

namespace WpfTestApp.Controls {
    public partial class FirstContainerControl : Helpers.BaseUserControl {
        public object ControlPanelContent {
            get { return this.GetValue(ControlPanelContentProperty); }
            set { this.SetValue(ControlPanelContentProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelContentProperty =
            DependencyProperty.Register(
                nameof(ControlPanelContent),
                typeof(object),
                typeof(FirstContainerControl),
                new PropertyMetadata(null));


        public DataTemplate ControlPanelTemplateFirst => (DataTemplate)this.Resources["ControlPanelTemplateFirst"];

        public FirstContainerControl() {
            InitializeComponent();
        }
    }
}