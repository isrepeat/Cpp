using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;


namespace Helpers.Text {
    namespace Ex {
        public static class TextExtensions {
            public static bool ex_IsGuidName(this string str) {
                return Guid.TryParse(str, out _);
            }
        }
    }
}