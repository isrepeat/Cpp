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
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName) {
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
                    OnPropertyChanged(nameof(Header));
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
                    OnPropertyChanged(nameof(Header));
                }
            }
        }

        private ICommand _command;
        public ICommand Command {
            get => _command;
            set {
                if (_command != value) {
                    _command = value;
                    OnPropertyChanged(nameof(Command));
                }
            }
        }
    }


    public class MenuItemSeparator : IMenuItem { }

}