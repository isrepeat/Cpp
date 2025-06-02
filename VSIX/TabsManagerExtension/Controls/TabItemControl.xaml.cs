using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TabsManagerExtension.Controls {
    public partial class TabItemControl : UserControl {
        public TabItemControl() {
            this.InitializeComponent();
            this.CustomContextMenu.Closed += this.CustomContextMenu_Closed;
        }

        public string Title {
            get { return (string)this.GetValue(TitleProperty); }
            set { this.SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register("Title", typeof(string), typeof(TabItemControl), new PropertyMetadata("Document Title"));


        public bool IsSelected {
            get { return (bool)this.GetValue(IsSelectedProperty); }
            set { this.SetValue(IsSelectedProperty, value); }
        }

        public static readonly DependencyProperty IsSelectedProperty =
            DependencyProperty.Register("IsSelected", typeof(bool), typeof(TabItemControl), new PropertyMetadata(false));


        public object ControlPanelContent {
            get { return this.GetValue(ControlPanelContentProperty); }
            set { this.SetValue(ControlPanelContentProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelContentProperty =
            DependencyProperty.Register("ControlPanelContent", typeof(object), typeof(TabItemControl), new PropertyMetadata(null));


        public Visibility ControlPanelVisibility {
            get { return (Visibility)this.GetValue(ControlPanelVisibilityProperty); }
            set { this.SetValue(ControlPanelVisibilityProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelVisibilityProperty =
            DependencyProperty.Register("ControlPanelVisibility", typeof(Visibility), typeof(TabItemControl), new PropertyMetadata(Visibility.Collapsed));


        public object ContextMenuContent {
            get { return this.GetValue(ContextMenuContentProperty); }
            set { this.SetValue(ContextMenuContentProperty, value); }
        }

        public static readonly DependencyProperty ContextMenuContentProperty =
            DependencyProperty.Register("ContextMenuContent", typeof(object), typeof(TabItemControl), new PropertyMetadata(null));

        public ICommand OnContextMenuOpenCommand {
            get { return (ICommand)this.GetValue(OnContextMenuOpenCommandProperty); }
            set { this.SetValue(OnContextMenuOpenCommandProperty, value); }
        }

        public static readonly DependencyProperty OnContextMenuOpenCommandProperty =
            DependencyProperty.Register("OnContextMenuOpenCommand", typeof(ICommand), typeof(TabItemControl), new PropertyMetadata(null));

        public ICommand OnContextMenuClosedCommand {
            get { return (ICommand)this.GetValue(OnContextMenuClosedCommandProperty); }
            set { this.SetValue(OnContextMenuClosedCommandProperty, value); }
        }

        public static readonly DependencyProperty OnContextMenuClosedCommandProperty =
            DependencyProperty.Register("OnContextMenuClosedCommand", typeof(ICommand), typeof(TabItemControl), new PropertyMetadata(null));


        private void RootControl_MouseRightButtonUp(object sender, MouseButtonEventArgs e) {
            if (this.ContextMenuContent != null) {
                this.CustomContextMenu.IsOpen = true;
                if (this.OnContextMenuOpenCommand != null && this.OnContextMenuOpenCommand.CanExecute(this.DataContext)) {
                    this.OnContextMenuOpenCommand.Execute(this.DataContext);
                }
                e.Handled = true;
            }
        }
        private void CustomContextMenu_Closed(object sender, EventArgs e) {
            if (this.OnContextMenuClosedCommand != null && this.OnContextMenuClosedCommand.CanExecute(this.DataContext)) {
                this.OnContextMenuClosedCommand.Execute(this.DataContext);
            }
        }
    }
}