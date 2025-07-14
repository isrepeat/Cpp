using System;
using System.Windows.Input;

namespace Helpers {
    public class RelayCommand : ICommand {
        private readonly System.Action _execute;
        private readonly Func<bool> _canExecute;

        public RelayCommand(System.Action execute, Func<bool> canExecute = null) {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter) {
            return _canExecute?.Invoke() ?? true;
        }

        public void Execute(object parameter) {
            _execute();
        }

        public event EventHandler CanExecuteChanged {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }


    public class RelayCommand<T> : ICommand {
        private readonly System.Action<T> _execute;
        private readonly Func<T, bool> _canExecute;

        public RelayCommand(System.Action<T> execute, Func<T, bool> canExecute = null) {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter) {
            return _canExecute?.Invoke((T)parameter) ?? true;
        }

        public void Execute(object parameter) {
            _execute((T)parameter);
        }

        public event EventHandler CanExecuteChanged {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }
}