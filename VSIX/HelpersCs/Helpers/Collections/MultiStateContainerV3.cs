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


namespace HelpersV3._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs : EventArgs {
        public HelpersV3.Collections.IMultiStateElement PreviousState { get; }
        public MultiStateElementEnabledEventArgs(
            HelpersV3.Collections.IMultiStateElement previousState
            ) {
            this.PreviousState = previousState;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs : EventArgs {
        public HelpersV3.Collections.IMultiStateElement NextState { get; }
        public HelpersV3.Collections.IMultiStateElement? OldNextState { get; }
        public MultiStateElementDisabledEventArgs(
            HelpersV3.Collections.IMultiStateElement nextState,
            HelpersV3.Collections.IMultiStateElement? oldNextState
            ) {
            this.NextState = nextState;
            this.OldNextState = oldNextState;
        }
    }
}



namespace HelpersV3.Collections {
    public interface ICommonState {
    }

    public abstract class CommonStateBase : ICommonState {
        public event PropertyChangedEventHandler? SharedStatePropertyChanged;

        protected void OnSharedStatePropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.SharedStatePropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }


    public abstract class CommonStateViewModelBase<TCommon> :
        Helpers.ObservableObject,
        IDisposable
        where TCommon : ICommonState {

        protected TCommon CommonState { get; }

        protected CommonStateViewModelBase(TCommon commonState) {
            this.CommonState = commonState;
            if (commonState is CommonStateBase observable) {
                observable.SharedStatePropertyChanged += this.OnSharedStatePropertyChanged;
            }
        }

        public virtual void Dispose() {
            if (this.CommonState is CommonStateBase observable) {
                observable.SharedStatePropertyChanged -= this.OnSharedStatePropertyChanged;
            }
        }

        protected virtual void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) { }
    }



    public interface IMultiStateElement {
        void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e);
        void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e);
    }

    public class UnknownMultiStateElement : IMultiStateElement {
        public void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e) {
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

        protected MultiStateContainerBase(TCommonState commonState) {
            _commonState = commonState;
            _current = new UnknownMultiStateElement();
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

            this.StateChanged?.Invoke();

            var disabledEventArgs = new HelpersV3._EventArgs.MultiStateElementDisabledEventArgs(
                next,
                null);

            var enabledEventArgs = new HelpersV3._EventArgs.MultiStateElementEnabledEventArgs(
                previous);

            previous.OnStateDisabled(disabledEventArgs);
            _current.OnStateEnabled(enabledEventArgs);
        }


        public void SwitchToNew<T>(T newNextObj)
            where T : IMultiStateElement {

            if (!_instances.TryGetValue(typeof(T), out var oldNextObj)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            if (newNextObj == null || !object.ReferenceEquals(newNextObj, oldNextObj)) {
                throw new InvalidOperationException($"newNextObj is not in valid state");
            }

            if (oldNextObj is IDisposable oldNextDisposable) {
                oldNextDisposable.Dispose();
            }
            _instances[typeof(T)] = newNextObj; // replace old ref with new in dictionary

            var previous = _current;
            _current = newNextObj;
            
            this.StateChanged?.Invoke();

            var disabledEventArgs = new HelpersV3._EventArgs.MultiStateElementDisabledEventArgs(
                newNextObj,
                oldNextObj);

            var enabledEventArgs = new HelpersV3._EventArgs.MultiStateElementEnabledEventArgs(
                previous);

            previous.OnStateDisabled(disabledEventArgs);
            _current.OnStateEnabled(enabledEventArgs);
        }


        public T? Get<T>() where T : class, IMultiStateElement {
            if (_instances.TryGetValue(typeof(T), out var state)) {
                return (T)state;
            }

            System.Diagnostics.Debugger.Break();
            return null;
        }


        public void ForEachOther(System.Action<IMultiStateElement> action) {
            foreach (var state in _instances.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        protected void Register<T>(IMultiStateElement element) where T : IMultiStateElement {
            _instances[typeof(T)] = element;
        }


        protected static T CreateInstance<T>(params object[] args) where T : class, IMultiStateElement {
            var argTypes = args.Select(a => a.GetType()).ToArray();

            var ctor = typeof(T).GetConstructor(argTypes);
            if (ctor == null) {
                var signature = string.Join(", ", argTypes.Select(t => t.Name));
                throw new InvalidOperationException($"Type {typeof(T).Name} must have constructor ({signature})");
            }

            return (T)ctor.Invoke(args);
        }
    }



    public class MultiStateContainer<TCommonState, A, B> : MultiStateContainerBase<TCommonState>
        where TCommonState : class, ICommonState, new()
        where A : class, IMultiStateElement
        where B : class, IMultiStateElement {

        public MultiStateContainer() : base(new TCommonState()) {
            var a = MultiStateContainerBase<TCommonState>.CreateInstance<A>(base._commonState);
            var b = MultiStateContainerBase<TCommonState>.CreateInstance<B>(base._commonState);

            base.Register<A>(a);
            base.Register<B>(b);
        }
    }
}