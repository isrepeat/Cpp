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


namespace CodeAnalyzer.Pipeline {
    public class CodeGenerationPipeline {
        private readonly Data.Field _field;
        private readonly Templates.ParsedPropertyTemplate _parsedPopertyTemplate;
        private readonly List<Templates.IPropertyTemplateEmitter> _emitters;
        private readonly Templates.PropertyTemplateContext _ctx;

        public CodeGenerationPipeline(
            Data.Field field,
            string templateText,
            List<Templates.IPropertyTemplateEmitter> emitters
            ) {
            _field = field;
            _parsedPopertyTemplate = new Templates.ParsedPropertyTemplate(templateText);
            _emitters = emitters;
            _ctx = new Templates.PropertyTemplateContext();
        }


        public string Generate(
            Dictionary<Templates.TemplateSlot, List<Type>> resolvedOrder,
            string indent = ""
            ) {
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

            return _ctx.Render(_parsedPopertyTemplate, indent);
        }
    }
}