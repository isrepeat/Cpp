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

namespace WpfTestApp.Controls {
    public partial class TextEditorOverlayControl : Helpers.BaseUserControl {
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
                    if (value != null) {
                        this.NavigateToLine(value.LineNumber);
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
            this.LoadAnchorsFromActiveDocument();
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


        public void LoadAnchorsFromActiveDocument() {
            this.Anchors.Clear();

            this.Anchors.Add(new Local.AnchorPoint("Initialization", 10, Local.Enums.AnchorLevel.Section));
            this.Anchors.Add(new Local.AnchorPoint("Load config", 12, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Init services", 13, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Start engine", 14, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint(string.Empty, -1, Local.Enums.AnchorLevel.Separator));

            this.Anchors.Add(new Local.AnchorPoint("Event handlers", 50, Local.Enums.AnchorLevel.Section));
            this.Anchors.Add(new Local.AnchorPoint("Attach events", 52, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Detach events", 53, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Raise events", 54, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint(string.Empty, -1, Local.Enums.AnchorLevel.Separator));

            this.Anchors.Add(new Local.AnchorPoint("Internal logic", 70, Local.Enums.AnchorLevel.Section));
            this.Anchors.Add(new Local.AnchorPoint("Diagnostics", 72, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Validation", 73, Local.Enums.AnchorLevel.Subsection));
            this.Anchors.Add(new Local.AnchorPoint("Execution", 74, Local.Enums.AnchorLevel.Subsection));
        }

        private void NavigateToLine(int lineNumber) {
        }
    }
}