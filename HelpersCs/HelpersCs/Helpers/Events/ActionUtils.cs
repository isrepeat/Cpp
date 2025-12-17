using System;
using System.Reflection;


namespace Helpers {
    namespace Events {
        public static class ActionUtils {
            public static void ClearSubscribers(object target) {
                if (target == null) {
                    throw new ArgumentNullException(nameof(target));
                }

                var type = target.GetType();
                var flags = BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public;

                foreach (var field in type.GetFields(flags)) {
                    if (typeof(IDisposableEvent).IsAssignableFrom(field.FieldType)) {
                        var value = field.GetValue(target);
                        if (value is IDisposableEvent disposable) {
                            disposable.Dispose();
                        }
                    }
                }
            }
        }
    }
}