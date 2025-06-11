using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace Helpers.Controls {
    public partial class Separator : Helpers.BaseUserControl {
        public Brush LineColor {
            get => (Brush)this.GetValue(LineColorProperty);
            set => this.SetValue(LineColorProperty, value);
        }
        public static readonly DependencyProperty LineColorProperty =
            DependencyProperty.Register(
                nameof(LineColor),
                typeof(Brush),
                typeof(Separator),
                new FrameworkPropertyMetadata(Brushes.Gray, FrameworkPropertyMetadataOptions.AffectsRender));

        public double LineThickness {
            get => (double)this.GetValue(LineThicknessProperty);
            set => this.SetValue(LineThicknessProperty, value);
        }
        public static readonly DependencyProperty LineThicknessProperty =
            DependencyProperty.Register(
                nameof(LineThickness),
                typeof(double),
                typeof(Separator),
                new FrameworkPropertyMetadata(1.0, FrameworkPropertyMetadataOptions.AffectsRender));


        public Separator() {
            this.InitializeComponent();
        }
    }
}