using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace VideoPlayerPrototype {
    public static class Util
    {
        public static async void DoLater(this Page page, DispatchedHandler action)
        {
            await page.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, action);
        }
    }
}
