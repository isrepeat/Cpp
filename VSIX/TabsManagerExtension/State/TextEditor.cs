using System;
using System.Collections.Generic;

namespace TabsManagerExtension.State.TextEditor {
    public class AnchorPoint {
        public string Title { get; set; }
        public int LineNumber { get; set; }

        public AnchorPoint(string title, int lineNumber) {
            this.Title = title;
            this.LineNumber = lineNumber;
        }
    }

    public static class AnchorParser {
        private const string AnchorPrefix = "// #anchor:";

        public static List<AnchorPoint> ParseLines(IEnumerable<string> lines) {
            var result = new List<AnchorPoint>();
            int lineNumber = 1;

            foreach (var line in lines) {
                if (line.TrimStart().StartsWith(AnchorPrefix, StringComparison.OrdinalIgnoreCase)) {
                    string title = line.Trim().Substring(AnchorPrefix.Length).Trim();
                    if (!string.IsNullOrEmpty(title)) {
                        result.Add(new AnchorPoint(title, lineNumber));
                    }
                }

                lineNumber++;
            }

            return result;
        }
    }
}