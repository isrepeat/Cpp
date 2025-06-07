using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace TabsManagerExtension.Controls {
    public partial class TabItemControl : Helpers.BaseUserControl {
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


        public DataTemplate ControlPanelPrimarySlotTemplate {
            get => (DataTemplate)this.GetValue(ControlPanelPrimarySlotTemplateProperty);
            set => this.SetValue(ControlPanelPrimarySlotTemplateProperty, value);
        }
        public static readonly DependencyProperty ControlPanelPrimarySlotTemplateProperty =
            DependencyProperty.Register(nameof(ControlPanelPrimarySlotTemplate),
                typeof(DataTemplate),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        public DataTemplate ControlPanelSecondarySlotTemplate {
            get => (DataTemplate)this.GetValue(ControlPanelSecondarySlotTemplateProperty);
            set => this.SetValue(ControlPanelSecondarySlotTemplateProperty, value);
        }
        public static readonly DependencyProperty ControlPanelSecondarySlotTemplateProperty =
            DependencyProperty.Register(nameof(ControlPanelSecondarySlotTemplate),
                typeof(DataTemplate),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        public DataTemplate ContextMenuTemplate {
            get => (DataTemplate)this.GetValue(ContextMenuTemplateProperty);
            set => this.SetValue(ContextMenuTemplateProperty, value);
        }
        public static readonly DependencyProperty ContextMenuTemplateProperty =
            DependencyProperty.Register(nameof(ContextMenuTemplate),
                typeof(DataTemplate),
                typeof(TabItemControl),
                new PropertyMetadata(null));


        private bool _isMouseInside = false;
        public bool IsMouseInside {
            get { return _isMouseInside; }
            set {
                if (_isMouseInside != value) {
                    _isMouseInside = value;
                    OnPropertyChanged();
                }
            }
        }


        private WeakReference<MenuControl>? _cachedWeakMenuControl;

        public TabItemControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
            this.MouseEnter += this.OnMouseEnter;
            this.MouseLeave += this.OnMouseLeave;
            this.MouseRightButtonUp += this.OnMouseRightButtonUp;
        }

        private void OnLoaded(object sender, RoutedEventArgs e) {
            this.FindAndCacheMenuControl();
        }

        private void OnMouseEnter(object sender, MouseEventArgs e) {
            this.IsMouseInside = true;
        }

        private void OnMouseLeave(object sender, MouseEventArgs e) {
            this.IsMouseInside = false;
        }
        private void OnMouseRightButtonUp(object sender, MouseButtonEventArgs e) {
            MenuControl? menuControl = null;

            if (_cachedWeakMenuControl?.TryGetTarget(out var cachedMenuControl) == true) {
                menuControl = cachedMenuControl;
            }
            else {
                menuControl = this.FindAndCacheMenuControl();
            }

            if (menuControl != null) {
                menuControl.ShowMenu(PlacementMode.MousePoint, isStaysOpen: false);
            }

            e.Handled = true;
        }


        private MenuControl FindAndCacheMenuControl() {
            var menuControl = Helpers.VisualTree.FindChildByType<MenuControl>(this.ContextMenuContentPresenter);
            if (menuControl == null) {
                throw new InvalidOperationException("ContextMenuTemplate must produce a MenuControl.");
            }
            _cachedWeakMenuControl = new WeakReference<MenuControl>(menuControl);
            return menuControl;
        }
    }
}