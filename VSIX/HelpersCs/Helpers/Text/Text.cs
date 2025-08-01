using System;
using System.Linq;


namespace Helpers.Text {
    namespace Ex {
        public static class TextExtensions {
            public static bool ex_IsGuidName(this string str) {
                return Guid.TryParse(str, out _);
            }
        }
    }
}