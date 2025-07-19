namespace Helpers.Attributes {
    namespace Markers {
        namespace Access {
            public enum Get { _ }
            public enum Set { _ }
            public enum PrivateSet { _ }
        }
    }
    public static class AccessMarker {
        public const Markers.Access.Get Get = Markers.Access.Get._;
        public const Markers.Access.Set Set = Markers.Access.Set._;
        public const Markers.Access.PrivateSet PrivateSet = Markers.Access.PrivateSet._;
    }


    [System.AttributeUsage(System.AttributeTargets.Field)]
    public sealed class ObservablePropertyAttribute : System.Attribute {
        public ObservablePropertyAttribute(
            Markers.Access.Get p0 = default,
            Markers.Access.Set p1 = default
            ) {
        }
        public ObservablePropertyAttribute(
            Markers.Access.Get p0,
            Markers.Access.PrivateSet p1
            ) {
        }
    }


    [System.AttributeUsage(System.AttributeTargets.Field)]
    public sealed class InvalidatablePropertyAttribute : System.Attribute {
        public InvalidatablePropertyAttribute(
            Markers.Access.Get p0 = default,
            Markers.Access.Set p1 = default
            ) {
        }
        public InvalidatablePropertyAttribute(
            Markers.Access.Get p0,
            Markers.Access.PrivateSet p1
            ) {
        }
        public InvalidatablePropertyAttribute(
            Markers.Access.Get p0
            ) {
        }
        //public InvalidatablePropertyAttribute(
        //    Markers.Access.PrivateSet p0
        //    ) {
        //}
    }


    [System.AttributeUsage(System.AttributeTargets.Field)]
    public sealed class InvalidatableLazyPropertyAttribute : System.Attribute {
        public InvalidatableLazyPropertyAttribute(
            string factoryMethodName,
            Markers.Access.Get p1 = default
            ) {
        }
        public InvalidatableLazyPropertyAttribute(
            Markers.Access.Get p0,
            string factoryMethodName
            ) {
        }
    }
}