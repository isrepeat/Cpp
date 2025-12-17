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


namespace CodeAnalyzer.Data {
    public abstract class FieldMetadataBase {
        public FieldDeclarationSyntax Syntax { get; }
        public IFieldSymbol Symbol { get; }
        public string Name { get; }
        public string PropName { get; }
        public string TypeName { get; }
        public string ContainingNamespace { get; }

        public FieldMetadataBase(FieldDeclarationSyntax syntax, IFieldSymbol symbol) {
            this.Syntax = syntax;
            this.Symbol = symbol;

            this.Name = symbol.Name;
            this.PropName = this.GetPropertyNameFromFieldName(this.Name);
            this.TypeName = symbol.Type.ToDisplayString();
            this.ContainingNamespace = symbol.ContainingNamespace.ToDisplayString();
        }

        private string GetPropertyNameFromFieldName(string fieldName) {
            string coreName = fieldName.StartsWith("_")
                ? fieldName.Substring(1)
                : fieldName;

            return char.ToUpperInvariant(coreName[0]) + coreName.Substring(1);
        }
    }


    public sealed class Field : FieldMetadataBase {
        private List<Attributes.PropertyAttributeBase> _propertyAttributes = new();
        public IReadOnlyList<Attributes.PropertyAttributeBase> PropertyAttributes => _propertyAttributes;

        public Field(
            FieldDeclarationSyntax fieldSyntax,
            IFieldSymbol fieldSymbol
            ) : base(fieldSyntax, fieldSymbol) {

            foreach (var attributeData in fieldSymbol.GetAttributes()) {
                if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.ObservablePropertyAttribute))) {
                    _propertyAttributes.Add(new Attributes.ObservablePropertyAttr(attributeData));
                }
                else if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.ObservableMultiStatePropertyAttribute))) {
                    _propertyAttributes.Add(new Attributes.ObservableMultiStatePropertyAttr(attributeData));
                }
                else if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.InvalidatablePropertyAttribute))) {
                    _propertyAttributes.Add(new Attributes.InvalidatablePropertyAttr(attributeData));
                }
                else if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.InvalidatableLazyPropertyAttribute))) {
                    _propertyAttributes.Add(new Attributes.InvalidatableLazyPropertyAttr(attributeData));
                }
            }
        }
    }
}