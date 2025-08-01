using System;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;


namespace CodeAnalyzer {
    namespace Ex {
        public static class TextExtensions {
            public static string ex_GetIdentationForFirstLine(this string text) {
                if (string.IsNullOrEmpty(text)) {
                    return string.Empty;
                }

                var match = Regex.Match(text, @"^(?<indent>[ \t]*)\S", RegexOptions.Multiline);
                return match.Success ? match.Groups["indent"].Value : string.Empty;
            }
        }


        public static class ClassExtensions {
            public static bool ex_HasFieldAttribute<TAttr>(this Data.Class cls)
                where TAttr : Attributes.PropertyAttributeBase {
                return cls.Fields.Any(field => field.ex_HasAttribute<TAttr>());
            }
        }


        public static class FieldExtensions {
            public static bool ex_HasAttribute<TAttr>(this Data.Field field)
                where TAttr : Attributes.PropertyAttributeBase {
                return field.PropertyAttributes.Any(attr => attr is TAttr);
            }

            public static bool ex_TryGetAttribute<TAttr>(this Data.Field field, out TAttr? result)
                where TAttr : Attributes.PropertyAttributeBase {

                foreach (var attr in field.PropertyAttributes) {
                    if (attr is TAttr matched) {
                        result = matched;
                        return true;
                    }
                }

                result = null;
                return false;
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

            public static bool ex_TryGetConstructorArgumentValue<T>(
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
}