using System;
using System.Collections.Generic;
using System.ComponentModel;


namespace Helpers.MultiState {
    /// <summary>
    /// Базовый ViewModel для объектов, обёрнутых вокруг общего состояния (CommonState).
    /// </summary>
    public abstract class CommonStateViewModelBase<TCommonState> :
        Helpers.ObservableObject,
        IDisposable
        where TCommonState : ICommonState {
        protected TCommonState CommonState { get; }

        protected CommonStateViewModelBase(TCommonState commonState) {
            this.CommonState = commonState;

            if (this.CommonState is _Details.CommonStateBase commonStateBase) {
                commonStateBase.CommonStatePropertyChanged += this.OnCommonStatePropertyChanged;
            }
        }

        /// <summary>
        /// ⚠️ ВАЖНО: Намеренно НЕ отписываемся от commonStateBase.CommonStatePropertyChanged./>.
        /// 
        /// Причины:
        /// 1. ViewModel продолжает использовать `CommonState` даже после вызова Dispose().
        ///    Например, `CommonState.Dispose()` может вызывать `OnPropertyChanged`, и `ViewModel`
        ///    должен его обработать.
        /// 
        /// 2. Это означает, что `CommonState` сохраняет жёсткую ссылку на делегат `ViewModel`,
        ///    а значит, `ViewModel` не будет собран GC, пока жив `CommonState`.
        /// 
        /// Это поведение считается безопасным и допустимым, потому что:
        /// – 'CommonState' гарантированно живёт дольше, чем ViewModel;
        /// </summary>
        public virtual void Dispose() {
        }


        public override bool Equals(object? obj) {
            if (obj is CommonStateViewModelBase<TCommonState> other) {
                return EqualityComparer<TCommonState>.Default.Equals(this.CommonState, other.CommonState);
            }
            return false;
        }


        public override int GetHashCode() {
            return EqualityComparer<TCommonState>.Default.GetHashCode(this.CommonState);
        }


        protected virtual void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) { }
    }
}