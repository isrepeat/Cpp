using System;
using System.Windows;
using System.Windows.Data;

namespace Helpers {
    public static class FocusWatcherBehavior {
        public static readonly DependencyProperty WatchKeyProperty =
            DependencyProperty.RegisterAttached(
                "WatchKey",
                typeof(string),
                typeof(FocusWatcherBehavior),
                new PropertyMetadata(null, OnWatchKeyChanged));

        public static string GetWatchKey(DependencyObject obj) {
            return (string)obj.GetValue(WatchKeyProperty);
        }

        public static void SetWatchKey(DependencyObject obj, string value) {
            obj.SetValue(WatchKeyProperty, value);
        }

        private static void OnWatchKeyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) {
            if (d is FrameworkElement fe && e.NewValue is string watchKey) {
                RoutedEventHandler loadedHandler = null!;
                loadedHandler = (s, _) => {
                    fe.Loaded -= loadedHandler;

                    // Регистрируем в Watcher, чтобы обновлялся Metadata
                    FocusWatcher.RegisterFocusGot(watchKey, fe, () => { });
                    FocusWatcher.RegisterFocusLost(watchKey, fe, () => { });
                };

                if (fe.IsLoaded) {
                    loadedHandler(fe, new RoutedEventArgs());
                }
                else {
                    fe.Loaded += loadedHandler;
                }
            }
        }
    }
}