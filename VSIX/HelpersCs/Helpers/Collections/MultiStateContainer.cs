using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers.Collections {
    public interface IMultiStateElement {
        void OnStateEnabled(IMultiStateElement previousState);
        void OnStateDisabled(IMultiStateElement nextState);
    }


    public class UnknownMultiStateElement : IMultiStateElement {
        public void OnStateEnabled(IMultiStateElement previousState) {
        }
        public void OnStateDisabled(IMultiStateElement nextState) {
        }
    }

    public abstract class MultiStateContainerBase {
        private readonly Dictionary<Type, IMultiStateElement> _states;
        private IMultiStateElement _current;

        protected MultiStateContainerBase(Dictionary<Type, IMultiStateElement> states) {
            _states = states;
            _current = new UnknownMultiStateElement();
        }

        public IMultiStateElement Current => _current;

        public void SwitchTo<T>() where T : IMultiStateElement {
            if (!_states.TryGetValue(typeof(T), out var next)) {
                throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
            }

            var previous = _current;
            _current = next;

            previous.OnStateDisabled(_current);
            _current.OnStateEnabled(previous);
        }

        public T? Get<T>() where T : class, IMultiStateElement {
            return _states.TryGetValue(typeof(T), out var state) 
                ? (T)state
                : null;
        }

        public void ForEachOther(Action<IMultiStateElement> action) {
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



    public class MultiStateContainer<A, B> : MultiStateContainerBase
        where A : IMultiStateElement
        where B : IMultiStateElement {
        public MultiStateContainer(A a, B b)
            : base(MultiStateContainerBase.BuildStatesMap(a, b)) {
        }
    }

    public class MultiStateContainer<A, B, C> : MultiStateContainerBase
        where A : IMultiStateElement
        where B : IMultiStateElement
        where C : IMultiStateElement {
        public MultiStateContainer(A a, B b, C c)
            : base(MultiStateContainerBase.BuildStatesMap(a, b, c)) {
        }
    }



    //public class MultiStateContainer {
    //    private readonly List<IMultiStateElement> _states;
        
    //    private IMultiStateElement _current;
    //    public IMultiStateElement Current => _current;

        
    //    public MultiStateContainer(IEnumerable<IMultiStateElement> states) {
    //        _states = states.ToList();
    //        _current = _states.First(); // или можно передавать явно активный
    //    }


    //    public void SwitchTo<T>() where T : class, IMultiStateElement {
    //        var next = _states.OfType<T>().FirstOrDefault();
    //        if (next == null) {
    //            throw new InvalidOperationException($"No state of type {typeof(T).Name} found.");
    //        }

    //        var previous = _current;
    //        _current = next;

    //        previous.OnStateDisabled(_current);
    //        _current.OnStateEnabled(previous);
    //    }

        
    //    public T? Get<T>() where T : class, IMultiStateElement {
    //        return _states.OfType<T>().FirstOrDefault();
    //    }
        
        
    //    public void ForEachOther(Action<IMultiStateElement> action) {
    //        foreach (var state in _states) {
    //            if (!ReferenceEquals(state, _current)) {
    //                action(state);
    //            }
    //        }
    //    }
    //}
}