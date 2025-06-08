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
    } // namespace __Local


   

    public partial class MainWindow_4_TextEditorAnchorMenu : Window {

        public MainWindow_4_TextEditorAnchorMenu() {
            this.InitializeComponent();
            this.DataContext = this;
        }
    }
}