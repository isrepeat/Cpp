using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace Helpers._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs : EventArgs {
        public Helpers.MultiState.IMultiStateElement PreviousState { get; }
        public object? UpdatePackageObj { get; }
        public MultiStateElementEnabledEventArgs(
            Helpers.MultiState.IMultiStateElement previousState,
            object? updatePackageObj = null
            ) {
            this.PreviousState = previousState;
            this.UpdatePackageObj = updatePackageObj;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs : EventArgs {
        public Helpers.MultiState.IMultiStateElement NextState { get; }
        public MultiStateElementDisabledEventArgs(
            Helpers.MultiState.IMultiStateElement nextState
            ) {
            this.NextState = nextState;
        }
    }
}



namespace Helpers.MultiState {
    public interface IMultiStateElement {
        void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e);
        void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e);
    }

    public class UnknownMultiStateElement : IMultiStateElement {
        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs e) {
        }
    }



    public abstract class MultiStateContainerBase<TCommonState> :
        IDisposable
        where TCommonState : ICommonState {
        
        public event System.Action? StateChanged;


        private IMultiStateElement _current;
        public IMultiStateElement Current => _current;


        protected readonly TCommonState _commonState;
        protected readonly Dictionary<Type, IMultiStateElement> _instances = new();

        //private readonly IMultiStateEntry<TCommonState>? _multiStateEntry;


        protected MultiStateContainerBase(
            TCommonState commonState
            ) {
            _commonState = commonState;

            var defaultElement = new UnknownMultiStateElement();
            _current = defaultElement;
        }

        //protected MultiStateContainerBase(
        //    TCommonState commonState,
        //    IMultiStateEntry<TCommonState> multiStateEntry
        //    ) {
        //    _commonState = commonState;
        //    _multiStateEntry = multiStateEntry;

        //    var defaultElement = new UnknownMultiStateElement();
        //    _current = defaultElement;
        //}


        public void Dispose() {
            foreach (var state in _instances.Values) {
                if (state is IDisposable stateDisposable) {
                    stateDisposable.Dispose();
                }
            }

            if (_commonState is IDisposable sharedStateDisposable) {
                sharedStateDisposable.Dispose();
            }
        }


        public void SwitchTo<TState>(object? updatePackageObj = null) 
            where TState : IMultiStateElement {

            if (!_instances.TryGetValue(typeof(TState), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(TState).Name} found.");
            }

            var previous = _current;
            _current = next;
            
            this.StateChanged?.Invoke();

            previous.OnStateDisabled(new Helpers._EventArgs.MultiStateElementDisabledEventArgs(next));
            _current.OnStateEnabled(new Helpers._EventArgs.MultiStateElementEnabledEventArgs(previous, updatePackageObj));
        }


        //public void UpdateAndSwitchTo<TUpdateParams, TState>(TUpdateParams updateParams)
        //    where TState : IMultiStateElement {

        //    Helpers.ThrowableAssert.Require(_commonState is IDynamicCommonState<TUpdateParams>);

        //    if (!_instances.TryGetValue(typeof(TState), out var next)) {
        //        throw new InvalidOperationException($"No state of type {typeof(TState).Name} found.");
        //    }

        //    if (_commonState is IDynamicCommonState<TUpdateParams> dynamicCommonState) {
        //        dynamicCommonState.UpdateData(updateParams);
        //    }

        //    var previous = _current;
        //    _current = next;

        //    this.StateChanged?.Invoke();

        //    previous.OnStateDisabled(new Helpers._EventArgs.MultiStateElementDisabledEventArgs(next));
        //    _current.OnStateEnabled(new Helpers._EventArgs.MultiStateElementEnabledEventArgs(previous));
        //}


        public T As<T>() where T : class, IMultiStateElement {
            if (_current is T typed) {
                return typed;
            }

            throw new InvalidOperationException(
                $"The current state is not of type '{typeof(T).Name}'. Actual type: '{_current.GetType().Name}'");
        }


        public TViewModel AsViewModel<TViewModel>()
            where TViewModel : CommonStateViewModelBase<TCommonState> {

            if (_current is TViewModel typed) {
                return typed;
            }

            throw new InvalidOperationException(
                $"The current state is not of type '{typeof(TViewModel).Name}'. Actual type: '{_current.GetType().Name}'");
        }


        public void ForEachOther(System.Action<IMultiStateElement> action) {
            foreach (var state in _instances.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        public override bool Equals(object? obj) {
            if (obj is MultiStateContainerBase<TCommonState> other) {
                return EqualityComparer<TCommonState>.Default.Equals(_commonState, other._commonState);
            }
            return false;
        }


        public override int GetHashCode() {
            return EqualityComparer<TCommonState>.Default.GetHashCode(_commonState);
        }


        // ToString делегируется в _current.
        public override string ToString() {
            return _current?.ToString() ?? base.ToString()!;
        }


        protected void Register<T>(IMultiStateElement element) 
            where T : IMultiStateElement {

            _instances[typeof(T)] = element;
        }
    }


    // <TCommonState, A, B>
    public class MultiStateContainer<TCommonState, A, B> :
        MultiStateContainerBase<TCommonState>
        where TCommonState : class, ICommonState
        where A : class, IMultiStateElement
        where B : class, IMultiStateElement {

        public MultiStateContainer(TCommonState commonState) 
            : base(commonState) {

            // Создание начальных экземпляров
            var a = (A)Activator.CreateInstance(typeof(A), _commonState)!;
            var b = (B)Activator.CreateInstance(typeof(B), _commonState)!;

            base.Register<A>(a);
            base.Register<B>(b);

        }

        public MultiStateContainer(
            TCommonState commonState,
            Func<TCommonState, A> factoryA,
            Func<TCommonState, B> factoryB
            ) : base(commonState) {

            // Создание начальных экземпляров
            var a = factoryA(_commonState);
            var b = factoryB(_commonState);

            base.Register<A>(a);
            base.Register<B>(b);
        }
    }


    // <TCommonState, A, B, C>
    public class MultiStateContainer<TCommonState, A, B, C> :
        MultiStateContainerBase<TCommonState>
        where TCommonState : class, ICommonState
        where A : class, IMultiStateElement
        where B : class, IMultiStateElement
        where C : class, IMultiStateElement {

        public MultiStateContainer(TCommonState commonState)
            : base(commonState) {

            var a = (A)Activator.CreateInstance(typeof(A), _commonState)!;
            var b = (B)Activator.CreateInstance(typeof(B), _commonState)!;
            var c = (C)Activator.CreateInstance(typeof(C), _commonState)!;

            base.Register<A>(a);
            base.Register<B>(b);
            base.Register<C>(c);
        }

        public MultiStateContainer(
            TCommonState commonState,
            Func<TCommonState, A> factoryA,
            Func<TCommonState, B> factoryB,
            Func<TCommonState, C> factoryC
            ) : base(commonState) {

            var a = factoryA(_commonState);
            var b = factoryB(_commonState);
            var c = factoryC(_commonState);

            base.Register<A>(a);
            base.Register<B>(b);
            base.Register<C>(c);
        }
    }

}