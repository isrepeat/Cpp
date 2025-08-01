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


namespace CodeAnalyzer.Formatters {
    /// <summary>
    /// Удаляет указанный rootNamespace из начала всех имен типов (включая generic аргументы).
    /// Поддерживает вложенные generic-типы.
    /// Пример:
    ///     RootNamespace = "MyApp.Models"
    ///     Input: MyApp.Models.MyType<MyApp.Models.Inner>
    ///     Output: MyType<Inner>
    /// </summary>
    public sealed class SimplifyNamespaceRule : ITypeFormatterRule {
        private readonly string _rootNamespace;

        public SimplifyNamespaceRule(string rootNamespace) {
            _rootNamespace = rootNamespace.TrimEnd('.') + ".";
        }

        public string Apply(string fullTypeName) {
            return SimplifyRecursive(fullTypeName);
        }

        /// <summary>
        /// Разбирает строку с generic-типом и рекурсивно упрощает generic-аргументы.
        /// </summary>
        private string SimplifyRecursive(string typeName) {
            int genericStart = typeName.IndexOf('<');
            if (genericStart == -1) {
                return SimplifyOne(typeName);
            }

            int genericEnd = typeName.LastIndexOf('>');
            string outerType = typeName.Substring(0, genericStart);
            string genericArgs = typeName.Substring(genericStart + 1, genericEnd - genericStart - 1);

            var simplifiedArgs = SplitGenericArguments(genericArgs)
                .Select(SimplifyRecursive);

            return $"{SimplifyOne(outerType)}<{string.Join(", ", simplifiedArgs)}>";
        }

        /// <summary>
        /// Удаляет rootNamespace из имени типа, если оно начинается с него.
        /// </summary>
        private string SimplifyOne(string type) {
            return type.StartsWith(_rootNamespace)
                ? type.Substring(_rootNamespace.Length)
                : type;
        }

        /// <summary>
        /// Разбивает список generic-аргументов с учётом вложенности.
        /// Например: Dictionary<string, List<int>> → ["Dictionary<string, List<int>>"]
        /// </summary>
        private static List<string> SplitGenericArguments(string genericArgs) {
            var result = new List<string>();
            int depth = 0;
            int lastIndex = 0;

            for (int i = 0; i < genericArgs.Length; i++) {
                char c = genericArgs[i];

                if (c == '<') {
                    depth++;
                }
                else if (c == '>') {
                    depth--;
                }
                else if (c == ',' && depth == 0) {
                    result.Add(genericArgs.Substring(lastIndex, i - lastIndex).Trim());
                    lastIndex = i + 1;
                }
            }

            result.Add(genericArgs.Substring(lastIndex).Trim());
            return result;
        }
    }
}