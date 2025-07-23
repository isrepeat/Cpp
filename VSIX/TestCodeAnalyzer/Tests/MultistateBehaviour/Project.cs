using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using Helpers.Attributes;


namespace TestCodeAnalyzer.Tests.MultistateBehaviour {
    public partial class ProjectNode :
        Helpers.ObservableObject {

        [ObservableProperty]
        //[InvalidatableProperty]
        //[InvalidatableLazyProperty(nameof(this.InitName))]
        private string _name;

        public void Invalidate() {
        }

        public string InitName() {
            return "";
        }
    }
}