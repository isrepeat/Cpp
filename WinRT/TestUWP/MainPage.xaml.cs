﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
//using WinRtLibrary;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace TestUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            new Test_WinRT_Component_2.WinRtLibrary.Hello();
            //var instance1 = Class3_Singleton.CreateInstance(1, 3.14f);
            //var instance2 = Class3_Singleton.CreateInstance(7, 0.99f);
            //var instance3 = Class3_Singleton.GetInstance();
            //var instance4 = Class3_Singleton.GetInstance();


            //instance4.Dispose();
            ////using (var lk = instance.LockScoped())
            ////{
            ////    int xxx = 1;
            ////}

            int xx = 9;
        }
    }
}
