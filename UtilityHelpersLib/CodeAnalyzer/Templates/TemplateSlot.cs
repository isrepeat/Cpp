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
    public enum SlotKind {
        Inline,
        Multiline,
    }

    public class TemplateSlot {
        public string Id { get; }
        public SlotKind Kind { get; }

        public TemplateSlot(string id, SlotKind slotKind = SlotKind.Multiline) {
            Id = id;
            Kind = slotKind;
        }


        public override string ToString() {
            return $"{{{{{Id}}}}}";
        }

        public override bool Equals(object obj) {
            if (obj is not TemplateSlot other) {
                return false;
            }
            return Id == other.Id;
        }

        public override int GetHashCode() {
            int hash = 17;
            hash = hash * 23 + (Id?.GetHashCode() ?? 0);
            return hash;
        }
    }


    public class TemplateSlotDesc {
        public string Indent { get; init; }
    }
}