using Helpers.Ex;
using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace TabsManagerExtension.Controls {
    public partial class TabItemControl : UserControl {
        public TabItemControl() {
            this.InitializeComponent();
        }

        public string Title {
            get { return (string)this.GetValue(TitleProperty); }
            set { this.SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register(
                nameof(Title),
                typeof(string),
                typeof(TabItemControl),
                new PropertyMetadata("Document Title"));


        public bool IsSelected {
            get { return (bool)this.GetValue(IsSelectedProperty); }
            set { this.SetValue(IsSelectedProperty, value); }
        }

        public static readonly DependencyProperty IsSelectedProperty =
            DependencyProperty.Register(
                nameof(IsSelected),
                typeof(bool),
                typeof(TabItemControl),
                new PropertyMetadata(false));


        public object ControlPanelContent {
            get { return this.GetValue(ControlPanelContentProperty); }
            set { this.SetValue(ControlPanelContentProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelContentProperty =
            DependencyProperty.Register(
                nameof(ControlPanelContent),
                typeof(object),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        public Visibility ControlPanelVisibility {
            get { return (Visibility)this.GetValue(ControlPanelVisibilityProperty); }
            set { this.SetValue(ControlPanelVisibilityProperty, value); }
        }

        public static readonly DependencyProperty ControlPanelVisibilityProperty =
            DependencyProperty.Register(
                nameof(ControlPanelVisibility),
                typeof(Visibility),
                typeof(TabItemControl),
                new PropertyMetadata(Visibility.Collapsed));


        public ObservableCollection<Helpers.IMenuItem> ContextMenuItems {
            get { return (ObservableCollection<Helpers.IMenuItem>)this.GetValue(ContextMenuItemsProperty); }
            set { this.SetValue(ContextMenuItemsProperty, value); }
        }

        public static readonly DependencyProperty ContextMenuItemsProperty =
            DependencyProperty.Register(
                nameof(ContextMenuItems),
                typeof(ObservableCollection<Helpers.IMenuItem>),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        public ICommand OnContextMenuOpenCommand {
            get => (ICommand)this.GetValue(OnContextMenuOpenCommandProperty);
            set => this.SetValue(OnContextMenuOpenCommandProperty, value);
        }

        public static readonly DependencyProperty OnContextMenuOpenCommandProperty =
            DependencyProperty.Register(
                nameof(OnContextMenuOpenCommand),
                typeof(ICommand),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        public ICommand OnContextMenuClosedCommand {
            get => (ICommand)this.GetValue(OnContextMenuClosedCommandProperty);
            set => this.SetValue(OnContextMenuClosedCommandProperty, value);
        }

        public static readonly DependencyProperty OnContextMenuClosedCommandProperty =
            DependencyProperty.Register(
                nameof(OnContextMenuClosedCommand),
                typeof(ICommand),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        private void RootControl_MouseRightButtonUp(object sender, MouseButtonEventArgs e) {
            //Point mouseScreenPoint = this.ToDpiAwareScreen(e.GetPosition(this));
            //this.ContextMenu.ShowMenu(new Point{ }); 
            this.ContextMenu.ShowMenu(PlacementMode.MousePoint, isStaysOpen: false); 
            e.Handled = true;
        }
    }
}