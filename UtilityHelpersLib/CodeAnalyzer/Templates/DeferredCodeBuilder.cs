using System;
using System.Text.RegularExpressions;
using System.Text;
using System.Linq;
using System.Threading;
using System.Reflection;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Helpers.Attributes;


namespace CodeAnalyzer.Templates {
    public class DeferredCodeBuilder : IDisposable {
        private readonly List<string> _lines = new();
        private readonly PropertyTemplateContext _ctx;
        private readonly TemplateSlot _slot;
        private readonly Type _emitterType;

        private bool _disposed;
        private bool _commited;

        public DeferredCodeBuilder(
            PropertyTemplateContext ctx,
            TemplateSlot slot,
            Type emitterType
        ) {
            _ctx = ctx;
            _slot = slot;
            _emitterType = emitterType;
        }

        public void Dispose() {
            if (_disposed) {
                return;
            }

            this.Commit();

            _disposed = true;
        }

        public void Add(string line) {
            _lines.Add(line);
        }

        private void Commit() {
            if (_commited) {
                return;
            }

            if (_lines.Count > 0) {
                string code = string.Join("\n", _lines);
                _ctx.InsertCode(_slot, code, _emitterType);
            }

            _commited = true;
        }
    }
}