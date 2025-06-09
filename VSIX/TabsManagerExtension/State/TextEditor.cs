using System;
using System.Collections.Generic;

namespace TabsManagerExtension {
    namespace Enums {
        public enum AnchorKind {
            Section,
            Subsection,
            Separator,
        }
    }
}

namespace TabsManagerExtension.State.TextEditor {
    public class AnchorPoint {
        public string Title { get; set; }
        public int LineNumber { get; set; }
        public Enums.AnchorKind AnchorKind { get; set; }

        public AnchorPoint(string title, int lineNumber, Enums.AnchorKind kind = Enums.AnchorKind.Subsection) {
            this.Title = title;
            this.LineNumber = lineNumber;
            this.AnchorKind = kind;
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
                        return new AnchorPoint(title, first.Index + 1, Enums.AnchorKind.Section);
                    }
                }
            }

            // Subsection (если без подчёркивания)
            string title2 = line.TrimStart('/', ' ', '░').Trim();
            if (!string.IsNullOrEmpty(title2)) {
                return new AnchorPoint(title2, first.Index + 1, Enums.AnchorKind.Subsection);
            }

            return null;
        }

        public static List<AnchorPoint> InsertSeparators(List<AnchorPoint> anchors) {
            var result = new List<AnchorPoint>();

            for (int i = 0; i < anchors.Count; i++) {
                var current = anchors[i];
                result.Add(current);

                bool isLast = i == anchors.Count - 1;
                bool nextIsSection = !isLast && anchors[i + 1].AnchorKind == Enums.AnchorKind.Section;

                if (current.AnchorKind == Enums.AnchorKind.Subsection && nextIsSection) {
                    result.Add(new AnchorPoint(string.Empty, -1, Enums.AnchorKind.Separator));
                }
            }

            return result;
        }
    }
}