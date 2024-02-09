namespace TestWPF {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    //using LoggerCLR;


    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        public MainWindow() {
            InitializeComponent();
            //Logger.Init("D:\\TestLogger_WPF.log", Logger.InitFlags.DefaultFlags | Logger.InitFlags.CreateInPackageFolder);
            MyFunc();
        }

        void MyFunc() {
            //Logger.LogFunctionEnter("MyFunc()");
            //Logger.Log(Logger.Pattern.Raw, Logger.Level.Info, "test raw log ...");
            //Logger.LogDebug("Отладка log");
            //Logger.LogError("Error log");
        }
    }
}
