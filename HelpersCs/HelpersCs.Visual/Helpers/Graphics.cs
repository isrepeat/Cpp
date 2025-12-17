using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    namespace Ex {
        public static class DpiExtensions {
            public static Point ex_ToDpiAwareScreen(this FrameworkElement element, Point localOffset) {
                var source = PresentationSource.FromVisual(element);
                if (source == null) {
                    Helpers.Diagnostic.Logger.LogError("Element is not connected to a PresentationSource.");
                    return new Point();
                }

                var screenPoint = element.PointToScreen(localOffset);
                var dpiMatrix = source.CompositionTarget?.TransformToDevice ?? Matrix.Identity;

                return new Point(
                    screenPoint.X / dpiMatrix.M11,
                    screenPoint.Y / dpiMatrix.M22
                );
            }
        }
    }
}