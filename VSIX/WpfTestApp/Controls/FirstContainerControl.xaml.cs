using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Runtime.CompilerServices;
using System.Collections.ObjectModel;
using System.ComponentModel;

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


        private Helpers.Properties.VisibilityProperty _isControlPanelVisible = new();
        public Helpers.Properties.VisibilityProperty IsControlPanelVisible {
            get { return _isControlPanelVisible; }
            set {
                if (_isControlPanelVisible != value) {
                    _isControlPanelVisible = value;
                    OnPropertyChanged();
                }
            }
        }
        public DataTemplate ControlPanelTemplateFirst => (DataTemplate)this.Resources["ControlPanelTemplateFirst"];

        public FirstContainerControl() {
            this.InitializeComponent();
        }
        private void OnMouseEnter(object sender, MouseEventArgs e) {
            this.IsControlPanelVisible.Value = true;
        }

        private void OnMouseLeave(object sender, MouseEventArgs e) {
            this.IsControlPanelVisible.Value = false;
        }
    }
}