using System;
using System.Text;
using System.Linq;
using System.Threading;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using CodeAnalyzer.Ex;


namespace CodeAnalyzer.Formatters {
    public sealed class TypeFormatter {
        private readonly List<ITypeFormatterRule> _rules = new();

        public TypeFormatter AddRule(ITypeFormatterRule rule) {
            _rules.Add(rule);
            return this;
        }

        public string Format(string fullTypeName) {
            string result = fullTypeName;

            foreach (var rule in _rules) {
                result = rule.Apply(result);
            }

            return result;
        }
    }
}