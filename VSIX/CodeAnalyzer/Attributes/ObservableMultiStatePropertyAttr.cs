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
        public string? NotifyMethodName { get; private set; }

        public ObservableMultiStatePropertyAttr(AttributeData attributeData) {
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
            using (var builder = ctx.GetCodeBuilder(PropertyTemplate.Set.BEFORE_ASSIGNMENT, this.GetType())) {
                builder.Add($"if ({field.Name} != null) {{");
                builder.Add($"    {field.Name}.StateChanged -= this.On{field.PropName}Changed;");
                builder.Add($"    {field.Name}.Dispose();");
                builder.Add($"}}");
            }

            string notifyCode = string.IsNullOrEmpty(this.NotifyMethodName)
                ? $""
                : $"{this.NotifyMethodName}(nameof(this.{field.PropName}));";

            using (var builder = ctx.GetCodeBuilder(PropertyTemplate.Set.AFTER_ASSIGNMENT, this.GetType())) {
                builder.Add($"{field.Name}.StateChanged += this.On{field.PropName}Changed;");
                
                if (!string.IsNullOrEmpty(notifyCode)) {
                    builder.Add($"{notifyCode}");
                }
            }

            using (var builder = ctx.GetCodeBuilder(PropertyTemplate.Property.EXTRAS, this.GetType())) {
                builder.Add($"\n\n");
                builder.Add($"private void On{field.PropName}Changed() {{");
                
                if (!string.IsNullOrEmpty(notifyCode)) {
                    builder.Add($"    {notifyCode}");
                }

                builder.Add($"}}");
            }
        }
    }
}