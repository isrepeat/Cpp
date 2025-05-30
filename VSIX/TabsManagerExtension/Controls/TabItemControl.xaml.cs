using System.Windows;
using System.Windows.Controls;

namespace TabsManagerExtension.Controls {
    public partial class TabItemControl : UserControl {
        public TabItemControl() {
            InitializeComponent();
        }

        public string Title {
            get { return (string)GetValue(TitleProperty); }
            set { SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register("Title", typeof(string), typeof(TabItemControl), new PropertyMetadata("Document Title"));


        public bool IsActive {
            get { return (bool)GetValue(IsActiveProperty); }
            set { SetValue(IsActiveProperty, value); }
        }

        public static readonly DependencyProperty IsActiveProperty =
            DependencyProperty.Register("IsActive", typeof(bool), typeof(TabItemControl), new PropertyMetadata(false));


        public bool IsSelected {
            get { return (bool)GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
        }

        public static readonly DependencyProperty IsSelectedProperty =
            DependencyProperty.Register("IsSelected", typeof(bool), typeof(TabItemControl), new PropertyMetadata(false));


        // Свойство для управления контентом панели (кнопки)
        public object ControlPanelContent {
            get { return (object)GetValue(ControlPanelContentProperty); }
            set { SetValue(ControlPanelContentProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelContentProperty =
            DependencyProperty.Register("ControlPanelContent", typeof(object), typeof(TabItemControl), new PropertyMetadata(null));


        public Visibility ControlPanelVisibility {
            get { return (Visibility)GetValue(ControlPanelVisibilityProperty); }
            set { SetValue(ControlPanelVisibilityProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelVisibilityProperty =
            DependencyProperty.Register("ControlPanelVisibility", typeof(Visibility), typeof(TabItemControl), new PropertyMetadata(Visibility.Hidden));
    }
}
