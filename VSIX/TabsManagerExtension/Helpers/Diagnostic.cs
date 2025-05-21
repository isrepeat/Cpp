using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Globalization;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System.Threading;
using System.Runtime.CompilerServices;


namespace TabsManagerExtension.Helpers {
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
        private string logParamPrefix = "    * ";
        private string logErrorPrefix = "ERROR: ";
        private string logWarningPrefix = "WARNING: ";

        public void LogDebug(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            if (!String.IsNullOrEmpty(caller)) {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} [{callerInfo.CallerName}]");
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage}");
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
        }

        public void LogWarning(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            if (!String.IsNullOrEmpty(caller)) {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logWarningPrefix}{logMessage} [{callerInfo.CallerName}]");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logWarningPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logWarningPrefix}{logMessage}");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logWarningPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
        }

        public void LogError(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            if (!String.IsNullOrEmpty(caller)) {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logErrorPrefix}{logMessage} [{callerInfo.CallerName}]");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logErrorPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logErrorPrefix}{logMessage}");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logErrorPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
        }

        public void LogParam(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            if (!String.IsNullOrEmpty(caller)) {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logParamPrefix}{logMessage} [{callerInfo.CallerName}]");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logParamPrefix}{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logParamPrefix}{logMessage}");
                //CppFeatures.Cx.Logger.LogDebug($"{this.logParamPrefix}{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
        }

        public Releaser LogFunctionScope(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
            if (!String.IsNullOrEmpty(callerInfo.CallerName)) {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} enter [{callerInfo.CallerName}]");
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
            else {
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} enter");
                //CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
            }
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
                if (!String.IsNullOrEmpty(this.callerInfo.CallerName)) {
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logMessage} exit [{callerInfo.CallerName}]");
                    //CppFeatures.Cx.Logger.LogDebug($"{this.logMessage} exit [{this.callerInfo.CallerName}]", this.callerInfo.FilePath, this.callerInfo.MemberName, this.callerInfo.LineNumber);
                }
                else {
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logMessage} exit");
                    //CppFeatures.Cx.Logger.LogDebug($"{this.logMessage} exit", this.callerInfo.FilePath, this.callerInfo.MemberName, this.callerInfo.LineNumber);
                }
            }
        }
    }
}