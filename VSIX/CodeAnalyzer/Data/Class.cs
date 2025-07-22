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


namespace CodeAnalyzer.Data {
    public abstract class ClassMetadataBase {
        public INamedTypeSymbol Symbol { get; }

        public string Name { get; }
        public string Namespace { get; }
        public string FullName { get; }

        public ClassMetadataBase(INamedTypeSymbol symbol) {
            this.Symbol = symbol;
            this.Name = symbol.Name;
            this.Namespace = symbol.ContainingNamespace.ToDisplayString();
            this.FullName = $"{this.Namespace}.{this.Name}";
        }

        public override string ToString() {
            return this.FullName;
        }
    }


    public sealed class Class : ClassMetadataBase {
        public IReadOnlyList<Field> Fields { get; }

        public Class(INamedTypeSymbol symbol, IEnumerable<Field> fields)
            : base(symbol) {
            this.Fields = fields.ToList(); // делаем копию
        }
    }
}