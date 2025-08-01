using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Linq;
using System.Threading;
using System.Reflection;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Helpers.Attributes;


namespace CodeAnalyzer.Templates {
    public static class PropertyTemplate {
        public static class Property {
            public static readonly TemplateSlot TYPE_NAME = new("TYPE_NAME", SlotKind.Inline);
            public static readonly TemplateSlot PROP_NAME = new("PROP_NAME", SlotKind.Inline);
            public static readonly TemplateSlot FIELD_NAME = new("FIELD_NAME", SlotKind.Inline);
            public static readonly TemplateSlot GETTER = new("GETTER");
            public static readonly TemplateSlot SETTER = new("SETTER");
            public static readonly TemplateSlot EXTRAS = new("EXTRAS");

            public static readonly Template Template = new Template(
                text: $"public {Property.TYPE_NAME} {Property.PROP_NAME} {{\n" +
                      $"    {Property.GETTER}\n" +
                      $"    {Property.SETTER}\n" +
                      $"}}{Property.EXTRAS}",
                usedTemplateSlotOwners: new[] {
                    typeof(Property),
                }
            );
        }

        public static class Get {
            public static readonly TemplateSlot ACCESS = new("GET:ACCESS", SlotKind.Inline);
            public static readonly TemplateSlot BEGIN = new("GET:BEGIN");
            public static readonly TemplateSlot RETURN = new("GET:RETURN");

            public static readonly Template Template = new Template(
                text: $"{Get.ACCESS}get {{\n" +
                      $"    {Get.BEGIN}\n" +
                      $"    return {Get.RETURN};\n" +
                      $"}}",
                usedTemplateSlotOwners: new[] {
                    typeof(Get),
                }
            );
        }

        public static class Set {
            public static readonly TemplateSlot ACCESS = new("SET:ACCESS", SlotKind.Inline);
            public static readonly TemplateSlot BEGIN = new("SET:BEGIN");
            public static readonly TemplateSlot BEFORE_ASSIGNMENT = new("SET:BEFORE_ASSIGNMENT");
            //public static readonly TemplateSlot ASSIGNMENT = new("SET:ASSIGNMENT");
            public static readonly TemplateSlot AFTER_ASSIGNMENT = new("SET:AFTER_ASSIGNMENT");
            public static readonly TemplateSlot END = new("SET:END");

            public static readonly Template Template = new Template(
                text: $"{Set.ACCESS}set {{\n" +
                      $"    {Set.BEGIN}\n" +
                      $"    if (!object.ReferenceEquals({Property.FIELD_NAME}, value)) {{\n" +
                      $"        {Set.BEFORE_ASSIGNMENT}\n" +
                      $"        {Property.FIELD_NAME} = value;\n" +
                      $"        {Set.AFTER_ASSIGNMENT}\n" +
                      $"    }}\n" +
                      $"    {Set.END}\n" +
                      $"}}",
                usedTemplateSlotOwners: new[] {
                    typeof(Set),
                    typeof(Property),
                }
            );
        }
    }
}