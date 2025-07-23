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
using System.Data.SqlTypes;


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
        public string Apply(string typeName) {
            int genericStart = typeName.IndexOf('<');
            int genericEnd = typeName.LastIndexOf('>');

            // Не generic — возвращаем как есть.
            if (genericStart == -1 || genericEnd == -1 || genericEnd <= genericStart) {
                return typeName;
            }

            // Отрезаем суффикс (всё после '>')
            string typeSuffix = typeName.Substring(genericEnd + 1); // суффикс: ?, [], * и т.п.
            string typeWithoutSuffix = typeName.Substring(0, genericEnd + 1);

            string baseType = typeWithoutSuffix.Substring(0, genericStart);
            string genericContent = typeWithoutSuffix.Substring(genericStart + 1, genericEnd - genericStart - 1);
            string[] typeArgs = genericContent.Split(',').Select(t => t.Trim()).ToArray();

            // Если один аргумент — оставляем однострочно.
            if (typeArgs.Length <= 2) {
                return typeName;
            }

            var sb = new StringBuilder();
            sb.AppendLine($"{baseType}<");

            for (int i = 0; i < typeArgs.Length; i++) {
                bool isLast = i == typeArgs.Length - 1;
                sb.AppendLine($"    {typeArgs[i]}{(isLast ? "" : ",")}");
            }

            sb.Append('>');
            sb.Append(typeSuffix); // <- возвращаем суффикс

            return sb.ToString();
        }
    }
}