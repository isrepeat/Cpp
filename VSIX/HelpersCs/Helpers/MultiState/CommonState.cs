using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;


namespace Helpers.MultiState {
    public interface ICommonState {
    }

    public interface IDynamicCommonState<TUpdateParams> : ICommonState {
        void UpdateData(TUpdateParams initParams);
    }

    namespace _Details {
        public abstract class CommonStateBase : ICommonState {
            public event PropertyChangedEventHandler? CommonStatePropertyChanged;

            protected void OnCommonStatePropertyChanged([CallerMemberName] string? propertyName = null) {
                if (propertyName == null) {
                    throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
                }
                this.CommonStatePropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }


    public abstract class StaticCommonStateBase : _Details.CommonStateBase {
    }

    public abstract class DynamicCommonStateBase<TUpdateParams> : _Details.CommonStateBase, IDynamicCommonState<TUpdateParams> {
        public abstract void UpdateData(TUpdateParams initParams);
    }
}