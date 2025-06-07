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

        public DataTemplate ControlPanelButtonsPrimarySlotTemplate {
            get => (DataTemplate)GetValue(ControlPanelButtonsPrimarySlotTemplateProperty);
            set => SetValue(ControlPanelButtonsPrimarySlotTemplateProperty, value);
        }
        public static readonly DependencyProperty ControlPanelButtonsPrimarySlotTemplateProperty =
            DependencyProperty.Register(
                nameof(ControlPanelButtonsPrimarySlotTemplate),
                typeof(DataTemplate),
                typeof(SecondContainerControl),
                new PropertyMetadata(null)
            );


        public SecondContainerControl() {
            InitializeComponent();
            this.DataContext = this;
        }
    }
}