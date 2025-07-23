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
        private readonly Templates.Template _rootTemplate;
        private readonly List<Templates.IPropertyTemplateEmitter> _emitters;
        private readonly Templates.PropertyTemplateContext _ctx;

        public CodeGenerationPipeline(
            Data.Field field,
            Templates.Template rootTemplate,
            List<Templates.IPropertyTemplateEmitter> emitters
            ) {
            _field = field;
            _rootTemplate = rootTemplate;
            _emitters = emitters;
            _ctx = new Templates.PropertyTemplateContext();
        }


        public string Generate(
            Dictionary<Templates.TemplateSlot, List<Type>> conflictResolvingMap,
            string indent = ""
            ) {
            foreach (var emitter in _emitters) {
                emitter.EmitToPropertyTemplate(_field, _ctx);
            }

            foreach (var kvp in conflictResolvingMap) {
                var slot = kvp.Key;
                var resolvedEmitersOrder = kvp.Value;

                _ctx.ResolveConflicts(slot, resolvedEmitersOrder);
            }

            var types = _emitters.Select(e => e.GetType());
            _ctx.FallbackResolve(types);

            return _ctx.Render(_rootTemplate, indent);
        }
    }
}