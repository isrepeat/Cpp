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


namespace HelpersV4._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs : EventArgs {
        public HelpersV4.Collections.IMultiStateElement PreviousState { get; }
        public MultiStateElementEnabledEventArgs(
            HelpersV4.Collections.IMultiStateElement previousState
            ) {
            this.PreviousState = previousState;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs : EventArgs {
        public HelpersV4.Collections.IMultiStateElement NextState { get; }
        public MultiStateElementDisabledEventArgs(
            HelpersV4.Collections.IMultiStateElement nextState
            ) {
            this.NextState = nextState;
        }
    }
}



namespace HelpersV4.Collections {
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



    public interface IMultiStateElement {
        void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e);
        void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e);
    }

    public class UnknownMultiStateElement : IMultiStateElement {
        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }
    }



    public abstract class MultiStateContainerBase<TCommonState> :
        IDisposable
        where TCommonState : ICommonState {
        
        public event System.Action? StateChanged;


        private IMultiStateElement _current;
        public IMultiStateElement Current => _current;

        public Dictionary<Type, string> InstancesHashMap { get; } = new();
        public HashSet<string> InstancesHashSet { get; } = new();
        public string CurrentHash { get; private set; }


        protected readonly TCommonState _commonState;
        protected readonly Dictionary<Type, IMultiStateElement> _instances = new();


        protected MultiStateContainerBase(TCommonState commonState) {
            _commonState = commonState;
            
            var defaultElement = new UnknownMultiStateElement();
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


        public void SwitchTo<T>() where T : IMultiStateElement {
            if (!_instances.TryGetValue(typeof(T), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            var previous = _current;
            _current = next;
            
            this.CurrentHash = this.InstancesHashMap[typeof(T)];
            this.StateChanged?.Invoke();

            previous.OnStateDisabled(new HelpersV4._EventArgs.MultiStateElementDisabledEventArgs(next));
            _current.OnStateEnabled(new HelpersV4._EventArgs.MultiStateElementEnabledEventArgs(previous));
        }


        public T As<T>() where T : class, IMultiStateElement {
            if (_current is T typed) {
                return typed;
            }

            throw new InvalidOperationException(
                $"The current state is not of type '{typeof(T).Name}'. Actual type: '{_current.GetType().Name}'");
        }


        public TViewModel AsViewModel<TViewModel>()
            where TViewModel : CommonStateViewModelBase<TCommonState> {

            if (this._current is TViewModel typed) {
                return typed;
            }

            throw new InvalidOperationException(
                $"The current state is not of type '{typeof(TViewModel).Name}'. Actual type: '{this._current.GetType().Name}'");
        }


        public void ForEachOther(System.Action<IMultiStateElement> action) {
            foreach (var state in _instances.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        public override bool Equals(object? obj) {
            if (obj is not MultiStateContainerBase<TCommonState> other) {
                return false;
            }
            return EqualityComparer<TCommonState>.Default.Equals(_commonState, other._commonState);
        }


        public override int GetHashCode() {
            return EqualityComparer<TCommonState>.Default.GetHashCode(_commonState!);
        }


        // ToString делегируется в _current.
        public override string ToString() {
            return _current?.ToString() ?? base.ToString()!;
        }


        protected void Register<T>(IMultiStateElement element) 
            where T : IMultiStateElement {

            _instances[typeof(T)] = element;

            this.InstancesHashMap[typeof(T)] = $"0x{RuntimeHelpers.GetHashCode(element):X8}";
            this.InstancesHashSet.Add(this.InstancesHashMap[typeof(T)]);
        }
    }



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
}