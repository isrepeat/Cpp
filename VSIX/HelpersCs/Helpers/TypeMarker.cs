using System;

namespace Helpers {
    public readonly struct TypeMarker<T> {
        public static readonly TypeMarker<T> Instance = new TypeMarker<T>();
    }
}