using System.Windows;
using System.Windows.Controls;

namespace TabsManagerExtension.Controls {
    public partial class TabItemControl : UserControl {
        public TabItemControl() {
            InitializeComponent();
        }

        // Свойство Title для заголовка документа
        public string Title {
            get { return (string)GetValue(TitleProperty); }
            set { SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register("Title", typeof(string), typeof(TabItemControl), new PropertyMetadata("Document Title"));

        // Свойство для управления контентом панели (кнопки)
        public object ControlContent {
            get { return (object)GetValue(ControlContentProperty); }
            set { SetValue(ControlContentProperty, value); }
        }

        public static readonly DependencyProperty ControlContentProperty =
            DependencyProperty.Register("ControlContent", typeof(object), typeof(TabItemControl), new PropertyMetadata(null));

        // Свойство для управления видимостью ControlPanel
        public Visibility ControlPanelVisibility {
            get { return (Visibility)GetValue(ControlPanelVisibilityProperty); }
            set { SetValue(ControlPanelVisibilityProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelVisibilityProperty =
            DependencyProperty.Register("ControlPanelVisibility", typeof(Visibility), typeof(TabItemControl), new PropertyMetadata(Visibility.Hidden));

        // Свойство для управления прозрачностью ControlPanel
        public double ControlPanelOpacity {
            get { return (double)GetValue(ControlPanelOpacityProperty); }
            set { SetValue(ControlPanelOpacityProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelOpacityProperty =
            DependencyProperty.Register("ControlPanelOpacity", typeof(double), typeof(TabItemControl), new PropertyMetadata(0.0));
    }
}
