#define ENABLE_VERBOSE_LOGGING
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

//using Local = WpfTestApp.__Local;
using Local = Helpers;

namespace WpfTestApp {
    namespace __Local {
        namespace Enums {
            public enum AnchorLevel {
                Section,
                Subsection,
                Separator,
            }

        }
        
        public class AnchorPoint {
            public string Title { get; set; }
            public int LineNumber { get; set; }
            public Enums.AnchorLevel Level { get; set; }

            public AnchorPoint(string title, int lineNumber, Enums.AnchorLevel level = Enums.AnchorLevel.Subsection) {
                this.Title = title;
                this.LineNumber = lineNumber;
                this.Level = level;
            }
        }


        public static class AnchorParser {
            public class SourceLine {
                public int Index { get; }
                public string Text { get; }

                public SourceLine(int index, string text) {
                    this.Index = index;
                    this.Text = text;
                }
            }

            public static List<AnchorPoint> ParseLinesWithContextWindow(List<string> lines) {
                var result = new List<AnchorPoint>();
                int i = 0;

                while (i < lines.Count) {
                    string trimmed = lines[i].TrimStart();
                    if (!trimmed.StartsWith("// ░")) {
                        i++;
                        continue;
                    }

                    var context = BuildContext(lines, i, linesAfter: 3);
                    var anchor = TryParseAnchor(context, out int linesConsumed);

                    if (anchor != null) {
                        result.Add(anchor);
                        i += linesConsumed;
                    }
                    else {
                        i++;
                    }
                }

                return result;
            }

            private static List<SourceLine> BuildContext(List<string> lines, int startIndex, int linesAfter) {
                var context = new List<SourceLine>();

                int end = Math.Min(lines.Count - 1, startIndex + linesAfter);
                for (int i = startIndex; i <= end; i++) {
                    context.Add(new SourceLine(i, lines[i]));
                }

                return context;
            }

            private static AnchorPoint? TryParseAnchor(List<SourceLine> contextLines, out int linesConsumed) {
                linesConsumed = 1;

                if (contextLines.Count == 0) {
                    return null;
                }

                var first = contextLines[0];
                string line = first.Text.TrimStart();

                if (!line.StartsWith("// ░")) {
                    return null;
                }

                // Section: если есть подчёркивание в следующей строке
                if (contextLines.Count >= 2) {
                    string second = contextLines[1].Text.TrimStart();
                    if (second.StartsWith("// ░░░")) {
                        string title = line.TrimStart('/', ' ', '░').Trim();
                        if (!string.IsNullOrEmpty(title)) {
                            linesConsumed = 2;

                            // используем first.Index + 1, так как в редакторе строки 1-based (первая строка — это 1, не 0)
                            return new AnchorPoint(title, first.Index + 1, Enums.AnchorLevel.Section);
                        }
                    }
                }

                // Subsection (если без подчёркивания)
                string title2 = line.TrimStart('/', ' ', '░').Trim();
                if (!string.IsNullOrEmpty(title2)) {
                    return new AnchorPoint(title2, first.Index + 1, Enums.AnchorLevel.Subsection);
                }

                return null;
            }

            public static List<AnchorPoint> InsertSeparators(List<AnchorPoint> anchors) {
                var result = new List<AnchorPoint>();

                for (int i = 0; i < anchors.Count; i++) {
                    var current = anchors[i];
                    result.Add(current);

                    bool isLast = i == anchors.Count - 1;
                    bool nextIsSection = !isLast && anchors[i + 1].Level == Enums.AnchorLevel.Section;

                    if (current.Level == Enums.AnchorLevel.Subsection && nextIsSection) {
                        result.Add(new AnchorPoint(string.Empty, -1, Enums.AnchorLevel.Separator));
                    }
                }

                return result;
            }
        }

    } // namespace __Local



    public partial class MainWindow_4_TextEditorAnchorMenu : Window {
        
        public MainWindow_4_TextEditorAnchorMenu() {
            this.InitializeComponent();
            this.__AnchorTextViewerControl.DisplayText(System.IO.File.ReadAllText("Controls\\4\\TestCodeFile.rs.txt"));
            this.__TextEditorOverlayControl.LoadAnchorsFromText(System.IO.File.ReadAllText("Controls\\4\\TestCodeFile.rs.txt"));
            this.__TextEditorOverlayControl.OnNavigateToLine = this.OnNavigateToLine;
            this.DataContext = this;
        }

        private void OnNavigateToSmth(object sender, RoutedEventArgs e) {
            this.OnNavigateToLine(52);
        }

        private void OnNavigateToLine(int lineNumber) {
            this.__AnchorTextViewerControl.ScrollToLineWithOffset(lineNumber, 100);
        }
    }
}