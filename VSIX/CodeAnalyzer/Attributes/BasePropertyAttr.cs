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
    public sealed class BasePropertyAttr : PropertyAttributeBase, IPropertyTemplateEmitter {
        public BasePropertyAttr(Data.Field field) {
            this.GetterAccess = GetterAccess.Get;

            // Если есть несколько атрибутов, где один требует Set, а другой PrivateSet — выбираем наиболее ограниченный
            if (field.PropertyAttributes.Any(a => a.SetterAccess == Attributes.SetterAccess.PrivateSet)) {
                this.SetterAccess = Attributes.SetterAccess.PrivateSet;
            }
            else if (field.PropertyAttributes.Any(a => a.SetterAccess == Attributes.SetterAccess.Set)) {
                this.SetterAccess = Attributes.SetterAccess.Set;
            }
        }

        public void EmitToPropertyTemplate(Data.Field field, PropertyTemplateContext ctx) {
            var formatter = new Formatters.TypeFormatter()
                .AddRule(new Formatters.SimplifyNamespaceRule(field.ContainingNamespace))
                .AddRule(new Formatters.MultilineGenericRule());

            ctx.InsertCode(PropertyTemplate.Property.TYPE_NAME, $"{formatter.Format(field.TypeName)}", this.GetType());
            ctx.InsertCode(PropertyTemplate.Property.PROP_NAME, $"{field.PropName}", this.GetType());
            

            if (this.GetterAccess != GetterAccess.None) {
                ctx.InsertTemplate(PropertyTemplate.Property.GETTER, PropertyTemplate.Get.Template, this.GetType());

                var getterAccessStr = this.GetterAccess switch {
                    GetterAccess.Get => "",
                    _ => string.Empty
                };
                ctx.InsertCode(PropertyTemplate.Get.ACCESS, $"{getterAccessStr}", this.GetType());
                ctx.InsertCode(PropertyTemplate.Get.RETURN, $"{field.Name}", this.GetType());
            }


            if (this.SetterAccess != SetterAccess.None) {
                ctx.InsertTemplate(PropertyTemplate.Property.SETTER, PropertyTemplate.Set.Template, GetType());

                var setterAccessStr = this.SetterAccess switch {
                    SetterAccess.Set => "",
                    SetterAccess.PrivateSet => "private ",
                    _ => string.Empty
                };
                ctx.InsertCode(PropertyTemplate.Set.ACCESS, $"{setterAccessStr}", GetType());
            }

            ctx.InsertCode(PropertyTemplate.Property.FIELD_NAME, $"{field.Name}", GetType());
        }
    }
}