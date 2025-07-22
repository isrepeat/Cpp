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
    public sealed class InvalidatableLazyPropertyAttr : PropertyAttributeBase {
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
    }


    public class InvalidatableLazyPropertyEmmiter : IPropertyTemplateEmitter {
        public void Emit(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.SET.AFTER_ASSIGNMENT,
                $"int ddd = 1;",
                GetType()
            );
        }
    }
}