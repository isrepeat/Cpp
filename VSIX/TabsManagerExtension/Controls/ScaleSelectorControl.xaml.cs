using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using TabsManagerExtension.VsShell.TextEditor;
using System.Windows.Threading;

namespace TabsManagerExtension.Controls {
    public partial class ScaleSelectorControl : Helpers.BaseUserControl {
        public string Title {
            get { return (string)this.GetValue(TitleProperty); }
            set { this.SetValue(TitleProperty, value); }
        }
        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register(
                nameof(Title),
                typeof(string),
                typeof(ScaleSelectorControl),
                new PropertyMetadata(""));

        public double ScaleFactor {
            get => (double)GetValue(ScaleFactorProperty);
            set => SetValue(ScaleFactorProperty, value);
        }
        public static readonly DependencyProperty ScaleFactorProperty =
            DependencyProperty.Register(
                nameof(ScaleFactor),
                typeof(double),
                typeof(ScaleSelectorControl),
                new PropertyMetadata(1.0));


        public event EventHandler<double> ScaleChanged;

        private TextBox _comboBoxTextBox;
        private double _minScale = 0.5;
        private double _maxScale = 1.5;

        public ScaleSelectorControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
            this.Unloaded += this.OnUnloaded;

            // WARNING:
            // Не присваивай this.DataContext = this — это может вызвать StackOverflow из-за биндингов вроде {Binding Text}.
            // Такие биндинги могут замкнуться на унаследованные свойства Control.Text, Content и т.п.
            // Используй ElementName / RelativeSource или выноси данные в отдельную ViewModel.
        }

        private void OnLoaded(object sender, RoutedEventArgs e) {
            Services.ExtensionServices.BeginUsage();

            this.ScaleComboBox.LostFocus += this.ScaleComboBox_OnLostFocus;
            this.ScaleComboBox.KeyDown += this.ScaleComboBox_OnKeyDown;
            this.ScaleComboBox.SelectionChanged += this.ScaleComboBox_OnSelectionChanged;
            VsShell.TextEditor.Services.TextEditorCommandFilterService.Instance.AddTrackedInputElement(this);

            // Получаем ссылку на текстовое поле внутри ComboBox (editable part)
            _comboBoxTextBox = (TextBox)this.ScaleComboBox.Template.FindName("PART_EditableTextBox", this.ScaleComboBox);
            this.UpdateComboBoxText();
        }

        private void OnUnloaded(object sender, RoutedEventArgs e) {
            VsShell.TextEditor.Services.TextEditorCommandFilterService.Instance.RemoveTrackedInputElement(this);
            this.ScaleComboBox.SelectionChanged -= this.ScaleComboBox_OnSelectionChanged;
            this.ScaleComboBox.KeyDown -= this.ScaleComboBox_OnKeyDown;
            this.ScaleComboBox.LostFocus -= this.ScaleComboBox_OnLostFocus;

            Services.ExtensionServices.EndUsage();
        }


        private void ScaleComboBox_OnLostFocus(object sender, RoutedEventArgs e) {
            this.ApplyScaleFromText();
        }

        private void ScaleComboBox_OnKeyDown(object sender, KeyEventArgs e) {
            if (e.Key == Key.Enter) {
                this.ApplyScaleFromText();
                e.Handled = true;
            }
        }

        private void ScaleComboBox_OnSelectionChanged(object sender, SelectionChangedEventArgs e) {
            if (this.ScaleComboBox.SelectedItem is ComboBoxItem selectedItem && selectedItem.Tag != null) {
                if (double.TryParse(selectedItem.Tag.ToString(), System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out double scaleFactor)) {
                    scaleFactor = Helpers.Math.Clamp(scaleFactor, _minScale, _maxScale);

                    if (Math.Abs(this.ScaleFactor - scaleFactor) > 0.001) {
                        this.ScaleFactor = scaleFactor;
                        this.ScaleChanged?.Invoke(this, this.ScaleFactor);
                        this.UpdateComboBoxText();
                    }
                }
            }
        }

        private void ApplyScaleFromText() {
            if (_comboBoxTextBox != null) {
                string input = _comboBoxTextBox.Text.Replace("%", "").Trim();

                if (double.TryParse(input, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out double scaleValue)) {
                    scaleValue = Helpers.Math.Clamp(scaleValue / 100.0, _minScale, _maxScale);

                    if (Math.Abs(this.ScaleFactor - scaleValue) > 0.001) {
                        this.ScaleFactor = scaleValue;
                        this.ScaleChanged?.Invoke(this, this.ScaleFactor);
                        this.UpdateComboBoxText();
                    }
                }
                else {
                    this.UpdateComboBoxText();
                }

                this.ScaleComboBox.SelectedItem = null;
            }
        }

        private void UpdateComboBoxText() {
            if (_comboBoxTextBox != null) {
                string newText = (this.ScaleFactor * 100).ToString("F0") + " %";

                if (_comboBoxTextBox.Text != newText) {
                    _comboBoxTextBox.Text = newText;

                    // Ставим каретку перед символом '%'
                    int caretPos = newText.LastIndexOf('%');
                    if (caretPos > 0) {
                        _comboBoxTextBox.CaretIndex = caretPos - 1; // перед пробелом
                    }
                }
            }
        }
    }
}