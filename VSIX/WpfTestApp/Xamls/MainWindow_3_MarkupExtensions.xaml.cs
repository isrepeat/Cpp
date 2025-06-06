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

using Local = WpfTestApp.__Local;
//using Local = Helpers;

namespace WpfTestApp {
    namespace __Local {
    
    } // namespace __Local


    public partial class MainWindow_3_MarkupExtensions : Window {
        public event PropertyChangedEventHandler? PropertyChanged;

        public MainWindow_3_MarkupExtensions() {
            this.InitializeComponent();

            this.DataContext = this;
        }
    }
}