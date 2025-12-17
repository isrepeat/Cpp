using System;
using System.Text.RegularExpressions;
using System.Text;
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
    public class Template {
        public string Text { get; }
        public Type[] UsedTemplateSlotOwners { get; }
        public Dictionary<TemplateSlot, TemplateSlotDesc> MapSlotToSlotDesc { get; } = new();


        public Template(string text, Type[] usedTemplateSlotOwners) {
            this.Text = text;
            this.UsedTemplateSlotOwners = usedTemplateSlotOwners;

            this.Parse(this.Text);
        }

        private void Parse(string text) {
            var usedSlots = Helpers.Reflaction.FieldsUtils
                .CollectAllStaticFieldsOfType<TemplateSlot>(this.UsedTemplateSlotOwners);

            var mapIdToSlot = usedSlots.ToDictionary(s => s.Id);

            var regex = new Regex(@"\{\{([A-Z:_]+)\}\}");

            foreach (Match match in regex.Matches(text)) {
                var slotId = match.Groups[1].Value;

                if (!mapIdToSlot.TryGetValue(slotId, out var slot)) {
                    throw new InvalidOperationException($"Unrecognized slot '{slotId}'");
                }

                string indent = this.TryExtractIndentBeforeSlot(text, match.Index);

                this.MapSlotToSlotDesc[slot] = new TemplateSlotDesc {
                    Indent = indent
                };
            }
        }


        private string TryExtractIndentBeforeSlot(string text, int slotStartIndex) {
            if (slotStartIndex <= 0) {
                return "";
            }

            // Находим индекс начала строки, где расположен {{SLOT_ID}}
            // Ищем последний \n перед позицией слота
            int lineStart = text.LastIndexOf('\n', slotStartIndex - 1);

            // Если не найден — значит, это первая строка (lineStart = 0)
            // Иначе — смещаемся вперёд на 1, чтобы попасть на первый символ строки
            int indentStart = (lineStart == -1) ? 0 : lineStart + 1;

            // Извлекаем префикс от начала строки до {{SLOT_ID}}
            string prefix = text.Substring(indentStart, slotStartIndex - indentStart);

            // Если этот префикс состоит только из пробелов и табов — это отступ
            // Иначе (например, "return {{FIELD}}") — слот встроен в код, отступ отсутствует
            return prefix.All(c => c == ' ' || c == '\t') ? prefix : "";
        }
    }
}