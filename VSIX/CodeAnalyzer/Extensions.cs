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


namespace CodeAnalyzer {
    namespace Ex {
        public static class ClassExtensions {
            public static bool ex_HasFieldAttribute<TAttr>(this Class cls)
                where TAttr : PropertyAttributeBase {
                return cls.Fields.Any(field => field.ex_HasAttribute<TAttr>());
            }
        }

        public static class FieldExtensions {
            public static bool ex_HasAttribute<TAttr>(this Field field)
                where TAttr : PropertyAttributeBase {
                return field.PropertyAttributes.Any(attr => attr is TAttr);
            }
        }

        public static class AttributeDataExtensions {
            public static bool ex_IsAttribute(this AttributeData attrData, Type attributeType) {
                string fullName = attributeType.Name;
                string suffix = "Attribute";

                string shortName = fullName.EndsWith(suffix, StringComparison.Ordinal)
                    ? fullName.Substring(0, fullName.Length - suffix.Length)
                    : fullName;

                string? actualName = attrData.AttributeClass?.Name;
                return actualName == fullName || actualName == shortName;
            }
        }
    }


    public static class AttributeDataExtensions {
        public static bool TryGetConstructorArgumentValue<T>(
            this AttributeData attributeData,
            int index,
            out T? value
            ) {

            value = default;

            if (attributeData == null) {
                return false;
            }

            if (index < 0 || index >= attributeData.ConstructorArguments.Length) {
                return false;
            }

            var arg = attributeData.ConstructorArguments[index];

            if (arg.Value is T tValue) {
                value = tValue;
                return true;
            }

            if (typeof(T).IsEnum &&
                arg.Type?.ToDisplayString() == typeof(T).FullName &&
                arg.Value is int intVal) {
                value = (T)(object)intVal;
                return true;
            }

            return false;
        }
    }
}