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
        private string _name;
    }
}