using System;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Controls;
using System.Reflection;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Threading;

namespace Helpers {


    // NOTE: The "Path" property do ned updated after Hot Reload.
    [MarkupExtensionReturnType(typeof(object))]
    public class FromNearControlExtension : MarkupExtension {
        public string Path { get; set; }
        public BindingMode Mode { get; set; } = BindingMode.OneWay;


        private bool _isLoaded = false;

        public FromNearControlExtension(string path) {
            this.Path = path;
        }

        //public override object ProvideValue(IServiceProvider serviceProvider) {
        //    var binding = new Binding {
        //        Path = new PropertyPath(this.Path),
        //        RelativeSource = new RelativeSource(RelativeSourceMode.FindAncestor, typeof(BaseUserControl), 1),
        //        Mode = this.Mode
        //    };
        //    return binding;
        //}

        public override object ProvideValue(IServiceProvider serviceProvider) {
            if (serviceProvider.GetService(typeof(IProvideValueTarget)) is not IProvideValueTarget targetProvider) {
                return this;
            }

            if (targetProvider.TargetObject is not DependencyObject targetObject ||
                targetProvider.TargetProperty is not DependencyProperty targetProperty) {
                return this;
            }

            // 📌 Откладываем установку Binding до загрузки дерева
            //if (_isLoaded) {
            //var binding = new Binding {
            //    Path = new PropertyPath(this.Path),
            //    RelativeSource = new RelativeSource(RelativeSourceMode.FindAncestor, typeof(Helpers.BaseUserControl), 1),
            //    Mode = this.Mode
            //};
            //BindingOperations.ClearBinding(targetObject, targetProperty);
            //BindingOperations.SetBinding(targetObject, targetProperty, binding);
            //}
            //else {
            _ = targetObject.Dispatcher.BeginInvoke(new System.Action(() => {
                _isLoaded = true;
                var binding = new Binding {
                    Path = new PropertyPath(this.Path),
                    RelativeSource = new RelativeSource(RelativeSourceMode.FindAncestor, typeof(Helpers.BaseUserControl), 1),
                    Mode = this.Mode
                };
                BindingOperations.ClearBinding(targetObject, targetProperty);
                BindingOperations.SetBinding(targetObject, targetProperty, binding);
            }), DispatcherPriority.Loaded);
            //}

            return DependencyProperty.UnsetValue;
        }
    }
}