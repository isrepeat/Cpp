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
        public static readonly TemplateSlot TYPE_NAME = new("TYPE_NAME", SlotKind.Inline);
        public static readonly TemplateSlot PROP_NAME = new("PROP_NAME", SlotKind.Inline);
        public static readonly TemplateSlot FIELD_NAME = new("FIELD_NAME", SlotKind.Inline);

        public static class GET {
            public static readonly TemplateSlot MODIFICATOR = new("GET:MODIFICATOR", SlotKind.Inline);
            public static readonly TemplateSlot BEGIN = new("GET:BEGIN");
        }

        public static class SET {
            public static readonly TemplateSlot MODIFICATOR = new("SET:MODIFICATOR", SlotKind.Inline);
            public static readonly TemplateSlot BEGIN = new("SET:BEGIN");
            public static readonly TemplateSlot BEFORE_ASSIGNMENT = new("SET:BEFORE_ASSIGNMENT");
            public static readonly TemplateSlot AFTER_ASSIGNMENT = new("SET:AFTER_ASSIGNMENT");
            public static readonly TemplateSlot END = new("SET:END");
        }

        public static readonly HashSet<TemplateSlot> Slots = PropertyTemplate.CollectSlots();
        public static readonly IReadOnlyDictionary<string, TemplateSlot> MapIdToSlot = Slots.ToDictionary(s => s.Id);

        public static readonly string Base =
            $"public {TYPE_NAME} {PROP_NAME} {{\n" +
            $"    {GET.MODIFICATOR}get {{\n" +
            $"        {GET.BEGIN}\n" +
            $"        return {FIELD_NAME};\n" +
            $"    }}\n" +
            $"    {SET.MODIFICATOR}set {{\n" +
            $"        {SET.BEGIN}\n" +
            $"        if (!object.ReferenceEquals({FIELD_NAME}, value)) {{\n" +
            $"            {SET.BEFORE_ASSIGNMENT}\n" +
            $"            {FIELD_NAME} = value;\n" +
            $"            {SET.AFTER_ASSIGNMENT}\n" +
            $"        }}\n" +
            $"        {SET.END}\n" +
            $"    }}\n" +
            $"}}";



        private static HashSet<TemplateSlot> CollectSlots() {
            var flags = BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy;
            var result = new HashSet<TemplateSlot>();

            // Добавляем слоты из PropertyTemplate и всех вложенных типов
            foreach (var type in PropertyTemplate.GetAllNestedTypesIncludingSelf(typeof(PropertyTemplate))) {
                var fields = type.GetFields(flags)
                    .Where(f => f.FieldType == typeof(TemplateSlot))
                    .Select(f => (TemplateSlot)f.GetValue(null)!);

                foreach (var slot in fields) {
                    result.Add(slot);
                }
            }

            return result;
        }


        private static List<Type> GetAllNestedTypesIncludingSelf(Type root) {
            var result = new List<Type>();
            PropertyTemplate.CollectRecursive(root, result);
            return result;
        }


        private static void CollectRecursive(Type type, List<Type> result) {
            result.Add(type);

            foreach (var nested in type.GetNestedTypes(BindingFlags.Public | BindingFlags.Static)) {
                PropertyTemplate.CollectRecursive(nested, result);
            }
        }
    }


    public class ParsedPropertyTemplate {
        public string RawText { get; }
        public Dictionary<string, TemplateSlotDesc> MapIdToSlotDesc { get; } = new();

        public ParsedPropertyTemplate(string templateText) {
            this.RawText = templateText;

            var regex = new Regex(
                @"^([ \t]*)\{\{([A-Z:_]+)\}\}",
                RegexOptions.Multiline
            );

            foreach (Match match in regex.Matches(templateText)) {
                var indent = match.Groups[1].Value;
                var slotId = $"{match.Groups[2].Value}";

                if (!PropertyTemplate.MapIdToSlot.TryGetValue(slotId, out var slot)) {
                    continue; // Пропустить неизвестный слот
                }

                this.MapIdToSlotDesc[slot.Id] = new TemplateSlotDesc {
                    Slot = slot,
                    Indent = indent
                };
            }
        }
    }
}