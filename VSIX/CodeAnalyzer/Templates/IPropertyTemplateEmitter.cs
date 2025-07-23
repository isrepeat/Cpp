using System;
using System.Text;
using System.Text.RegularExpressions;
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
    public interface IPropertyTemplateEmitter {
        void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx);
    }
}