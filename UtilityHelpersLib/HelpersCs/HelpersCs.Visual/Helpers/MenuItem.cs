using System;
using System.Linq;
using System.Text;
using System.Windows.Input;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    public interface IMenuItem { }

    public class MenuItemBase : IMenuItem, INotifyPropertyChanged {
        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null) {
            if (propertyName == null) {
                throw new ArgumentNullException(nameof(propertyName), "CallerMemberName did not supply a property name.");
            }
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class MenuItemHeader : MenuItemBase {
        private string _header = "";
        public string Header {
            get => _header;
            set {
                if (_header != value) {
                    _header = value;
                    OnPropertyChanged();
                }
            }
        }
    }


    public class MenuItemCommand : MenuItemBase {
        private string _header = "";
        public string Header {
            get => _header;
            set {
                if (_header != value) {
                    _header = value;
                    OnPropertyChanged();
                }
            }
        }

        private ICommand _command;
        public ICommand Command {
            get => _command;
            set {
                if (_command != value) {
                    _command = value;
                    OnPropertyChanged();
                }
            }
        }

        private object _commandParameterContext;
        public object CommandParameterContext {
            get => _commandParameterContext;
            set {
                if (_commandParameterContext != value) {
                    _commandParameterContext = value;
                    OnPropertyChanged();
                }
            }
        }
    }


    public class MenuItemSeparator : IMenuItem { }
}