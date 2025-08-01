using System;

namespace Helpers {
    public interface IAttachable<T> {
        void Attach(T value);
        void Detach(TypeMarker<T> marker);
    }

    //public static class AttachableExtensions {
    //    public static void Detach<T>(this IAttachable<T> attachable) {
    //        attachable.Detach(TypeMarker<T>.Instance);
    //    }
    //}
}