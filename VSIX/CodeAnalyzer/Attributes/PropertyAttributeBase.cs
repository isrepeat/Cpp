namespace CodeAnalyzer.Attributes {
    public enum GetterAccess {
        None,
        Get,
    }

    public enum SetterAccess {
        None,
        Set,
        PrivateSet,
    }

    public abstract class PropertyAttributeBase {
        public GetterAccess GetterAccess { get; protected set; } = GetterAccess.None;
        public SetterAccess SetterAccess { get; protected set; } = SetterAccess.None;
    }
}