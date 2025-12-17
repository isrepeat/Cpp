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
    public sealed class InvalidatablePropertyAttr : PropertyAttributeBase, IPropertyTemplateEmitter {
        public InvalidatablePropertyAttr(AttributeData attributeData) {
            int argIndex = 0;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
            //else if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
            //    this.SetterAccess = SetterAccess.PrivateSet;
            //}

            argIndex++;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Set>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.Set;
            }
            else if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.PrivateSet;
            }
        }


        public void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.Get.BEGIN,
                $"if (!_invalidatablePropertiesState.Is{field.PropName}Valid) {{\n" +
                $"    System.Diagnostics.Debugger.Break();\n" +
                $"    return default;\n" +
                $"}}",
                GetType());

            ctx.InsertCode(
                PropertyTemplate.Set.BEGIN,
                $"if (!_invalidatablePropertiesState.Is{field.PropName}Valid) {{\n" +
                $"    System.Diagnostics.Debugger.Break();\n" +
                $"    return;\n" +
                $"}}",
                GetType());

            ctx.InsertCode(
                PropertyTemplate.Set.AFTER_ASSIGNMENT,
                $"_invalidatablePropertiesState.{field.PropName}Cached = value;",
                GetType());

            ctx.InsertCode(
                PropertyTemplate.Property.EXTRAS,
                $"\n\n" +
                $"public {field.TypeName} {field.PropName}Cached {{\n" +
                $"    get {{\n" +
                $"        if (_invalidatablePropertiesState.{field.PropName}Cached == null) {{\n" +
                $"            _invalidatablePropertiesState.{field.PropName}Cached = {field.Name};\n" +
                $"        }}\n" +
                $"        return ({field.TypeName})_invalidatablePropertiesState.{field.PropName}Cached;\n" +
                $"    }}\n" +
                $"}}",
                GetType());
        }
    }
}