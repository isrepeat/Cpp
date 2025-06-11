using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace WpfTestApp.Controls {
    public class TextLineViewModel : Helpers.ObservableObject {
        public int LineNumber { get; set; }

        public string Text { get; set; } = "";

        private bool _isHighlighted = false;
        public bool IsHighlighted {
            get => _isHighlighted;
            set {
                if (_isHighlighted != value) {
                    _isHighlighted = value;
                    OnPropertyChanged();
                }
            }
        }
    }


    public partial class AnchorTextViewerControl : Helpers.BaseUserControl {
        private ObservableCollection<TextLineViewModel> _lines = new();
        public ObservableCollection<TextLineViewModel> Lines {
            get => _lines;
            set {
                if (_lines != value) {
                    _lines = value;
                    OnPropertyChanged();
                }
            }
        }

        private readonly List<TextBlock> _lineBlocks = new();
        private readonly List<Border> _lineBorders = new();

        public AnchorTextViewerControl() {
            this.InitializeComponent();
            this.DataContext = this;
        }



        public void DisplayText(string text) {
            this.Lines.Clear();
            var split = text.Replace("\r", "").Split('\n');

            for (int i = 0; i < split.Length; i++) {
                this.Lines.Add(new TextLineViewModel {
                    LineNumber = i + 1,
                    Text = split[i],
                    IsHighlighted = false
                });
            }
        }

        public void ScrollToLine(int lineNumber) {
            if (lineNumber < 1 || lineNumber > this.Lines.Count) {
                return;
            }

            // Сбросить прошлое выделение
            foreach (var line in this.Lines) {
                line.IsHighlighted = false;
            }

            var target = this.Lines[lineNumber - 1];
            target.IsHighlighted = true;

            var container = this.TextItemsControl.ItemContainerGenerator.ContainerFromIndex(lineNumber - 1) as FrameworkElement;
            container?.BringIntoView();
        }

        public void ScrollToLineWithOffset(int lineNumber, double offset = 0.0) {
            if (lineNumber < 1 || lineNumber > this.Lines.Count) {
                return;
            }

            // Сбросить прошлое выделение
            foreach (var line in this.Lines) {
                line.IsHighlighted = false;
            }

            var target = this.Lines[lineNumber - 1];
            target.IsHighlighted = true;

            var container = this.TextItemsControl.ItemContainerGenerator.ContainerFromIndex(lineNumber - 1) as FrameworkElement;
            if (container == null) {
                return;
            }

            // Получаем ScrollViewer
            var scrollViewer = Helpers.VisualTree.FindParentByType<ScrollViewer>(this.TextItemsControl);
            if (scrollViewer == null) {
                return;
            }

            // Вычисляем координаты элемента относительно ScrollViewer
            var relativePosition = container.TransformToAncestor(scrollViewer)
                                            .Transform(new Point(0, 0));

            double currentOffset = scrollViewer.VerticalOffset;
            double targetOffset = currentOffset + relativePosition.Y - offset;

            scrollViewer.ScrollToVerticalOffset(targetOffset);
        }

        private void OnBlockMouseWheelBubbling(object sender, MouseWheelEventArgs e) {
            e.Handled = true;
        }
    }
}