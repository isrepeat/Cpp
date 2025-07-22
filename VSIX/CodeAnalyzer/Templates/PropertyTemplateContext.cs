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
    public class CodeBlock {
        public Type EmitterType;
        public string Code;
    }


    public class PropertyTemplateContext {
        private readonly Dictionary<TemplateSlot, CodeBlock> _mapSlotToCodeBlock = new();
        private readonly Dictionary<TemplateSlot, List<CodeBlock>> _mapSlotToListConflict = new();

        public void InsertCode(TemplateSlot slot, string code, Type emitterType) {
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


        public void ResolveConflicts(TemplateSlot slot, List<Type> order) {
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


        public string Render(ParsedPropertyTemplate parsedPropertyTemplate, string indent) {
            var result = parsedPropertyTemplate.RawText;

            foreach (var kvp in _mapSlotToCodeBlock) {
                var slot = kvp.Key;
                var codeBlock = kvp.Value;

                var resultCode = "";

                if (slot.Kind == SlotKind.Inline) {
                    resultCode = codeBlock.Code;
                }
                else if (slot.Kind == SlotKind.Multiline) {
                    string slotIndent = parsedPropertyTemplate.MapIdToSlotDesc.TryGetValue(slot.Id, out var slotDesc)
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
}