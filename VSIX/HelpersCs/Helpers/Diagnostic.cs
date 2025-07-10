using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Helpers {
    public class Diagnostic {
        private static readonly Diagnostic _instance = new Diagnostic();
        public static Diagnostic Instance => _instance;
        public static Logger Logger { get; set; } = new Logger();
    }


    public struct CallerInfo {
        public string FilePath { get; }
        public string CallerName { get; }
        public string MemberName { get; }
        public int LineNumber { get; }

        public CallerInfo(string filePath, string callerName, string memberName, int lineNumber) {
            FilePath = filePath;
            CallerName = callerName;
            MemberName = memberName;
            LineNumber = lineNumber;
        }
    }


    public class Logger {
        private const string LogParamPrefix = "  * ";
        private const string LogErrorPrefix = "ERROR: ";
        private const string LogWarningPrefix = "WARNING: ";

        private const int MemberNameWidth = 64;

        public void LogDebug(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0) {

            // Определяем имя класса через StackTrace
            var stackFrame = new System.Diagnostics.StackTrace(1, false).GetFrame(0);
            var method = stackFrame.GetMethod();
            var callerTypeName = method?.DeclaringType?.Name ?? "";

            // Заменяем memberName на полное имя <TypeName>.<MemberName>
            memberName = !string.IsNullOrEmpty(callerTypeName)
                ? $"{callerTypeName}.{memberName}"
                : memberName;

            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            string memberFormatted = $"{callerInfo.MemberName}()".PadRight(Logger.MemberNameWidth);

            string logLine;
            if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {logMessage} [{callerInfo.CallerName}]";
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {logMessage}";
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }

            System.Diagnostics.Debug.WriteLine(logLine);
        }


        public void LogWarning(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0) {

            // Определяем имя класса через StackTrace
            var stackFrame = new System.Diagnostics.StackTrace(1, false).GetFrame(0);
            var method = stackFrame.GetMethod();
            var callerTypeName = method?.DeclaringType?.Name ?? "";

            // Заменяем memberName на полное имя <TypeName>.<MemberName>
            memberName = !string.IsNullOrEmpty(callerTypeName)
                ? $"{callerTypeName}.{memberName}"
                : memberName;

            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            string memberFormatted = $"{callerInfo.MemberName}()".PadRight(Logger.MemberNameWidth);

            string logLine;
            if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {Logger.LogWarningPrefix}{logMessage} [{callerInfo.CallerName}]";
                //CppFeatures.Cx.Logger.LogDebug($"{logWarningPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {Logger.LogWarningPrefix}{logMessage}";
                //CppFeatures.Cx.Logger.LogDebug($"{logWarningPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }

            System.Diagnostics.Debug.WriteLine(logLine);
        }


        public void LogError(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0) {

            // Определяем имя класса через StackTrace
            var stackFrame = new System.Diagnostics.StackTrace(1, false).GetFrame(0);
            var method = stackFrame.GetMethod();
            var callerTypeName = method?.DeclaringType?.Name ?? "";

            // Заменяем memberName на полное имя <TypeName>.<MemberName>
            memberName = !string.IsNullOrEmpty(callerTypeName)
                ? $"{callerTypeName}.{memberName}"
                : memberName;

            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            string memberFormatted = $"{callerInfo.MemberName}()".PadRight(Logger.MemberNameWidth);

            string logLine;
            if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {Logger.LogErrorPrefix}{logMessage} [{callerInfo.CallerName}]";
                //CppFeatures.Cx.Logger.LogDebug($"{logErrorPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {Logger.LogErrorPrefix}{logMessage}";
                //CppFeatures.Cx.Logger.LogDebug($"{logErrorPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }

            System.Diagnostics.Debug.WriteLine(logLine);
        }



        public void LogParam(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0) {

            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            string memberFormatted = $"".PadRight(Logger.MemberNameWidth);

            string logLine;
            if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}  {Logger.LogParamPrefix}{logMessage} [{callerInfo.CallerName}]";
                //CppFeatures.Cx.Logger.LogDebug($"{logParamPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}  {Logger.LogParamPrefix}{logMessage}";
                //CppFeatures.Cx.Logger.LogDebug($"{logParamPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }

            System.Diagnostics.Debug.WriteLine(logLine);
        }


        public Releaser LogFunctionScope(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0) {

            // Определяем имя класса через StackTrace
            var stackFrame = new System.Diagnostics.StackTrace(1, false).GetFrame(0);
            var method = stackFrame.GetMethod();
            var callerTypeName = method?.DeclaringType?.Name ?? "";

            // Заменяем memberName на полное имя <TypeName>.<MemberName>
            memberName = !string.IsNullOrEmpty(callerTypeName)
                ? $"{callerTypeName}.{memberName}"
                : memberName;

            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            string memberFormatted = $"{callerInfo.MemberName}()".PadRight(Logger.MemberNameWidth);

            string logLine;
            if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {logMessage} enter [{callerInfo.CallerName}]";
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {logMessage} enter";
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }

            System.Diagnostics.Debug.WriteLine(logLine);

            return new Releaser(logMessage, callerInfo);
        }

        public class Releaser : IDisposable {
            public string logMessage = "";
            public CallerInfo callerInfo;

            public Releaser(string logMessage, CallerInfo callerInfo) {
                this.logMessage = logMessage;
                this.callerInfo = callerInfo;
            }

            public void Dispose() {
                string memberFormatted = $"{callerInfo.MemberName}()".PadRight(Logger.MemberNameWidth);

                string logLine;
                if (!string.IsNullOrEmpty(callerInfo.CallerName)) {
                    logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {this.logMessage} exit [{this.callerInfo.CallerName}]";
                    //CppFeatures.Cx.Logger.LogDebug($"{logMessage} exit [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
                }
                else {
                    logLine = $"[{DateTime.Now:HH:mm:ss:fff}] {memberFormatted}: {this.logMessage} exit";
                    //CppFeatures.Cx.Logger.LogDebug($"{logMessage} exit", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
                }

                System.Diagnostics.Debug.WriteLine(logLine);
            }
        }
    }

}