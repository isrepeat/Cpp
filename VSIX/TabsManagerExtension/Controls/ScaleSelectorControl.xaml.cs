using System;
using System.Windows;
using System.Windows.Controls;

namespace TabsManagerExtension.Controls {
    public partial class ScaleSelectorControl : Helpers.BaseUserControl {
        public static readonly DependencyProperty ScaleFactorProperty =
            DependencyProperty.Register(
                nameof(ScaleFactor),
                typeof(double),
                typeof(ScaleSelectorControl),
                new PropertyMetadata(1.0, OnScaleFactorChanged));

        private TextBox _comboBoxTextBox;

        public double ScaleFactor {
            get => (double)GetValue(ScaleFactorProperty);
            set => SetValue(ScaleFactorProperty, value);
        }

        public event EventHandler<double> ScaleChanged;

        public ScaleSelectorControl() {
            this.InitializeComponent();
        }

        private static void OnScaleFactorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            var control = d as ScaleSelectorControl;
            control?.UpdateComboBoxText();
        }

        private void ScaleComboBox_Loaded(object sender, RoutedEventArgs e) {
            _comboBoxTextBox = (TextBox)this.ScaleComboBox.Template.FindName("PART_EditableTextBox", this.ScaleComboBox);
            this.UpdateComboBoxText();
        }

        private void ScaleComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            if (ScaleComboBox.SelectedItem is ComboBoxItem selectedItem && selectedItem.Tag != null) {
                if (double.TryParse(selectedItem.Tag.ToString(), System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out double scaleFactor)) {
                    this.ScaleFactor = scaleFactor;
                    this.ScaleChanged?.Invoke(this, this.ScaleFactor);
                    this.UpdateComboBoxText();
                }
            }
        }

        private void ScaleComboBox_LostFocus(object sender, RoutedEventArgs e) {
            this.ApplyScaleFromText();
        }

        private void ScaleComboBox_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e) {
            if (e.Key == System.Windows.Input.Key.Enter) {
                this.ApplyScaleFromText();
                e.Handled = true;
            }
        }

        private void ApplyScaleFromText() {
            if (_comboBoxTextBox != null) {
                string input = _comboBoxTextBox.Text.TrimEnd('%').Trim();
                if (double.TryParse(input, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out double scaleValue)) {
                    // Применяем масштаб
                    scaleValue = Helpers.Math.Clamp(scaleValue / 100.0, 0.1, 5.0);
                    this.ScaleFactor = scaleValue;
                    this.ScaleChanged?.Invoke(this, this.ScaleFactor);

                    // Сбрасываем выбор в ComboBox, чтобы он не сохранял выбранный элемент
                    this.ScaleComboBox.SelectedItem = null;
                    this.UpdateComboBoxText();
                }
                else {
                    this.UpdateComboBoxText();
                }
            }
        }


        private void UpdateComboBoxText() {
            if (_comboBoxTextBox != null) {
                _comboBoxTextBox.Text = (ScaleFactor * 100).ToString("F0") + " %";
            }
        }
    }
}