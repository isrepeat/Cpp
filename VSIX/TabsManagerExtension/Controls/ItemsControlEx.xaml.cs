using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TabsManagerExtension.Controls {
    public partial class ItemsControlEx : UserControl {
        public ItemsControlEx() {
            InitializeComponent();

            this.GotKeyboardFocus += (_, _) => {
                Helpers.FocusStateAttachedProperties.SetIsParentControlFocused(this, true);
            };

            this.LostKeyboardFocus += (_, _) => {
                Dispatcher.BeginInvoke((Action)(() => {
                    bool stillFocused = IsKeyboardFocusWithin;
                    Helpers.FocusStateAttachedProperties.SetIsParentControlFocused(this, stillFocused);
                }), DispatcherPriority.Input);
            };
        }

        public static readonly DependencyProperty ItemsProperty =
            DependencyProperty.Register(nameof(Items), typeof(object), typeof(ItemsControlEx), new PropertyMetadata(null));

        public object Items {
            get => GetValue(ItemsProperty);
            set => SetValue(ItemsProperty, value);
        }

        public static readonly DependencyProperty ItemTemplateProperty =
            DependencyProperty.Register(nameof(ItemTemplate), typeof(DataTemplate), typeof(ItemsControlEx), new PropertyMetadata(null));

        public DataTemplate ItemTemplate {
            get => (DataTemplate)GetValue(ItemTemplateProperty);
            set => SetValue(ItemTemplateProperty, value);
        }

        public static readonly DependencyProperty ItemContainerStyleProperty =
            DependencyProperty.Register(nameof(ItemContainerStyle), typeof(Style), typeof(ItemsControlEx), new PropertyMetadata(null));

        public Style ItemContainerStyle {
            get => (Style)GetValue(ItemContainerStyleProperty);
            set => SetValue(ItemContainerStyleProperty, value);
        }

        public static readonly DependencyProperty IsControlFocusedProperty =
            DependencyProperty.Register(nameof(IsControlFocused), typeof(bool), typeof(ItemsControlEx),
                new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        public bool IsControlFocused {
            get => (bool)GetValue(IsControlFocusedProperty);
            set => SetValue(IsControlFocusedProperty, value);
        }
    }
}