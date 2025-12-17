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
    public sealed class InvalidatableLazyPropertyAttr : PropertyAttributeBase, IPropertyTemplateEmitter {
        public string FactoryMethodName { get; }

        public InvalidatableLazyPropertyAttr(AttributeData attributeData) {
            int argIndex = 0;
            if (attributeData.ex_TryGetConstructorArgumentValue<string>(argIndex, out var arg0_factoryMethodName)) {
                this.FactoryMethodName = arg0_factoryMethodName;
            }
            else if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }

            argIndex++;
            if (attributeData.ex_TryGetConstructorArgumentValue<string>(argIndex, out var arg1_factoryMethodName)) {
                this.FactoryMethodName = arg1_factoryMethodName;
            }
            else if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
        }


        public void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.Get.BEGIN,
                $"if (!_invalidatablePropertiesState.Is{field.PropName}Valid) {{\n" +
                $"    System.Diagnostics.Debugger.Break();\n" +
                $"    return default;\n" +
                $"}}\n"+
                $"if ({field.Name} == null) {{\n" +
                $"    {field.Name} = this.{this.FactoryMethodName}();\n" +
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