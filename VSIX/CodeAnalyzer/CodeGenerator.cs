using System;
using System.Text;
using System.Linq;
using System.Threading;
using System.Collections.Generic;
using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using CodeAnalyzer.Ex;


namespace System.Runtime.CompilerServices {
    internal static class IsExternalInit { }
}


public static class AnalyzerFeatures {
    public static bool IsLogsEnabled { get; private set; } = false;

    public static void Analyze(Compilation compilation) {
        var attributes = compilation.Assembly.GetAttributes();

        AnalyzerFeatures.IsLogsEnabled = attributes
            .Any(attrData => attrData.ex_IsAttribute(typeof(Helpers.Attributes.CodeAnalyzerEnableLogsAttribute)));
    }
}


namespace CodeAnalyzer {
    [Generator]
    public sealed class CodeGenerator : IIncrementalGenerator {
        private string __logPrefix = "[CodeAnalyzer.CodeGenerator]";

        public void Initialize(IncrementalGeneratorInitializationContext context) {
            //if (!System.Diagnostics.Debugger.IsAttached) {
            //    System.Diagnostics.Debugger.Launch();
            //}

            // Compute config.
            var featureFlagsProvider = context.CompilationProvider
                .Select((compilation, _) => {
                    AnalyzerFeatures.Analyze(compilation);
                    return true;
                });

            context.RegisterSourceOutput(featureFlagsProvider, (spc, _) => {
            });


            // Build List<Field>.
            var fieldsValueProvider = context.SyntaxProvider
                .CreateSyntaxProvider(
                    predicate: this.IsCandidateField,
                    transform: this.TransformToCompoundFieldMetadata)
                .Where(f => f != null)
                .Collect();


            // Build List<Class>.
            var classesValueProvider = fieldsValueProvider
                .Select((fields, _) => {
                    var result = new List<Data.Class>();
                    var grouped = new Dictionary<INamedTypeSymbol, List<Data.Field>>(SymbolEqualityComparer.Default);

                    foreach (var field in fields) {
                        var classSymbol = field.Symbol?.ContainingType;
                        if (classSymbol == null) {
                            continue;
                        }

                        if (!grouped.TryGetValue(classSymbol, out var fieldList)) {
                            fieldList = new List<Data.Field>();
                            grouped[classSymbol] = fieldList;
                        }

                        fieldList.Add(field);
                    }

                    foreach (var entry in grouped) {
                        var symbol = entry.Key;
                        var fieldList = entry.Value;

                        result.Add(new Data.Class(symbol, fieldList));
                    }

                    return result.ToImmutableArray();
                });


            context.RegisterSourceOutput(classesValueProvider, this.GenerateCode);
        }


        private bool AnalyzeCompilation(Compilation compilation) {
            bool loggingEnabled = compilation.Assembly
            .GetAttributes()
            .Any(attr => attr.AttributeClass?.Name == "EnableCodeAnalyzerLogsAttribute");

            foreach (var attr in compilation.Assembly.GetAttributes()) {
                if (attr.AttributeClass?.ToDisplayString() == "YourAnalyzerNamespace.EnableCodeAnalyzerLogsAttribute") {
                    var arg = attr.ConstructorArguments.FirstOrDefault();
                    return arg.Value as bool? == true;
                }
            }

            return true;
        }

        private bool IsCandidateField(SyntaxNode node, CancellationToken _) {
            return node is FieldDeclarationSyntax field &&
                   field.AttributeLists.Count > 0;
        }


        private Data.Field TransformToCompoundFieldMetadata(GeneratorSyntaxContext context, CancellationToken _) {
            var fieldSyntax = (FieldDeclarationSyntax)context.Node;
            var variable = fieldSyntax.Declaration.Variables.First();
            var fieldSymbol = context.SemanticModel.GetDeclaredSymbol(variable) as IFieldSymbol;

            if (fieldSymbol == null) {
                return null;
            }

            var field = new Data.Field(fieldSyntax, fieldSymbol);
            if (field.PropertyAttributes.Count > 0) {
                return field;
            }
            return null;
        }


        private void GenerateCode(
            SourceProductionContext context,
            ImmutableArray<Data.Class> classes
            ) {
            Reporter.Msg(context, "[CodeGenerator] GenerateCode() start");
            Reporter.Msg(context, $"[CodeGenerator] classes.Count = {classes.Length}");

            foreach (var cls in classes) {
                var fields = cls.Fields;

                var className = cls.Name;
                var containingNamespace = cls.Namespace;

                var sb = new StringBuilder();
                sb.AppendLine("// Generated by 'CodeAnalyzer.CodeGenerator'");
                sb.AppendLine($"namespace {containingNamespace} {{");

                bool hasGeneratedInheritaceCode = this.TryGenerateInheritanceCode(context, cls, "        ", out var inheritanceCode);
                if (hasGeneratedInheritaceCode) {
                    sb.AppendLine($"    public partial class {className} {inheritanceCode} {{");
                }
                else {
                    sb.AppendLine($"    public partial class {className} {{");
                }

                bool hasGeneratedFieldsCode = this.TryGenerateFieldsCode(context, cls, "        ", out var fieldsCode);
                if (hasGeneratedFieldsCode) {
                    if (hasGeneratedInheritaceCode) {
                        sb.AppendLine();
                    }
                    sb.AppendLine($"{fieldsCode}");
                }

                bool hasGeneratedPropertiesCode = this.TryGeneratePropertiesCode(context, cls, "        ", out var propertiesCode);
                if (hasGeneratedPropertiesCode) {
                    if (hasGeneratedFieldsCode) {
                        sb.AppendLine();
                        sb.AppendLine();
                    }
                    sb.AppendLine($"{propertiesCode}");

                }

                bool hasGeneratedMethodsCode = this.TryGenerateMethodsCode(context, cls, "        ", out var methodsCode);
                if (hasGeneratedMethodsCode) {
                    if (hasGeneratedPropertiesCode) {
                        sb.AppendLine();
                        sb.AppendLine();
                    }
                    sb.AppendLine($"{methodsCode}");

                }

                bool hasGeneratedNestedClassesCode = this.TryGenerateNestedClassesCode(context, cls, "        ", out var nestedClassesCode);
                if (hasGeneratedNestedClassesCode) {
                    if (hasGeneratedMethodsCode) {
                        sb.AppendLine();
                        sb.AppendLine();
                    }
                    sb.AppendLine($"{nestedClassesCode}");
                }

                sb.AppendLine("    }");
                sb.AppendLine("}");

                var fileName = $"{className}.g.cs";
                context.AddSource(fileName, SourceText.From(sb.ToString(), Encoding.UTF8));

                Reporter.Msg(context, $"[CodeGenerator] generated code:");
                Reporter.Msg(context, $"{sb}", ReporterIdentationMarker.LineNumber);
            }

            Reporter.Msg(context, "[CodeGenerator] GenerateCode() end");
        }



        private bool TryGenerateInheritanceCode(
            SourceProductionContext context,
            Data.Class cls,
            string indent,
            out string result
            ) {
            result = string.Empty;

            var interfaceNames = new List<string>();

            if (cls.ex_HasFieldAttribute<Attributes.InvalidatablePropertyAttr>() ||
                cls.ex_HasFieldAttribute<Attributes.InvalidatableLazyPropertyAttr>()
                ) {
                interfaceNames.Add("Helpers.IInvalidatable");
            }

            if (interfaceNames.Count == 0) {
                return false;
            }

            string code = $":\n";
            for (int i = 0; i < interfaceNames.Count; i++) {
                bool isLastInterface = (i == interfaceNames.Count - 1);
                var interfaceName = interfaceNames[i];

                string comma = isLastInterface ? "" : ",";
                string newLine = isLastInterface ? "" : "\n";

                code += $"{indent}{interfaceName}{comma}{newLine}";
            }

            result = code;
            return result.Length > 0;
        }


        private bool TryGenerateFieldsCode(
            SourceProductionContext context,
            Data.Class cls,
            string indent,
            out string result
            ) {
            result = string.Empty;

            string newLine = $"\n{indent}";
            var code = "";

            var invalidatableFields = cls.Fields
                .Where(field => field.ex_HasAttribute<Attributes.InvalidatablePropertyAttr>() ||
                                field.ex_HasAttribute<Attributes.InvalidatableLazyPropertyAttr>())
                .ToList();

            if (invalidatableFields.Count > 0) {
                code += $"{newLine}private readonly InvalidatablePropertiesState _invalidatablePropertiesState = new();";
            }

            result = code.StartsWith("\n")
                ? code.Substring(1)
                : code;

            return result.Length > 0;
        }



        private bool TryGeneratePropertiesCode(
            SourceProductionContext context,
            Data.Class cls,
            string indent,
            out string result
            ) {
            result = string.Empty;

            string newLine = $"\n{indent}";
            string code = "";


            for (int i = 0; i < cls.Fields.Count; i++) {
                bool isLastField = (i == cls.Fields.Count - 1);
                var field = cls.Fields[i];

                if (this.TryGeneratePropertyCode(context, field, indent, out var propertyCode)) {
                    code += $"{propertyCode}";
                    if (!isLastField) {
                        code += $"\n\n\n";
                    }
                }
            }

            result = code.StartsWith("\n")
                ? code.Substring(1)
                : code;

            return result.Length > 0;
        }


        private bool TryGeneratePropertyCode(
            SourceProductionContext context,
            Data.Field field,
            string indent,
            out string result
            ) {
            result = string.Empty;

            var attrs = field.PropertyAttributes;

            // Определяем необходимость геттера и сеттера
            bool hasGetter = attrs.Any(a => a.GetterAccess == Attributes.GetterAccess.Get);
            bool hasSetter = attrs.Any(a => a.SetterAccess is Attributes.SetterAccess.Set or Attributes.SetterAccess.PrivateSet);

            var hasObservableAttribute = field.ex_TryGetAttribute<Attributes.ObservablePropertyAttr>(out var observablePropertyAttr);
            var hasInvalidatableAttribute = field.ex_TryGetAttribute<Attributes.InvalidatablePropertyAttr>(out var invalidatablePropertyAttr);
            var hasInvalidatableLazyAttribute = field.ex_TryGetAttribute<Attributes.InvalidatableLazyPropertyAttr>(out var invalidatableLazyPropertyAttr);
            var hasObservableMultiStateAttribute = field.ex_TryGetAttribute<Attributes.ObservableMultiStatePropertyAttr>(out var observableMultiStatePropertyAttr);

            // Validation:
            if (hasObservableAttribute && !hasSetter) {
                Reporter.Error(context, $"{__logPrefix} 'ObservablePropertyAttr' need setter, but another attribute suppress setter");
                return false;
            }
            if (hasInvalidatableAttribute && hasInvalidatableLazyAttribute) {
                Reporter.Error(context, $"{__logPrefix} Do not support both attributes 'InvalidatablePropertyAttr' and 'InvalidatableLazyPropertyAttr'");
                return false;
            }


            var emitters = new List<Templates.IPropertyTemplateEmitter>();
            emitters.Add(new Attributes.BasePropertyAttr(field));

            if (hasObservableAttribute) {
                emitters.Add(observablePropertyAttr);
            }
            if (hasInvalidatableAttribute) {
                emitters.Add(invalidatablePropertyAttr);
            }
            if (hasInvalidatableLazyAttribute) {
                emitters.Add(invalidatableLazyPropertyAttr);
            }
            if (hasObservableMultiStateAttribute) {
                emitters.Add(observableMultiStatePropertyAttr);
            }

            var conflictResolvingMap = new Dictionary<Templates.TemplateSlot, List<Type>>();

            conflictResolvingMap[Templates.PropertyTemplate.Set.AFTER_ASSIGNMENT] = new() {
                invalidatableLazyPropertyAttr?.GetType(),
                invalidatablePropertyAttr?.GetType(),
                observableMultiStatePropertyAttr?.GetType(),
                observablePropertyAttr?.GetType(),
            };

            //if (hasObservableAttribute &&
            //    hasInvalidatableAttribute
            //    ) {
            //    conflictResolvingMap[Templates.PropertyTemplate.Set.AFTER_ASSIGNMENT] = new() {
            //        invalidatablePropertyAttr?.GetType(),
            //        observablePropertyAttr?.GetType(),
            //    };
            //}
            //if (hasObservableAttribute &&
            //    hasInvalidatableLazyAttribute
            //    ) {
            //    conflictResolvingMap[Templates.PropertyTemplate.Set.AFTER_ASSIGNMENT] = new() {
            //        invalidatableLazyPropertyAttr?.GetType(),
            //        observablePropertyAttr?.GetType(),
            //    };
            //}


            var pipeline = new Pipeline.CodeGenerationPipeline(
                field,
                Templates.PropertyTemplate.Property.Template,
                emitters);


            string newLine = $"\n{indent}";
            string code = "";

            code += pipeline.Generate(conflictResolvingMap, indent);

            result = code.StartsWith("\n")
                ? code.Substring(1)
                : code;

            return result.Length > 0;
        }



        private bool TryGenerateMethodsCode(
            SourceProductionContext context,
            Data.Class cls,
            string indent,
            out string result
            ) {
            result = string.Empty;

            string newLine = $"\n{indent}";
            string code = "";

            var invalidatableFields = cls.Fields
                .Where(field => field.ex_HasAttribute<Attributes.InvalidatablePropertyAttr>() ||
                                field.ex_HasAttribute<Attributes.InvalidatableLazyPropertyAttr>())
                .ToList();

            if (invalidatableFields.Count > 0) {
                code += $"{newLine}private void InvalidateProperties() {{";
                code += $"{newLine}    _invalidatablePropertiesState.InvalidateAll();";
                code += $"{newLine}}}";
            }

            result = code.StartsWith("\n")
                ? code.Substring(1)
                : code;

            return result.Length > 0;
        }



        private bool TryGenerateNestedClassesCode(
            SourceProductionContext context,
            Data.Class cls,
            string indent,
            out string result
            ) {
            result = string.Empty;

            string newLine = $"\n{indent}";
            string code = "";
            
            //
            // Generate InvalidatablePropertiesState class
            //
            var invalidatableFields = cls.Fields
                .Where(field => field.ex_HasAttribute<Attributes.InvalidatablePropertyAttr>() ||
                                field.ex_HasAttribute<Attributes.InvalidatableLazyPropertyAttr>())
                .ToList();

            if (invalidatableFields.Count > 0) {
                code += $"{newLine}private sealed class InvalidatablePropertiesState {{";

                foreach (var f in invalidatableFields) {
                    code += $"{newLine}    public {f.TypeName}? {f.PropName}Cached {{ get; set; }}";
                    code += $"{newLine}    public bool Is{f.PropName}Valid {{ get; private set; }} = true;";
                }

                //code += $"{newLine}";
                //code += $"{newLine}    public InvalidatablePropertiesState(";

                //for (int i = 0; i < invalidatableFields.Count; i++) {
                //    var f = invalidatableFields[i];
                //    bool isLastField = (i == invalidatableFields.Count - 1);
                    
                //    string comma = isLastField ? "" : ",";
                    
                //    code += $"{newLine}        {f.TypeName} {f.Name}{comma}";
                //}

                //code += $"{newLine}    ) {{";

                //foreach (var f in invalidatableFields) {
                //    code += $"{newLine}        this.{f.PropName}Cached = {f.Name};";
                //    code += $"{newLine}        this.Is{f.PropName}Valid = true;";
                //}

                //code += $"{newLine}    }}";

                code += $"{newLine}";
                code += $"{newLine}    public void InvalidateAll() {{";

                foreach (var f in invalidatableFields) {
                    code += $"{newLine}        this.Is{f.PropName}Valid = false;";
                }

                code += $"{newLine}    }}";
                code += $"{newLine}}}";
            }

            result = code.StartsWith("\n")
                ? code.Substring(1)
                : code;

            return result.Length > 0;
        }
    }
}