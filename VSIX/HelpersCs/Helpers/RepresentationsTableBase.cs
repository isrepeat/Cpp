using System;
using System.Linq;
using System.Text;
using System.Windows;
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
    public abstract class RepresentationsTableBase<TRecord> {
        private readonly List<TRecord> _records = new();

        protected IReadOnlyList<TRecord> Records => _records;

        public void Add(TRecord record) {
            _records.Add(record);
        }

        public void AddRange(IEnumerable<TRecord> records) {
            _records.AddRange(records);
        }

        public abstract void BuildRepresentations();
    }
}