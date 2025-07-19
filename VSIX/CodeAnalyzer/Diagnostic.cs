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
    public enum ReporterIdentationMarker {
        None,
        Dot,
        LineNumber
    }

    public static class Reporter {
        private static int idCounterMsg = 0;
        private static int idCounterError = 0;

        public static void Msg(
            SourceProductionContext context,
            string message,
            ReporterIdentationMarker reporterIdentationMarker = ReporterIdentationMarker.None
            ) {
            var lines = message.Split(new[] { "\r\n", "\n" }, StringSplitOptions.None);
            var lineCounter = 0;

            foreach (var line in lines) {
                var descriptor = new DiagnosticDescriptor(
                    id: $"MSG{idCounterMsg++.ToString("D3")}",
                    title: "",
                    messageFormat: "{0}",
                    category: "SourceGenerator",
                    DiagnosticSeverity.Warning,
                    isEnabledByDefault: true);

                string identationMarker = "";
                switch (reporterIdentationMarker) {
                    case ReporterIdentationMarker.None:
                        break;

                    case ReporterIdentationMarker.Dot:
                        identationMarker = $"· ";
                        break;

                    case ReporterIdentationMarker.LineNumber:
                        identationMarker = $"{lineCounter++.ToString().PadLeft(3, '_')} ";
                        break;
                }

                context.ReportDiagnostic(Diagnostic.Create(
                    descriptor,
                    Location.None,
                    $"{identationMarker}{line}"));
            }
        }


        public static void Error(SourceProductionContext context, string msg) {
            string id = $"ERROR{idCounterError++.ToString("D3")}";

            var descriptor = new DiagnosticDescriptor(
                id: id,
                title: "",
                messageFormat: "{0}",
                category: "SourceGenerator",
                DiagnosticSeverity.Error,
                isEnabledByDefault: true);

            context.ReportDiagnostic(Diagnostic.Create(descriptor, Location.None, msg));
        }
    }
}