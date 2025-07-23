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
    /// Форматирует generic-типы так, чтобы каждый параметр был выведен на новой строке.
    /// Применяется только если параметров больше двух.
    /// Пример:
    ///     Input:  SomeType<A, B, C>?
    ///     Output: SomeType<
    ///                 A,
    ///                 B,
    ///                 C
    ///             >?
    /// </summary>
    public sealed class MultilineGenericRule : ITypeFormatterRule {
        public string Apply(string fullTypeName) {
            int genericStart = fullTypeName.IndexOf('<');
            int genericEnd = fullTypeName.LastIndexOf('>');

            // Не generic — не форматируем
            if (genericStart == -1 || genericEnd == -1 || genericEnd <= genericStart) {
                return fullTypeName;
            }

            string baseType = fullTypeName.Substring(0, genericStart);
            string genericContent = fullTypeName.Substring(genericStart + 1, genericEnd - genericStart - 1);
            string[] typeArgs = genericContent.Split(',').Select(t => t.Trim()).ToArray();

            // Если один аргумент — оставляем однострочно
            if (typeArgs.Length <= 2) {
                return fullTypeName;
            }

            var sb = new StringBuilder();
            sb.AppendLine($"{baseType}<");

            for (int i = 0; i < typeArgs.Length; i++) {
                bool isLast = (i == typeArgs.Length - 1);
                sb.AppendLine($"    {typeArgs[i]}{(isLast ? "" : ",")}");
            }

            sb.Append(">?");
            return sb.ToString();
        }
    }
}