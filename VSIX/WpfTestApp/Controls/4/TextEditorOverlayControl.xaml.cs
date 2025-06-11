using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using Local = WpfTestApp.__Local;
using WpfTestApp.__Local;

namespace WpfTestApp.Controls {
    public partial class TextEditorOverlayControl : Helpers.BaseUserControl {
        public Action<int>? OnNavigateToLine;

        private Helpers.VisibilityProperty _isAnchorToggleButtonVisible = new();
        public Helpers.VisibilityProperty IsAnchorToggleButtonVisible {
            get => _isAnchorToggleButtonVisible;
            set {
                if (_isAnchorToggleButtonVisible != value) {
                    _isAnchorToggleButtonVisible = value;
                    this.OnPropertyChanged();
                }
            }
        }

        private Helpers.VisibilityProperty _isAnchorListVisible = new();
        public Helpers.VisibilityProperty IsAnchorListVisible {
            get => _isAnchorListVisible;
            set {
                if (_isAnchorListVisible != value) {
                    _isAnchorListVisible = value;
                    this.OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<Local.AnchorPoint> Anchors { get; } = new();

        private Local.AnchorPoint? _selectedAnchor;
        public Local.AnchorPoint? SelectedAnchor {
            get => _selectedAnchor;
            set {
                if (_selectedAnchor != value) {
                    _selectedAnchor = value;
                    this.OnPropertyChanged();

                    if (_selectedAnchor != null) {
                        this.OnNavigateToLine?.Invoke(_selectedAnchor.LineNumber);
                    }
                }
            }
        }

        public ICommand OnToggleAnchorListCommand { get; }

        public TextEditorOverlayControl() {
            this.InitializeComponent();
            this.Loaded += this.OnLoaded;
            this.Unloaded += this.OnUnloaded;
            this.DataContext = this;

            this.OnToggleAnchorListCommand = new Helpers.RelayCommand<object>(this.OnToggleAnchorList);
        }

        private void OnLoaded(object sender, RoutedEventArgs e) {
            // IsHitTestVisible могут быть унаследованы от родителя (например, AdornerLayer),
            // поэтому значения из XAML не применяются гарантированно — устанавливаем явно в OnLoaded.
            this.IsHitTestVisible = true;
        }

        private void OnUnloaded(object sender, RoutedEventArgs e) {
            this.Anchors.Clear();
        }


        private void OnToggleAnchorList(object parameter) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnToggleAnchorList()");
            this.IsAnchorListVisible.Value = !this.IsAnchorListVisible.Value;

            if (this.IsAnchorListVisible.Value == false) {
                this.SelectedAnchor = null;
            }
        }


        public void LoadAnchorsFromText(string text) {
            this.Anchors.Clear();

            var lines = text.Replace("\r", "").Split('\n').ToList();
            var anchors = AnchorParser.ParseLinesWithContextWindow(lines);
            var final = AnchorParser.InsertSeparators(anchors);

            foreach (var anchor in final) {
                this.Anchors.Add(anchor);
            }
        }

        private void NavigateToLine(int lineNumber) {
        }
    }
}