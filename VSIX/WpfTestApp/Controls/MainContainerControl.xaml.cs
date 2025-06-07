using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace WpfTestApp.Controls {
    public partial class MainContainerControl : Helpers.BaseUserControl {
        public ObservableCollection<string> Items { get; } = new ObservableCollection<string> {
            "MainItem1", "MainItem2", "MainItem3"
        };
        public DataTemplate ControlPanelTemplateMain => (DataTemplate)this.Resources["ControlPanelTemplateMain"];
        public MainContainerControl() {
            InitializeComponent();
            this.DataContext = this;
        }
    }
}