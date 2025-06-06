using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace WpfTestApp.Controls {
    public partial class SecondContainerControl : Helpers.BaseUserControl {
        public ObservableCollection<string> Items { get; } = new ObservableCollection<string> {
            "One", "Two", "Three"
        };

        public DataTemplate ControlPanelTemplateSecond => (DataTemplate)this.Resources["ControlPanelTemplateSecond"];
        public DataTemplate ControlPanelTemplateSecond2 => (DataTemplate)this.Resources["ControlPanelTemplateSecond2"];

        public SecondContainerControl() {
            InitializeComponent();
            this.DataContext = this;
        }
    }
}