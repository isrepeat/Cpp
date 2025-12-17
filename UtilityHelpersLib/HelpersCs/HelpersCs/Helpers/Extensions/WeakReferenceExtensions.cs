using System;


namespace Helpers.Ex {
    public static class WeakReferenceExtensions {
        public static T ex_Get<T>(this WeakReference<T>? weak)
            where T : class {

            if (weak is null) {
                throw new InvalidOperationException($"WeakReference<{typeof(T).Name}> is null.");
            }

            if (weak.TryGetTarget(out var target) && target is not null) {
                return target;
            }

            throw new InvalidOperationException($"Target of WeakReference<{typeof(T).Name}> has been collected.");
        }


        public static WeakReference<Derived> ex_CastWeak<Base, Derived>(this WeakReference<Base> weak)
            where Base : class
            where Derived : class, Base {

            if (weak.TryGetTarget(out var target) && target is Derived derived) {
                return new WeakReference<Derived>(derived);
            }

            throw new InvalidOperationException($"Cannot cast WeakReference<{typeof(Base).Name}> to WeakReference<{typeof(Derived).Name}>");
        }
    }
}