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
    public interface ITypeFormatterRule {
        string Apply(string fullTypeName);
    }
}