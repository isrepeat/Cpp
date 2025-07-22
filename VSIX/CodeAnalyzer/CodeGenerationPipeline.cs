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
using CodeAnalyzer;
using System.Reflection;


namespace CodeAnalyzer {
    public class CodeGenerationPipeline {
        private readonly Field _field;
        private readonly ParsedTemplate _parsedTemplate;
        private readonly List<IPropertyTemplateEmitter> _emitters;
        private readonly PropertyTemplateContext _ctx;

        public CodeGenerationPipeline(
            Field field,
            string templateText,
            List<IPropertyTemplateEmitter> emitters
            ) {
            _field = field;
            _parsedTemplate = new ParsedTemplate(templateText);
            _emitters = emitters;
            _ctx = new PropertyTemplateContext();
        }

        public string Generate(Dictionary<Slot, List<Type>> resolvedOrder, string indent = "") {
            foreach (var emitter in _emitters) {
                emitter.Emit(_field, _ctx);
            }

            foreach (var kvp in resolvedOrder) {
                var slot = kvp.Key;
                var order = kvp.Value;

                _ctx.ResolveConflicts(slot, order);
            }

            var types = _emitters.Select(e => e.GetType());
            _ctx.FallbackResolve(types);

            return _ctx.Render(_parsedTemplate, indent);
        }
    }



    public class PropertyTemplateContext {
        private readonly Dictionary<Slot, CodeBlock> _mapSlotToCodeBlock = new();
        private readonly Dictionary<Slot, List<CodeBlock>> _mapSlotToListConflict = new();

        public void InsertCode(Slot slot, string code, Type emitterType) {
            var newBlock = new CodeBlock {
                EmitterType = emitterType,
                Code = code
            };

            // 1. Если уже конфликт — просто добавляем в список
            if (_mapSlotToListConflict.TryGetValue(slot, out var conflictList)) {
                conflictList.Add(newBlock);
                return;
            }

            // 2. Если слота ещё нет — просто вставляем
            if (!_mapSlotToCodeBlock.ContainsKey(slot)) {
                _mapSlotToCodeBlock[slot] = newBlock;
                return;
            }

            // 3. Конфликт впервые — переносим в список + добавляем новый
            var existing = _mapSlotToCodeBlock[slot];
            _mapSlotToCodeBlock.Remove(slot);

            _mapSlotToListConflict[slot] = new List<CodeBlock> {
                existing,
                newBlock
            };
        }


        public void ResolveConflicts(Slot slot, List<Type> order) {
            if (!_mapSlotToListConflict.TryGetValue(slot, out var conflicts)) {
                return;
            }

            var resolved = new List<string>();

            foreach (var type in order) {
                var entry = conflicts.FirstOrDefault(e => e.EmitterType == type);
                if (entry == null) {
                    throw new InvalidOperationException(
                        $"Type {type.Name} not found in conflict for slot '{slot}'"
                    );
                }
                resolved.Add(entry.Code);
            }

            _mapSlotToCodeBlock[slot] = new CodeBlock {
                EmitterType = null,
                Code = string.Join("\n", resolved)
            };
            _mapSlotToListConflict.Remove(slot);
        }


        public void FallbackResolve(IEnumerable<Type> orderedTypes) {
            foreach (var kvp in _mapSlotToListConflict.ToList()) {
                var slot = kvp.Key;
                var conflicts = kvp.Value;

                var resolved = orderedTypes
                    .Where(t => conflicts.Any(e => e.EmitterType == t))
                    .Select(t => conflicts.First(e => e.EmitterType == t).Code)
                    .ToList();

                _mapSlotToCodeBlock[slot] = new CodeBlock {
                    EmitterType = null,
                    Code = string.Join("\n", resolved)
                };

                _mapSlotToListConflict.Remove(slot);
            }
        }


        public string Render(ParsedTemplate template, string indent) {
            var result = template.RawText;
            
            foreach (var kvp in _mapSlotToCodeBlock) {
                var slot = kvp.Key;
                var codeBlock = kvp.Value;

                var resultCode = "";

                if (slot.Kind == SlotKind.Inline) {
                    resultCode = codeBlock.Code;
                }
                else if (slot.Kind == SlotKind.Multiline) {
                    string slotIndent = template.MapIdToSlotDesc.TryGetValue(slot.Id, out var slotDesc)
                        ? slotDesc.Indent
                        : "";

                    var lines = codeBlock.Code.Split('\n');

                    if (lines.Length == 0) {
                        resultCode = "";
                    }
                    else if (lines.Length == 1) {
                        resultCode = lines[0];
                    }
                    else {
                        resultCode = lines[0] + "\n";

                        var indentedCodeLines = lines
                            .Skip(1)
                            .Select(line => slotIndent + line);

                        resultCode += string.Join("\n", indentedCodeLines);
                    }
                }

                result = result.Replace($"{{{{{slot.Id}}}}}", resultCode);
            }

            // Remove other slot-placeholders that was not handled.
            result = Regex.Replace(
                result,
                @"^[ \t]*\{\{[A-Z:_]+\}\}[ \t]*(\r?\n)?",
                "",
                RegexOptions.Multiline
            );

            // Добавить внешний отступ ко всем строкам результата.
            var finalLines = result.Split('\n');
            var finalIndentedLines = finalLines.Select(line => indent + line);
            result = string.Join("\n", finalIndentedLines);
            return result;
        }
    }


    public class CodeBlock {
        public Type EmitterType;
        public string Code;
    }



    public enum SlotKind {
        Inline,
        Multiline,
    }

    public class Slot {
        public string Id { get; }
        public SlotKind Kind { get; }

        public Slot(string id, SlotKind slotKind = SlotKind.Multiline) {
            this.Id = id;
            this.Kind = slotKind;
        }


        public override string ToString() {
            return $"{{{{{this.Id}}}}}";
        }

        public override bool Equals(object? obj) {
            if (obj is not Slot other) {
                return false;
            }
            return this.Id == other.Id && this.Kind == other.Kind;
        }

        public override int GetHashCode() {
            int hash = 17;
            hash = hash * 23 + (this.Id?.GetHashCode() ?? 0);
            hash = hash * 23 + this.Kind.GetHashCode();
            return hash;
        }
    }


    public class SlotDesc {
        public Slot Slot { get; init; }
        public string Indent { get; init; }
    }


    public static class PropertyTemplate {
        public static readonly Slot TYPE_NAME = new("TYPE_NAME", SlotKind.Inline);
        public static readonly Slot PROP_NAME = new("PROP_NAME", SlotKind.Inline);
        public static readonly Slot FIELD_NAME = new("FIELD_NAME", SlotKind.Inline);

        public static class GET {
            public static readonly Slot MODIFICATOR = new("GET:MODIFICATOR", SlotKind.Inline);
            public static readonly Slot BEGIN = new("GET:BEGIN");
        }

        public static class SET {
            public static readonly Slot MODIFICATOR = new("SET:MODIFICATOR", SlotKind.Inline);
            public static readonly Slot BEGIN = new("SET:BEGIN");
            public static readonly Slot BEFORE_ASSIGNMENT = new("SET:BEFORE_ASSIGNMENT");
            public static readonly Slot AFTER_ASSIGNMENT = new("SET:AFTER_ASSIGNMENT");
            public static readonly Slot END = new("SET:END");
        }

        public static readonly HashSet<Slot> Slots = PropertyTemplate.CollectSlots();
        public static readonly IReadOnlyDictionary<string, Slot> MapIdToSlot = Slots.ToDictionary(s => s.Id);

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



        private static HashSet<Slot> CollectSlots() {
            var flags = BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy;
            var result = new HashSet<Slot>();

            // Добавляем слоты из PropertyTemplate и всех вложенных типов
            foreach (var type in PropertyTemplate.GetAllNestedTypesIncludingSelf(typeof(PropertyTemplate))) {
                var fields = type.GetFields(flags)
                    .Where(f => f.FieldType == typeof(Slot))
                    .Select(f => (Slot)f.GetValue(null)!);

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


    public class ParsedTemplate {
        public string RawText { get; }
        public Dictionary<string, SlotDesc> MapIdToSlotDesc { get; } = new();

        public ParsedTemplate(string templateText) {
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

                this.MapIdToSlotDesc[slot.Id] = new SlotDesc {
                    Slot = slot,
                    Indent = indent
                };
            }
        }
    }

   


    public interface IPropertyTemplateEmitter {
        void Emit(Field field, PropertyTemplateContext ctx);
    }

    public class BasePropertyEmitter : IPropertyTemplateEmitter {
        public void Emit(Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(PropertyTemplate.TYPE_NAME, $"{field.TypeName}", this.GetType());
            ctx.InsertCode(PropertyTemplate.PROP_NAME, $"{field.PropName}", this.GetType());
            ctx.InsertCode(PropertyTemplate.FIELD_NAME, $"{field.Name}", this.GetType());
            ctx.InsertCode(PropertyTemplate.GET.MODIFICATOR, $"", this.GetType());
            ctx.InsertCode(PropertyTemplate.SET.MODIFICATOR, $"", this.GetType());
        }
    }


    public class AttachableEmitter : IPropertyTemplateEmitter {
        public void Emit(Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.SET.AFTER_ASSIGNMENT,
                $"int aaa = 1;",
                this.GetType()
                );
        }
    }

    public class ObservableEmitter : IPropertyTemplateEmitter {
        public void Emit(Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.SET.AFTER_ASSIGNMENT,
                $"int bbb = 1;",
                this.GetType()
            );
        }
    }

    public class GlobalListeningEmitter : IPropertyTemplateEmitter {
        public void Emit(Field field, PropertyTemplateContext ctx) {
            ctx.InsertCode(
                PropertyTemplate.SET.AFTER_ASSIGNMENT,
                $"int ccc = 1;",
                this.GetType()
                );
        }
    }
}