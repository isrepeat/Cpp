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

namespace Helpers {
    public sealed class DisposableAction : IDisposable {
        private readonly Action _disposeAction;
        private bool _disposed;

        public DisposableAction(Action disposeAction) {
            _disposeAction = disposeAction ?? throw new ArgumentNullException(nameof(disposeAction));
        }

        public void Dispose() {
            if (!_disposed) {
                _disposeAction();
                _disposed = true;
            }
        }
    }
}