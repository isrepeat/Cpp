using System;
using System.Text;
using System.Linq;
using System.Threading;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Helpers.Attributes;
using System.Text.RegularExpressions;
using System.Reflection;
using CodeAnalyzer.Templates;


namespace CodeAnalyzer.Attributes {
    public class BasePropertyEmitter : IPropertyTemplateEmitter {
        public void Emit(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(PropertyTemplate.TYPE_NAME, $"{field.TypeName}", GetType());
            ctx.InsertCode(PropertyTemplate.PROP_NAME, $"{field.PropName}", GetType());
            ctx.InsertCode(PropertyTemplate.FIELD_NAME, $"{field.Name}", GetType());
            ctx.InsertCode(PropertyTemplate.GET.MODIFICATOR, $"", GetType());
            ctx.InsertCode(PropertyTemplate.SET.MODIFICATOR, $"", GetType());
        }
    }
}