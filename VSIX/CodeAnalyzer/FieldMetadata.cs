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


namespace CodeAnalyzer {
    public abstract class FieldMetadataBase {
        public FieldDeclarationSyntax Syntax { get; }
        public IFieldSymbol Symbol { get; }
        public string Name { get; }
        public string PropName { get; }
        public string TypeName { get; }

        public FieldMetadataBase(FieldDeclarationSyntax syntax, IFieldSymbol symbol) {
            this.Syntax = syntax;
            this.Symbol = symbol;

            this.Name = symbol.Name;
            this.PropName = this.GetPropertyNameFromFieldName(this.Name);
            this.TypeName = symbol.Type.ToDisplayString();
        }

        private string GetPropertyNameFromFieldName(string fieldName) {
            string coreName = fieldName.StartsWith("_")
                ? fieldName.Substring(1)
                : fieldName;

            return char.ToUpperInvariant(coreName[0]) + coreName.Substring(1);
        }
    }



    public enum GetterAccess {
        None,
        Get,
    }
    public enum SetterAccess {
        None,
        Set,
        PrivateSet,
    }

    public abstract class PropertyAttributeBase {
        public GetterAccess GetterAccess { get; protected set; } = GetterAccess.None;
        public SetterAccess SetterAccess { get; protected set; } = SetterAccess.None;
    }


    public sealed class ObservablePropertyAttr : PropertyAttributeBase {
        public ObservablePropertyAttr(AttributeData attributeData) {
            int argIndex = 0;

            if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
            argIndex++;

            if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Set>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.Set;
            }
            else if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.PrivateSet;
            }
        }
    }


    public sealed class InvalidatablePropertyAttr : PropertyAttributeBase {
        public InvalidatablePropertyAttr(AttributeData attributeData) {
            int argIndex = 0;

            if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
            //else if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
            //    this.SetterAccess = SetterAccess.PrivateSet;
            //}
            argIndex++;

            if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Set>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.Set;
            }
            else if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.PrivateSet>(argIndex, out _)) {
                this.SetterAccess = SetterAccess.PrivateSet;
            }
        }
    }


    public sealed class InvalidatableLazyPropertyAttr : PropertyAttributeBase {
        public string FactoryMethodName { get; }

        public InvalidatableLazyPropertyAttr(AttributeData attributeData) {
            int argIndex = 0;

            if (attributeData.TryGetConstructorArgumentValue<string>(argIndex, out var arg0_factoryMethodName)) {
                this.FactoryMethodName = arg0_factoryMethodName;
            }    
            else if (attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
            argIndex++;

            if (attributeData.TryGetConstructorArgumentValue<string>(argIndex, out var arg1_factoryMethodName)) {
                this.FactoryMethodName = arg1_factoryMethodName;
            }
            else if(attributeData.TryGetConstructorArgumentValue<Helpers.Attributes.Markers.Access.Get>(argIndex, out _)) {
                this.GetterAccess = GetterAccess.Get;
            }
        }
    }


    public sealed class Field : FieldMetadataBase {
        private List<PropertyAttributeBase> _propertyAttributes = new();
        public IReadOnlyList<PropertyAttributeBase> PropertyAttributes => _propertyAttributes;

        public Field(
            FieldDeclarationSyntax fieldSyntax,
            IFieldSymbol fieldSymbol
            ) : base(fieldSyntax, fieldSymbol) {

            foreach (var attributeData in fieldSymbol.GetAttributes()) {
                if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.ObservablePropertyAttribute))) {
                    _propertyAttributes.Add(new ObservablePropertyAttr(attributeData));
                }
                else if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.InvalidatablePropertyAttribute))) {
                    _propertyAttributes.Add(new InvalidatablePropertyAttr(attributeData));
                }
                else if (attributeData.ex_IsAttribute(typeof(Helpers.Attributes.InvalidatableLazyPropertyAttribute))) {
                    _propertyAttributes.Add(new InvalidatableLazyPropertyAttr(attributeData));
                }
            }
        }
    }
}