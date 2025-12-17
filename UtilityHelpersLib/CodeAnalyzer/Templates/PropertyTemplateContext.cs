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
    public abstract class TemplateReplacement {
        public Type EmitterType;
    }

    public class TemplateBlock : TemplateReplacement {
        public Template Template;
    }

    public class CodeBlock : TemplateReplacement {
        public string Code;
    }


    public class PropertyTemplateContext {
        private readonly Dictionary<TemplateSlot, TemplateReplacement> _mapSlotToTemplateReplacement = new();
        private readonly Dictionary<TemplateSlot, List<TemplateReplacement>> _mapSlotToListConflict = new();

        public void InsertTemplate(TemplateSlot slot, Template template, Type emitterType) {
            var templateBlock = new TemplateBlock {
                EmitterType = emitterType,
                Template = template
            };

            this.InsertBlock(slot, templateBlock);
        }


        public void InsertCode(TemplateSlot slot, string code, Type emitterType) {
            var codeBlock = new CodeBlock {
                EmitterType = emitterType,
                Code = code
            };

            this.InsertBlock(slot, codeBlock);
        }


        public DeferredCodeBuilder GetCodeBuilder(TemplateSlot slot,  Type emitterType) {
            return new DeferredCodeBuilder(this, slot, emitterType);
        }

        private void InsertBlock(TemplateSlot slot, TemplateReplacement newBlock) {
            // 1. Если уже конфликт — просто добавляем в список
            if (_mapSlotToListConflict.TryGetValue(slot, out var conflictList)) {
                conflictList.Add(newBlock);
                return;
            }

            // 2. Если слота ещё нет — просто вставляем
            if (!_mapSlotToTemplateReplacement.ContainsKey(slot)) {
                _mapSlotToTemplateReplacement[slot] = newBlock;
                return;
            }

            // 3. Конфликт впервые — переносим в список + добавляем новый
            var existingBlock = _mapSlotToTemplateReplacement[slot];
            _mapSlotToTemplateReplacement.Remove(slot);

            _mapSlotToListConflict[slot] = new List<TemplateReplacement> {
                existingBlock,
                newBlock
            };
        }


        public void ResolveConflicts(TemplateSlot slot, List<Type> resolvedEmitersOrder) {
            if (!_mapSlotToListConflict.TryGetValue(slot, out var conflicts)) {
                return;
            }

            if (conflicts.OfType<TemplateBlock>().Count() > 0) {
                throw new InvalidOperationException(
                    $"Do not support conflict resolving for TemplateBlock");
            }

            var codeBlockConficts = conflicts.OfType<CodeBlock>();
            var resolvedCodeLines = new List<string>();

            foreach (var emmiterType in resolvedEmitersOrder) {
                if (emmiterType == null) {
                    continue;
                }

                var codeBlock = codeBlockConficts.FirstOrDefault(e => e.EmitterType == emmiterType);
                if (codeBlock == null) {
                    throw new InvalidOperationException(
                        $"Type {emmiterType.Name} not found in conflict for slot '{slot}'");
                }

                resolvedCodeLines.AddRange(codeBlock.Code.Split('\n'));
            }

            _mapSlotToTemplateReplacement[slot] = new CodeBlock {
                EmitterType = null,
                Code = string.Join("\n", resolvedCodeLines)
            };
            _mapSlotToListConflict.Remove(slot);
        }


        public void FallbackResolve(IEnumerable<Type> orderedTypes) {
            foreach (var kvp in _mapSlotToListConflict.ToList()) {
                var slot = kvp.Key;
                var conflicts = kvp.Value;
                var codeBlockConficts = conflicts.OfType<CodeBlock>();

                var resolved = orderedTypes
                    .Where(t => codeBlockConficts.Any(e => e.EmitterType == t))
                    .Select(t => codeBlockConficts.First(e => e.EmitterType == t).Code)
                    .ToList();

                _mapSlotToTemplateReplacement[slot] = new CodeBlock {
                    EmitterType = null,
                    Code = string.Join("\n", resolved)
                };

                _mapSlotToListConflict.Remove(slot);
            }
        }


        public string Render(Template rootTemplate, string indent) {
            var result = this.RenderRecursive(rootTemplate, indent);
            result = this.CleanupUnusedSlots(result);
            return indent + result;
        }


        private string RenderRecursive(Template template, string indent) {
            string result = template.Text;

            foreach (var kvp in template.MapSlotToSlotDesc) {
                var slot = kvp.Key;
                var slotDesc = kvp.Value;

                string replacement;

                if (_mapSlotToListConflict.TryGetValue(slot, out var conflictList)) {
                    var emitters = string.Join(", ", conflictList.Select(e => e.EmitterType?.Name ?? "unknown"));
                    replacement = $"#error Conflicting emitters for slot {slot.Id}: {emitters}";
                }
                else if (_mapSlotToTemplateReplacement.TryGetValue(slot, out var block)) {
                    switch (block) {
                        case CodeBlock codeBlock:
                            replacement = this.ApplyIndent(codeBlock.Code, slotDesc.Indent, addIndentToFirstLine: false);
                            break;

                        case TemplateBlock templateBlock:
                            replacement = this.RenderRecursive(templateBlock.Template, slotDesc.Indent);
                            break;

                        default:
                            throw new InvalidOperationException($"Unsupported block type: {block.GetType().Name}");
                    }
                }
                else {
                    // Слот не был заполнен — оставим маркер
                    replacement = $"[[UNUSED_SLOT:{slot.Id}]]";
                }

                result = result.Replace($"{{{{{slot.Id}}}}}", replacement);
            }

            result = this.ApplyIndent(result, indent, addIndentToFirstLine: false);
            return result;
        }


        private string ApplyIndent(string text, string indent, bool addIndentToFirstLine) {
            var lines = text.Split('\n');

            if (lines.Length == 0) {
                return string.Empty;
            }

            var indentedFirstLine = addIndentToFirstLine
                ? indent + lines[0]
                : lines[0];

            var indentedRestLines = lines
                .Skip(1)
                .Select(line => indent + line);

            return string.Join("\n", new[] { indentedFirstLine }.Concat(indentedRestLines));
        }


        private string CleanupUnusedSlots(string text) {
            var lines = text.Split('\n');
            var finalLines = new List<string>();

            foreach (var line in lines) {
                string trimmed = line.Trim();

                // Поиск маркера незаполненного слота.
                var match = Regex.Match(trimmed, @"\[\[UNUSED_SLOT:([A-Z:_]+)\]\]");

                // Если строка не содержит необработанных маркеров - добавляем ее.
                if (!match.Success) {
                    finalLines.Add(line);
                    continue;
                }

                string fullMatch = match.Value;
                string slotId = match.Groups[1].Value;

                // Есл строка содержат только лишь необработанный маркер - игнорируем ее.
                if (trimmed == fullMatch) {
                    continue;
                }

                // Есл строка содержат специальный необработанный маркер - удаляем его.
                if (slotId == "EXTRAS") {
                    finalLines.Add(line.Replace(fullMatch, ""));
                    continue;
                }

                // Иначе — необработанный маркер встроен в код, это ошибка шаблона
                throw new InvalidOperationException(
                    $"Slot '{slotId}' was not filled but is embedded in code:\n{line}"
                );
            }

            return string.Join("\n", finalLines);
        }
    }
}