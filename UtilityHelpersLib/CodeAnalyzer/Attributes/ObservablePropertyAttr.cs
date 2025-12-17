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
    public sealed class ObservablePropertyAttr : PropertyAttributeBase, IPropertyTemplateEmitter {
        public string? NotifyMethodName { get; private set; }

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

            if (attributeData.NamedArguments.FirstOrDefault(kvp => kvp.Key == "NotifyMethod").Value.Value is string notifyName) {
                this.NotifyMethodName = notifyName;
            }
        }


        public void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx) {
            string notifyCode = string.IsNullOrEmpty(this.NotifyMethodName)
                ? $"base.OnPropertyChanged(nameof(this.{field.PropName}));"
                : $"{this.NotifyMethodName}(nameof(this.{field.PropName}));";

            ctx.InsertCode(
                PropertyTemplate.Set.AFTER_ASSIGNMENT,
                $"{notifyCode}",
                GetType()
            );
        }
    }
}