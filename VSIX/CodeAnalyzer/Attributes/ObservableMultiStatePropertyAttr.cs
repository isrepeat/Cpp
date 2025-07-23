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
using CodeAnalyzer.Ex;


namespace CodeAnalyzer.Attributes {
    public sealed class ObservableMultiStatePropertyAttr : PropertyAttributeBase, IPropertyTemplateEmitter {
        public ObservableMultiStatePropertyAttr(AttributeData attributeData) {
            int argIndex = 0;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }

            argIndex++;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Set>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.Set;
            }
        }


        public void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.Set.BEFORE_ASSIGNMENT,
                $"if ({field.Name} != null) {{\n" +
                $"    {field.Name}.StateChanged -= this.On{field.PropName}Changed;\n" +
                $"    {field.Name}.Dispose();\n" +
                $"}}",
                GetType());

            ctx.InsertCode(
                PropertyTemplate.Set.AFTER_ASSIGNMENT,
                $"{field.Name}.StateChanged += this.On{field.PropName}Changed;",
                GetType());

            ctx.InsertCode(
                PropertyTemplate.Property.EXTRAS,
                $"\n\nprivate void On{field.PropName}Changed() {{\n" +
                $"    this.OnSharedStatePropertyChanged(nameof(this.{field.PropName}));\n" +
                $"}}",
                GetType());
        }
    }
}