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


namespace CodeAnalyzer {
    //public static class PropertyTemplate {
    //    public const string Base = """
    //        public {{TYPE_NAME}} {{PROP_NAME}} {
    //            get {
    //                {{GET:START}}
    //                return {{FIELD_NAME}};
    //            }
    //            set {
    //                {{SET:START}}
    //                if (!object.ReferenceEquals({{FIELD_NAME}}, value)) {
    //                    {{SET:BEFORE_ASSIGNMENT}}
    //                    {{FIELD_NAME}} = value;
    //                    {{SET:AFTER_ASSIGNMENT}}
    //                }
    //                {{SET:END}}
    //            }
    //        }
    //        """;
    //}
}