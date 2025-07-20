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


namespace Helpers._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs<TSharedState> : EventArgs {
        public TSharedState SharedState { get; }
        public Helpers.Collections.IMultiStateElement<TSharedState> PreviousState { get; }
        public MultiStateElementEnabledEventArgs(
            TSharedState sharedState,
            Helpers.Collections.IMultiStateElement<TSharedState> previousState
            ) {
            this.SharedState = sharedState;
            this.PreviousState = previousState;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs<TSharedState> : EventArgs {
        public TSharedState SharedState { get; }
        public Helpers.Collections.IMultiStateElement<TSharedState> NextState { get; }
        public Helpers.Collections.IMultiStateElement<TSharedState>? OldNextState { get; }
        public MultiStateElementDisabledEventArgs(
            TSharedState sharedState,
            Helpers.Collections.IMultiStateElement<TSharedState> nextState,
            Helpers.Collections.IMultiStateElement<TSharedState>? oldNextState
            ) {
            this.SharedState = sharedState;
            this.NextState = nextState;
            this.OldNextState = oldNextState;
        }
    }
}



namespace Helpers.Collections {
    public interface IMultiStateElement<TSharedState> {
        void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<TSharedState> e);
        void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<TSharedState> e);
    }


    public class UnknownMultiStateElement<TSharedState> : IMultiStateElement<TSharedState> {
        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<TSharedState> e) {
        }
        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<TSharedState> e) {
        }
    }


    public abstract class MultiStateContainerBase<TSharedState> :
        IDisposable {

        private IMultiStateElement<TSharedState> _current;
        public IMultiStateElement<TSharedState> Current => _current;


        private readonly Dictionary<Type, IMultiStateElement<TSharedState>> _states;
        private readonly TSharedState _sharedState;

        protected MultiStateContainerBase(
            Dictionary<Type, IMultiStateElement<TSharedState>> states,
            TSharedState sharedState
            ) {
            _states = states;
            _sharedState = sharedState;
            _current = new UnknownMultiStateElement<TSharedState>();
        }


        public void Dispose() {
            foreach (var state in _states.Values) {
                if (state is IDisposable stateDisposable) {
                    stateDisposable.Dispose();
                }
            }

            if (_sharedState is IDisposable sharedStateDisposable) {
                sharedStateDisposable.Dispose();
            }
        }


        public void SwitchTo<T>() where T : IMultiStateElement<TSharedState> {
            if (!_states.TryGetValue(typeof(T), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            var previous = _current;
            _current = next;

            var disabledEventArgs = new Helpers._EventArgs.MultiStateElementDisabledEventArgs<TSharedState>(
                _sharedState,
                next,
                null);

            previous.OnStateDisabled(disabledEventArgs);

            var enabledEventArgs = new Helpers._EventArgs.MultiStateElementEnabledEventArgs<TSharedState>(
                _sharedState,
                previous);

            _current.OnStateEnabled(enabledEventArgs);
        }


        public void SwitchToNew<T>(T newNextObj)
            where T : IMultiStateElement<TSharedState> {

            if (!_states.TryGetValue(typeof(T), out var nextObj)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            if (newNextObj == null || !object.ReferenceEquals(newNextObj, nextObj)) {
                throw new InvalidOperationException($"newNextObj is not in valid state");
            }

            var oldNextObj = nextObj;

            if (nextObj is IDisposable oldNextDisposable) {
                oldNextDisposable.Dispose();
            }
            nextObj = newNextObj; // replace old ref with new in dictionary
    
            var previous = _current;
            _current = newNextObj;

            var disabledEventArgs = new Helpers._EventArgs.MultiStateElementDisabledEventArgs<TSharedState>(
                _sharedState,
                newNextObj,
                oldNextObj);

            var enabledEventArgs = new Helpers._EventArgs.MultiStateElementEnabledEventArgs<TSharedState>(
                _sharedState,
                previous);

            previous.OnStateDisabled(disabledEventArgs);
            _current.OnStateEnabled(enabledEventArgs);
        }


        public T? Get<T>() where T : class, IMultiStateElement<TSharedState> {
            if (_states.TryGetValue(typeof(T), out var state)) {
                return (T)state;
            }

            System.Diagnostics.Debugger.Break();
            return null;
        }


        public void ForEachOther(System.Action<IMultiStateElement<TSharedState>> action) {
            foreach (var state in _states.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        protected static Dictionary<Type, IMultiStateElement<TSharedState>> BuildStatesMap(params IMultiStateElement<TSharedState>[] elements) {
            var map = new Dictionary<Type, IMultiStateElement<TSharedState>>();
            foreach (var el in elements) {
                map[el.GetType()] = el;
            }
            return map;
        }
    }



    public class MultiStateContainer<A, B, TSharedState> : MultiStateContainerBase<TSharedState>
        where A : IMultiStateElement<TSharedState>
        where B : IMultiStateElement<TSharedState> {
        public MultiStateContainer(A a, B b, TSharedState sharedState)
            : base(MultiStateContainerBase<TSharedState>.BuildStatesMap(a, b), sharedState) {
        }
    }


    public class MultiStateContainer<A, B, C, TSharedState> : MultiStateContainerBase<TSharedState>
        where A : IMultiStateElement<TSharedState>
        where B : IMultiStateElement<TSharedState>
        where C : IMultiStateElement<TSharedState> {
        public MultiStateContainer(A a, B b, C c, TSharedState sharedState)
            : base(MultiStateContainerBase<TSharedState>.BuildStatesMap(a, b, c), sharedState) {
        }
    }
}