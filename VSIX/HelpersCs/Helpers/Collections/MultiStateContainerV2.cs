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


namespace HelpersV2._EventArgs {
    public sealed class MultiStateElementEnabledEventArgs : EventArgs {
        //public HelpersV2.Collections.ISharedState SharedState { get; }
        public HelpersV2.Collections.IMultiStateElement PreviousState { get; }
        public MultiStateElementEnabledEventArgs(
            //HelpersV2.Collections.ISharedState sharedState,
            HelpersV2.Collections.IMultiStateElement previousState
            ) {
            //this.SharedState = sharedState;
            this.PreviousState = previousState;
        }
    }


    public sealed class MultiStateElementDisabledEventArgs : EventArgs {
        //public HelpersV2.Collections.ISharedState SharedState { get; }
        public HelpersV2.Collections.IMultiStateElement NextState { get; }
        public HelpersV2.Collections.IMultiStateElement? OldNextState { get; }
        public MultiStateElementDisabledEventArgs(
            //HelpersV2.Collections.ISharedState sharedState,
            HelpersV2.Collections.IMultiStateElement nextState,
            HelpersV2.Collections.IMultiStateElement? oldNextState
            ) {
            //this.SharedState = sharedState;
            this.NextState = nextState;
            this.OldNextState = oldNextState;
        }
    }
}



namespace HelpersV2.Collections {
    public interface ISharedState {
    }

    public abstract class SharedStateBase : ISharedState {
        public event PropertyChangedEventHandler? SharedStatePropertyChanged;

        protected void OnSharedStatePropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.SharedStatePropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }


    public interface IMultiStateElement {
        //public ISharedState SharedState { get; }
        void OnStateEnabled(HelpersV2._EventArgs.MultiStateElementEnabledEventArgs e);
        void OnStateDisabled(HelpersV2._EventArgs.MultiStateElementDisabledEventArgs e);
    }


    //public class UnknownMultiStateElement<TSharedState> : IMultiStateElement
    //    where TSharedState : ISharedState {

    //    public ISharedState SharedState { get; }
    //    public UnknownMultiStateElement(TSharedState sharedState) {
    //        this.SharedState = sharedState ?? throw new ArgumentNullException(nameof(sharedState));
    //    }

    //    public void OnStateEnabled(HelpersV2._EventArgs.MultiStateElementEnabledEventArgs e) {
    //    }
    //    public void OnStateDisabled(HelpersV2._EventArgs.MultiStateElementDisabledEventArgs e) {
    //    }
    //}

    public class UnknownMultiStateElement : IMultiStateElement {
        public void OnStateEnabled(HelpersV2._EventArgs.MultiStateElementEnabledEventArgs e) {
        }
        public void OnStateDisabled(HelpersV2._EventArgs.MultiStateElementDisabledEventArgs e) {
        }
    }


    public abstract class MultiStateContainerBase<TSharedState> :
        IDisposable
        where TSharedState : ISharedState {
        
        public event System.Action? StateChanged;


        private IMultiStateElement _current;
        public IMultiStateElement Current => _current;


        private readonly Dictionary<Type, IMultiStateElement> _states;
        private readonly TSharedState _sharedState;

        protected MultiStateContainerBase(
            Dictionary<Type, IMultiStateElement> states,
            TSharedState sharedState
            ) {
            _states = states;
            _sharedState = sharedState;
            _current = new UnknownMultiStateElement();
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


        public void SwitchTo<T>() where T : IMultiStateElement {
            if (!_states.TryGetValue(typeof(T), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            var previous = _current;
            _current = next;

            this.StateChanged?.Invoke();

            var disabledEventArgs = new HelpersV2._EventArgs.MultiStateElementDisabledEventArgs(
                //_sharedState,
                next,
                null);

            previous.OnStateDisabled(disabledEventArgs);

            var enabledEventArgs = new HelpersV2._EventArgs.MultiStateElementEnabledEventArgs(
                //_sharedState,
                previous);

            _current.OnStateEnabled(enabledEventArgs);
        }


        //public void SwitchToNew<T>(T newNextObj)
        //    where T : IMultiStateElement {

        //    if (!_states.TryGetValue(typeof(T), out var nextObj)) {
        //        throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
        //    }

        //    if (newNextObj == null || !object.ReferenceEquals(newNextObj, nextObj)) {
        //        throw new InvalidOperationException($"newNextObj is not in valid state");
        //    }

        //    var oldNextObj = nextObj;

        //    if (nextObj is IDisposable oldNextDisposable) {
        //        oldNextDisposable.Dispose();
        //    }
        //    nextObj = newNextObj; // replace old ref with new in dictionary
    
        //    var previous = _current;
        //    _current = newNextObj;

        //    var disabledEventArgs = new HelpersV2._EventArgs.MultiStateElementDisabledEventArgs(
        //        _sharedState,
        //        newNextObj,
        //        oldNextObj);

        //    var enabledEventArgs = new HelpersV2._EventArgs.MultiStateElementEnabledEventArgs(
        //        _sharedState,
        //        previous);

        //    previous.OnStateDisabled(disabledEventArgs);
        //    _current.OnStateEnabled(enabledEventArgs);
        //}


        public T? Get<T>() where T : class, IMultiStateElement {
            if (_states.TryGetValue(typeof(T), out var state)) {
                return (T)state;
            }

            System.Diagnostics.Debugger.Break();
            return null;
        }


        public void ForEachOther(System.Action<IMultiStateElement> action) {
            foreach (var state in _states.Values) {
                if (!object.ReferenceEquals(state, _current)) {
                    action(state);
                }
            }
        }


        protected static Dictionary<Type, IMultiStateElement> BuildStatesMap(params IMultiStateElement[] elements) {
            var map = new Dictionary<Type, IMultiStateElement>();
            foreach (var el in elements) {
                map[el.GetType()] = el;
            }
            return map;
        }
    }



    public class MultiStateContainer<A, B, TSharedState> : MultiStateContainerBase<TSharedState>
        where A : IMultiStateElement
        where B : IMultiStateElement
        where TSharedState : ISharedState {
        public MultiStateContainer(A a, B b, TSharedState sharedState)
            : base(MultiStateContainerBase<TSharedState>.BuildStatesMap(a, b), sharedState) {
        }
    }


    public class MultiStateContainer<A, B, C, TSharedState> : MultiStateContainerBase<TSharedState>
        where A : IMultiStateElement
        where B : IMultiStateElement
        where C : IMultiStateElement
        where TSharedState : ISharedState {
        public MultiStateContainer(A a, B b, C c, TSharedState sharedState)
            : base(MultiStateContainerBase<TSharedState>.BuildStatesMap(a, b, c), sharedState) {
        }
    }
}