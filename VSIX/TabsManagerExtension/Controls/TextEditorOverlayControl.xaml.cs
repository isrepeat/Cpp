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
using Microsoft.VisualStudio.Shell;
using TabsManagerExtension.State.TextEditor;


namespace TabsManagerExtension.Controls {
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

        public ObservableCollection<AnchorPoint> Anchors { get; } = new();

        private AnchorPoint? _selectedAnchor;
        public AnchorPoint? SelectedAnchor {
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
            //VsShell.TextEditor.Services.TextEditorCommandFilterService.Instance.AddTrackedInputElement(this);

            // IsHitTestVisible могут быть унаследованы от родителя (например, AdornerLayer),
            // поэтому значения из XAML не применяются гарантированно — устанавливаем явно в OnLoaded.
            this.IsHitTestVisible = true;

            this.LoadAnchorsFromActiveDocument();
        }

        private void OnUnloaded(object sender, RoutedEventArgs e) {
            //VsShell.TextEditor.Services.TextEditorCommandFilterService.Instance.RemoveTrackedInputElement(this);
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
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Anchors.Clear();

            if (PackageServices.Dte2?.ActiveDocument?.Object("TextDocument") is not EnvDTE.TextDocument textDoc) {
                return;
            }

            var point = textDoc.StartPoint.CreateEditPoint();
            var lines = new List<string>();

            for (int i = 1; i <= textDoc.EndPoint.Line; i++) {
                lines.Add(point.GetLines(i, i + 1));
            }

            var anchors = AnchorParser.ParseLinesWithContextWindow(lines);
            var final = AnchorParser.InsertSeparators(anchors);

            foreach (var anchor in final) {
                this.Anchors.Add(anchor);
            }
        }

        private void NavigateToLine(int lineNumber) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (PackageServices.Dte2?.ActiveDocument?.Object("TextDocument") is EnvDTE.TextDocument textDoc) {
                var selection = textDoc.Selection;

                // Перемещаем каретку на нужную строку (на неё и останется курсор)
                selection.MoveToLineAndOffset(lineNumber, 1);

                // Для скролинга создаем точку выше — с контекстом
                int scrollLine = Math.Max(1, lineNumber - 5); // гарантируем что >= 1
                var scrollPoint = textDoc.CreateEditPoint();
                scrollPoint.MoveToLineAndOffset(scrollLine, 1);

                // Скроллим так, чтобы scrollLine оказался в самом верху
                scrollPoint.TryToShow(EnvDTE.vsPaneShowHow.vsPaneShowTop);
            }
        }
    }
}