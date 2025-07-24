using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Helpers.Attributes;


namespace WpfTestApp.States.MultistateBehaviour {
    //public abstract class DocumentMultiStateBase :
    //    HelpersV4.Collections.MultiStateContainer<
    //        DocumentCommonState,
    //        Document,
    //        InvalidatedDocument> {

    //    protected DocumentMultiStateBase(
    //        DocumentCommonState documentCommonState
    //        ) : base(documentCommonState) {
    //    }

    //    protected DocumentMultiStateBase(
    //        DocumentCommonState documentCommonState,
    //        Func<DocumentCommonState, Document> factoryA,
    //        Func<DocumentCommonState, InvalidatedDocument> factoryB
    //        ) : base(documentCommonState, factoryA, factoryB) {
    //    }
    //}

    //public abstract class MultiStateAbstract<TCommonState, A, B> :
    //    HelpersV4.Collections.MultiStateContainer<TCommonState, A, B>
    //    where TCommonState : class, HelpersV4.Collections.ICommonState
    //    where A : class, HelpersV4.Collections.IMultiStateElement
    //    where B : class, HelpersV4.Collections.IMultiStateElement {

    //    protected MultiStateAbstract(
    //        TCommonState documentCommonState
    //        ) : base(documentCommonState) {
    //    }

    //    protected MultiStateAbstract(
    //        TCommonState documentCommonState,
    //        Func<TCommonState, A> factoryA,
    //        Func<TCommonState, B> factoryB
    //        ) : base(documentCommonState, factoryA, factoryB) {
    //    }
    //}



    //public abstract class DocumentMultiStateBase : MultiStateAbstract<DocumentCommonState, Document, InvalidatedDocument> {
    //}


    public class DocumentMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument> {
        public DocumentMultiStateElement(string vsHierarchyData)
            : base(new DocumentCommonState(vsHierarchyData)) {
        }
    }

    public class ExteralIncludeMultiStateElement :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            ExternalInclude,
            InvalidatedDocument> {
        public ExteralIncludeMultiStateElement(string vsHierarchyData)
            : base(new DocumentCommonState(vsHierarchyData)) {
        }
    }



    public partial class DocumentCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        [ObservableMultiStateProperty(NotifyMethod = "base.OnCommonStatePropertyChanged")]
        private HelpersV4.Collections.MultiStateContainer<
            HierarchyCommonState,
            HierarchyItem,
            InvalidatedHierarchyItem
            > _hierarchyMultiState;

        public DocumentCommonState(string vsHierarchyData) {
            this.HierarchyMultiState = new(new HierarchyCommonState(vsHierarchyData));
        }

        public void Dispose() {
            this.HierarchyMultiState?.Dispose();
        }
    }



    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<DocumentCommonState> {

        public object HierarchyItemObj => this.CommonState.HierarchyMultiState.Current;
        public string HierarchyItemObjHash => this.CommonState.HierarchyMultiState.CurrentHash;
        public IEnumerable<string> HierarchyItemObjHashes => this.CommonState.HierarchyMultiState.InstancesHashSet.ToList();

        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
            this.UpdateHashes();
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
            if (e.PropertyName is nameof(DocumentCommonState.HierarchyMultiState)) {
                base.OnPropertyChanged(nameof(this.HierarchyItemObj));
                this.UpdateHashes();
            }
        }

        private void UpdateHashes() {
            base.OnPropertyChanged(nameof(this.HierarchyItemObjHash));
            base.OnPropertyChanged(nameof(this.HierarchyItemObjHashes));
        }
    }




    public class Document :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public Document(DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            base.CommonState.HierarchyMultiState.SwitchTo<HierarchyItem>();
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                base.CommonState.HierarchyMultiState.SwitchTo<InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return $"<Document>";
        }
    }


    public class ExternalInclude : Document {
        public ExternalInclude(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }
        public override string ToString() {
            return $"<ExternalInclude>";
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument>";
        }
    }
}





////class A {
////    private int _x;
////    public class B {
////        public B() {
////            var x = new A()._x;
////        }
////    }
////}


////public class Target {
////    private Target() { /* ... */ }  // приватный конструктор

////    public class Manager  // вложенный помощник
////    {
////        public Target CreateInstance() {
////            return new Target();  // допустимо, т.к. внутри Target
////        }
////    }
////}


////public class Owner {
////    // Публичное обращение – через интерфейс ITarget
////    public ITarget CreateTarget() => new TargetImpl();

////    // Закрытая реализация TargetImpl, доступная только внутри Owner
////    private class TargetImpl : ITarget {
////        public TargetImpl() { /* ... */ }
////        // ...реализация методов ITarget...
////    }
////}
////public interface ITarget {
////    // ... методы, доступные внешнему коду ...
////}

//using System;
//using FriendOnlyExample;
//using System.Windows;
//using static FriendOnlyExample.Owner;

//namespace FriendOnlyExample {

//    //public abstract class IInternalKey<TOwner> {
//    //    protected IInternalKey(IPrivateKey<TOwner> privateKey) {
//    //    }
//    //}

//    //public abstract class IPrivateKey<TOwner> {
//    //    public TOwner Owner { get; }
//    //    protected IPrivateKey(TOwner owner) {
//    //        this.Owner = owner;
//    //    }
//    //}



//    public class Target {
//        private Target() {
//            Console.WriteLine("Target created");
//        }

//        public static Target CreateInstance(Owner.InternalKey _) {
//            return new Target();
//        }
//    }



//    public class Owner {
//        public sealed class InternalKey {
//            private InternalKey(Owner.IPrivateKey<Owner.Temp._PrivateKey> privateKey) {
//            }
//            public static InternalKey CreateInstance(Owner.IPrivateKey<Owner.Temp._PrivateKey> privateKey) {
//                return new InternalKey(privateKey);
//            }
//        }

//        public abstract class IPrivateKey<T> {
//            T Value;
//            public IPrivateKey(T value) {
//                Value = value;
//            }
//        }

//        private class Temp {
//            public sealed class _PrivateKey {
//                public _PrivateKey() {
//                }
//            }
//            public static _PrivateKey Create() {
//                return new _PrivateKey();
//            }
//        }

//        private sealed class PrivateKey : IPrivateKey<Owner.Temp._PrivateKey> {
//            public PrivateKey() : base(Temp.Create()) { }
//        }

//        private InternalKey CreateInternalKey() {
//            return InternalKey.CreateInstance(new PrivateKey());
//        }


//        public Target GetTarget() {
//            return Target.CreateInstance(this.CreateInternalKey());
//        }
//    }


//    public class Imposter {
//        public sealed class InternalKey {
//            private InternalKey(Owner.IPrivateKey privateKey) {
//            }
//            public static InternalKey CreateInstance(Owner.IPrivateKey privateKey) {
//                return new InternalKey(privateKey);
//            }
//        }

//        //public abstract class IPrivateKey {
//        //}

//        private sealed class PrivateKey : Owner.IPrivateKey {
//            public PrivateKey() { }
//        }

//        private InternalKey CreateInternalKey() {
//            return InternalKey.CreateInstance(new PrivateKey());
//        }

//        public Target GetTarget() {
//            return Target.CreateInstance(Owner.InternalKey.CreateInstance(new PrivateKey()));
//        }
//    }


//    public static class Program {
//        public static void Main() {
//            var owner = new Owner();
//            var ownerTarget = owner.GetTarget();

//            var imposter = new Imposter();
//            var imposterTarget = imposter.GetTarget();
//        }
//    }
//}





//namespace FriendOnlyExample2 {

//    public class PassKey<TOwner> {
//    }


//    public class Target {
//        private Target() {
//            Console.WriteLine("Target created");
//        }

//        public static Target CreateInstance(Owner.InternalKey<Target> _) {
//            return new Target();
//        }
//    }



//    public class Owner {
//        public sealed class InternalKey<T> {
//            private InternalKey() {
//            }
//            public static InternalKey<T> CreateInstance() {
//                return new InternalKey<T>();
//            }
//        }

//        public Target GetTarget() {
//            return Target.CreateInstance(InternalKey<Target>.CreateInstance());
//        }
//    }


//    public class Imposter {
//        public Target GetTarget() {
//            return Target.CreateInstance(Owner.InternalKey<Target>.CreateInstance());
//        }
//    }


//    public static class Program {
//        public static void Main() {
//            var owner = new Owner();
//            var ownerTarget = owner.GetTarget();

//            var imposter = new Imposter();
//            var imposterTarget = imposter.GetTarget();
//        }
//    }
//}