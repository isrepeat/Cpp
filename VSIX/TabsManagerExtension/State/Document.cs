using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TabsManagerExtension {
    public class DocumentInfo : Helpers.ObservableObject {
        private string _displayName;
        private string _fullName;

        public string DisplayName {
            get => _displayName;
            set {
                if (_displayName != value) {
                    _displayName = value;
                    OnPropertyChanged();
                }
            }
        }

        public string FullName {
            get => _fullName;
            set {
                if (_fullName != value) {
                    _fullName = value;
                    OnPropertyChanged();
                }
            }
        }

        public string ProjectName { get; set; }
    }


    public class ProjectGroup : Helpers.ObservableObject {
        public string Name { get; set; }
        public ObservableCollection<DocumentInfo> Items { get; set; } = new ObservableCollection<DocumentInfo>();
    }
}