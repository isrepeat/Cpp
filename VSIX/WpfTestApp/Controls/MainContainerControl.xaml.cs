using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace WpfTestApp.Controls {
    public partial class MainContainerControl : Helpers.BaseUserControl {
        public DataTemplate ControlPanelTemplateMain => (DataTemplate)this.Resources["ControlPanelTemplateMain"];
        public MainContainerControl() {
            InitializeComponent();
        }
    }
}