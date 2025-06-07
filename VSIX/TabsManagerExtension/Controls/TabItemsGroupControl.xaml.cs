using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace TabsManagerExtension.Controls {
    public partial class TabItemsGroupControl : Helpers.BaseUserControl {
        public DataTemplate? TabItemTemplate {
            get => (DataTemplate?)this.GetValue(TabItemTemplateProperty);
            set => this.SetValue(TabItemTemplateProperty, value);
        }

        public static readonly DependencyProperty TabItemTemplateProperty =
            DependencyProperty.Register(
                nameof(TabItemTemplate),
                typeof(DataTemplate),
                typeof(TabItemsGroupControl),
                new PropertyMetadata(null));

        public TabItemsGroupControl() {
            InitializeComponent();
        }
    }
}