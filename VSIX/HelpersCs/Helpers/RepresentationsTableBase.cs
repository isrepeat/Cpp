using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
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
            if (record == null) {
                Helpers.Diagnostic.Logger.LogWarning($"'record == null'");
                return;
            }
            _records.Add(record);
        }

        public void AddRange(IEnumerable<TRecord> records) {
            if (records == null) {
                Helpers.Diagnostic.Logger.LogWarning($"'records == null'");
                return;
            }
            _records.AddRange(records);
        }

        public void Clear() {
            _records.Clear();
        }

        public abstract void BuildRepresentations();
    }
}