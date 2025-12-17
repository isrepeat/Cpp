using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace Helpers.Controls {
    public partial class FormattedTextBlock : Helpers.BaseUserControl {
        public string Format {
            get => (string)GetValue(FormatProperty);
            set => SetValue(FormatProperty, value);
        }
        public static readonly DependencyProperty FormatProperty =
        DependencyProperty.Register(
            nameof(Format),
            typeof(string),
            typeof(FormattedTextBlock),
            new PropertyMetadata("{0}", OnAnyValueChanged));

        public object? Value0 {
            get => GetValue(Value0Property);
            set => SetValue(Value0Property, value);
        }
        public static readonly DependencyProperty Value0Property =
            DependencyProperty.Register(
                nameof(Value0),
                typeof(object),
                typeof(FormattedTextBlock),
                new PropertyMetadata(null, OnAnyValueChanged));


        public object? Value1 {
            get => GetValue(Value1Property);
            set => SetValue(Value1Property, value);
        }
        public static readonly DependencyProperty Value1Property =
            DependencyProperty.Register(
                nameof(Value1),
                typeof(object),
                typeof(FormattedTextBlock),
                new PropertyMetadata(null, OnAnyValueChanged));


        public object? Value2 {
            get => GetValue(Value2Property);
            set => SetValue(Value2Property, value);
        }
        public static readonly DependencyProperty Value2Property =
            DependencyProperty.Register(
                nameof(Value2),
                typeof(object),
                typeof(FormattedTextBlock),
                new PropertyMetadata(null, OnAnyValueChanged));


        public string FormattedText {
            get => (string)GetValue(FormattedTextProperty);
            private set => SetValue(FormattedTextProperty, value);
        }
        public static readonly DependencyProperty FormattedTextProperty =
            DependencyProperty.Register(
                nameof(FormattedText),
                typeof(string),
                typeof(FormattedTextBlock),
                new PropertyMetadata(""));


        public FormattedTextBlock() {
            this.InitializeComponent();
        }

        private static void OnAnyValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            ((FormattedTextBlock)d).UpdateFormattedText();
        }

        private void UpdateFormattedText() {
            try {
                this.FormattedText = string.Format(
                    this.Format ?? "{0}",
                    this.Value0,
                    this.Value1,
                    this.Value2);
            }
            catch (FormatException) {
                this.FormattedText = "[Format Error]";
            }
        }
    }
}