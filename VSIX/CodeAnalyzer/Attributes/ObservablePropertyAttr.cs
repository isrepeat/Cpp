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
    public sealed class ObservablePropertyAttr : PropertyAttributeBase {
        public ObservablePropertyAttr(AttributeData attributeData) {
            int argIndex = 0;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }

            argIndex++;
            if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Set>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.Set;
            }
            else if (attributeData.ex_TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.PrivateSet;
            }
        }
    }


    public class ObservablePropertyEmmiter : IPropertyTemplateEmitter {
        public void Emit(Data.Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.SET.AFTER_ASSIGNMENT,
                $"int aaa = 1;",
                GetType()
            );
        }
    }
}