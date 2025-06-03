using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Collections.ObjectModel;
using System.Windows.Controls.Primitives;

namespace TabsManagerExtension.Controls {
    public partial class MenuControl : UserControl {
        public event EventHandler MouseEnteredPopup;
        public event EventHandler MouseLeftPopup;

        public class ContextMenuOpenRequest {
            public object DataContext { get; set; } = default!;
            public bool ShouldOpen { get; set; } = true;
        }

        public MenuControl() {
            this.InitializeComponent();
        }

        public ObservableCollection<Helpers.IMenuItem> MenuItems {
            get => (ObservableCollection<Helpers.IMenuItem>)this.GetValue(MenuItemsProperty);
            set => this.SetValue(MenuItemsProperty, value);
        }

        public static readonly DependencyProperty MenuItemsProperty =
            DependencyProperty.Register(
                nameof(MenuItems),
                typeof(ObservableCollection<Helpers.IMenuItem>),
                typeof(MenuControl),
                new PropertyMetadata(null));


        public ICommand OpenCommand {
            get => (ICommand)this.GetValue(OpenCommandProperty);
            set => this.SetValue(OpenCommandProperty, value);
        }

        public static readonly DependencyProperty OpenCommandProperty =
            DependencyProperty.Register(
                nameof(OpenCommand),
                typeof(ICommand),
                typeof(MenuControl),
                new PropertyMetadata(null));


        public ICommand CloseCommand {
            get => (ICommand)this.GetValue(CloseCommandProperty);
            set => this.SetValue(CloseCommandProperty, value);
        }

        public static readonly DependencyProperty CloseCommandProperty =
            DependencyProperty.Register(
                nameof(CloseCommand),
                typeof(ICommand),
                typeof(MenuControl),
                new PropertyMetadata(null));


        public object CommandParameterContext {
            get => this.GetValue(CommandParameterContextProperty);
            set => this.SetValue(CommandParameterContextProperty, value);
        }

        public static readonly DependencyProperty CommandParameterContextProperty =
            DependencyProperty.Register(
                nameof(CommandParameterContext),
                typeof(object),
                typeof(MenuControl),
                new PropertyMetadata(null));


        public void ShowMenu(PlacementMode placementMode, bool isStaysOpen, Point? screenPosition = null) {
            if (this.OpenCommand != null && this.OpenCommand.CanExecute(null)) {
                var request = new MenuControl.ContextMenuOpenRequest {
                    DataContext = this.CommandParameterContext
                };

                // Через команду заполняются MenuItems.
                this.OpenCommand.Execute(request);
                if (!request.ShouldOpen) {
                    return;
                }
            }

            this.MenuPopup.Placement = placementMode;
            this.MenuPopup.StaysOpen = isStaysOpen;
            if (screenPosition.HasValue) {
                this.MoveMenu(screenPosition.Value);
            }
            this.MenuPopup.IsOpen = true;
        }


        public void UpdateMenu(Point? screenPosition = null) {
            if (this.OpenCommand != null && this.OpenCommand.CanExecute(null)) {
                var request = new MenuControl.ContextMenuOpenRequest {
                    DataContext = this.CommandParameterContext
                };

                // Через команду заполняются MenuItems.
                this.OpenCommand.Execute(request);

                // Do not handle request.ShouldOpen here.
            }

            if (screenPosition.HasValue) {
                this.MoveMenu(screenPosition.Value);
            }
        }


        public void MoveMenu(Point screenPosition) {
            this.MenuPopup.HorizontalOffset = screenPosition.X;
            this.MenuPopup.VerticalOffset = screenPosition.Y;
        }

        //public void RefreshDataContext(object newContext) {
        //    this.DataContext = newContext;
        //    if (this.MenuPopup.Child is FrameworkElement fe) {
        //        fe.DataContext = newContext;
        //    }
        //}


        private void MenuPopup_Closed(object sender, System.EventArgs e) {
            if (this.CloseCommand != null && this.CloseCommand.CanExecute(this.CommandParameterContext)) {
                this.CloseCommand.Execute(this.CommandParameterContext);
            }
        }
        private void MenuPopup_MouseEnter(object sender, MouseEventArgs e) {
            this.MouseEnteredPopup?.Invoke(this, EventArgs.Empty);
        }
        private void MenuPopup_MouseLeave(object sender, MouseEventArgs e) {
            this.MouseLeftPopup?.Invoke(this, EventArgs.Empty);
        }
    }
}