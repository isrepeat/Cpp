using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace Helpers.MultiState {
    public interface IMultiStateEntry<TCommonState> {
        T As<T>() where T : IMultiStateEntry<TCommonState>;
    }

    public abstract class MultiStateEntryBase<TCommonState> : 
        Helpers.ObservableObject,
        IMultiStateEntry<TCommonState>,
        IDisposable
        where TCommonState : class, ICommonState {

        protected MultiStateContainerBase<TCommonState> MultiStateBase { get; }

        protected MultiStateEntryBase(MultiStateContainerBase<TCommonState> multiStateBase) {
            this.MultiStateBase = multiStateBase;
            this.MultiStateBase.StateChanged += this.OnMultiStateChanged;
        }

        public virtual void Dispose() {
            this.MultiStateBase.StateChanged -= this.OnMultiStateChanged;
            this.MultiStateBase.Dispose();
        }

        public T As<T>() where T : IMultiStateEntry<TCommonState> {
            if (this is T typed) {
                return typed;
            }

            throw new InvalidOperationException(
                $"Could not cast 'this' <{this.GetType().Name}> to <'{typeof(T).Name}'>");
        }

        public override bool Equals(object? obj) {
            if (obj is MultiStateEntryBase<TCommonState> other) {
                return this.MultiStateBase.Equals(other.MultiStateBase);
            }
            return false;
        }
        public override int GetHashCode() {
            return this.MultiStateBase.GetHashCode();
        }
        public override string ToString() {
            return this.MultiStateBase.ToString();
        }

        protected abstract void OnMultiStateChanged();
    }
}