using System;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Reflection;


namespace HelpersV5._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs<TCommonState> : EventArgs {
        public HelpersV5.Collections.IMultiStateElement<TCommonState> PreviousState { get; }
        public MultiStateElementEnabledEventArgs(
            HelpersV5.Collections.IMultiStateElement<TCommonState> previousState
            ) {
            this.PreviousState = previousState;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs<TCommonState> : EventArgs {
        public HelpersV5.Collections.IMultiStateElement<TCommonState> NextState { get; }
        public HelpersV5.Collections.IMultiStateElement<TCommonState>? OldNextState { get; }
        public MultiStateElementDisabledEventArgs(
            HelpersV5.Collections.IMultiStateElement<TCommonState> nextState,
            HelpersV5.Collections.IMultiStateElement<TCommonState>? oldNextState
            ) {
            this.NextState = nextState;
            this.OldNextState = oldNextState;
        }
    }
}



namespace HelpersV5.Collections {
    public interface ICommonState {
    }

    public abstract class CommonStateBase : ICommonState {
        public event PropertyChangedEventHandler? CommonStatePropertyChanged;

        protected void OnCommonStatePropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.CommonStatePropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }


    public abstract class CommonStateViewModelBase<TCommon> :
        Helpers.ObservableObject,
        IDisposable
        where TCommon : ICommonState {

        protected TCommon CommonState { get; }

        protected CommonStateViewModelBase(TCommon commonState) {
            this.CommonState = commonState;

            if (this.CommonState is CommonStateBase observable) {
                observable.CommonStatePropertyChanged += this.OnCommonStatePropertyChanged;
            }
        }

        public virtual void Dispose() {
            if (this.CommonState is CommonStateBase observable) {
                observable.CommonStatePropertyChanged -= this.OnCommonStatePropertyChanged;
            }
        }

        protected virtual void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) { }
    }



    public interface IMultiStateElement<TCommonState> {
        void OnConstructed(TCommonState commonState);
        void OnStateEnabled(HelpersV5._EventArgs.MultiStateElementEnabledEventArgs<TCommonState> e);
        void OnStateDisabled(HelpersV5._EventArgs.MultiStateElementDisabledEventArgs<TCommonState> e);
    }

    public class UnknownMultiStateElement<TCommonState> : IMultiStateElement<TCommonState> {
        public void OnConstructed(TCommonState commonState) {
        }
        public void OnStateEnabled(HelpersV5._EventArgs.MultiStateElementEnabledEventArgs<TCommonState> e) {
        }
        public void OnStateDisabled(HelpersV5._EventArgs.MultiStateElementDisabledEventArgs<TCommonState> e) {
        }
    }




    //public readonly struct CommonProperty<TState, TValue> {
    //    public PropertyInfo Property { get; }

    //    public CommonProperty(PropertyInfo prop) {
    //        if (prop.DeclaringType != typeof(TState)) {
    //            throw new ArgumentException($"Property {prop.Name} is not declared in {typeof(TState).Name}");
    //        }

    //        this.Property = prop;
    //    }
    //}



    public abstract class MultiStateContainerBase<TCommonState> :
        IDisposable
        where TCommonState : ICommonState {
        
        public event System.Action? StateChanged;


        private IMultiStateElement<TCommonState> _current;
        public IMultiStateElement<TCommonState> Current => _current;

        public Dictionary<Type, string> InstancesHashMap { get; } = new();
        public HashSet<string> InstancesHashSet { get; } = new();
        public string CurrentHash { get; private set; }


        protected readonly TCommonState _commonState;
        protected readonly Dictionary<Type, IMultiStateElement<TCommonState>> _instances = new();


        protected MultiStateContainerBase(TCommonState commonState) {
            _commonState = commonState;
            
            var defaultElement = new UnknownMultiStateElement<TCommonState>();
            _current = defaultElement;
        }


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


        public void SwitchTo<T>() where T : IMultiStateElement<TCommonState> {
            if (!_instances.TryGetValue(typeof(T), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            var previous = _current;
            _current = next;
            
            this.CurrentHash = this.InstancesHashMap[typeof(T)];
            this.StateChanged?.Invoke();

            previous.OnStateDisabled(new HelpersV5._EventArgs.MultiStateElementDisabledEventArgs<TCommonState>(
                next,
                null));

            _current.OnStateEnabled(new HelpersV5._EventArgs.MultiStateElementEnabledEventArgs<TCommonState>(
                previous));
        }


        public T? Get<T>() where T : class, IMultiStateElement<TCommonState> {
            if (_instances.TryGetValue(typeof(T), out var state)) {
                return (T)state;
            }

            System.Diagnostics.Debugger.Break();
            return null;
        }


        public void ForEachOther(System.Action<IMultiStateElement<TCommonState>> action) {
            foreach (var state in _instances.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        //public void ReadCommonState<TValue>(CommonProperty<TCommonState, TValue> prop, out TValue value) {
        //    value = (TValue)prop.Property.GetValue(_commonState)!;
        //}


        public override bool Equals(object? obj) {
            if (obj is not MultiStateContainerBase<TCommonState> other) {
                return false;
            }
            return EqualityComparer<TCommonState>.Default.Equals(_commonState, other._commonState);
        }

        public override int GetHashCode() {
            return EqualityComparer<TCommonState>.Default.GetHashCode(_commonState!);
        }

        public override string ToString() {
            return _commonState?.ToString() ?? base.ToString()!;
        }


        protected void Register<T>(IMultiStateElement<TCommonState> element) 
            where T : IMultiStateElement<TCommonState> {

            _instances[typeof(T)] = element;

            this.InstancesHashMap[typeof(T)] = $"0x{RuntimeHelpers.GetHashCode(element):X8}";
            this.InstancesHashSet.Add(this.InstancesHashMap[typeof(T)]);
        }
    }



    public class MultiStateContainer<TCommonState, A, B> :
        MultiStateContainerBase<TCommonState>
        where TCommonState : class, ICommonState
        where A : class, IMultiStateElement<TCommonState>
        where B : class, IMultiStateElement<TCommonState> {

        public MultiStateContainer(TCommonState commonState) 
            : base(commonState) {

            // Создание начальных экземпляров
            var a = (A)Activator.CreateInstance(typeof(A))!;
            a.OnConstructed(_commonState);

            var b = (B)Activator.CreateInstance(typeof(B))!;
            b.OnConstructed(_commonState);

            base.Register<A>(a);
            base.Register<B>(b);

        }

        //public MultiStateContainer(
        //    TCommonState commonState,
        //    Func<TCommonState, A> factoryA,
        //    Func<TCommonState, B> factoryB
        //    ) : base(commonState) {

        //    // Создание начальных экземпляров
        //    var a = factoryA(_commonState);
        //    var b = factoryB(_commonState);

        //    base.Register<A>(a);
        //    base.Register<B>(b);
        //}
    }
}