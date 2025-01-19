#define __IS_STAGE
using Microsoft.UI.Xaml.Media.Imaging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Globalization;
using System.Threading;
using System.Runtime.CompilerServices;


namespace SimpleApp.WinUI3.Helpers {
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
        public void LogDebug(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
#if __IS_STAGE
            if (!String.IsNullOrEmpty(caller)) {
#if DEBUG
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} [{callerInfo.CallerName}]");
#else
                CppFeatures.Cx.Logger.LogDebug($"{logMessage} [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
#endif
            }
            else {
#if DEBUG
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage}");
#else
                CppFeatures.Cx.Logger.LogDebug($"{logMessage}", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
#endif
            }
#endif
            }

        public Releaser LogFunctionScope(
            string logMessage,
            string caller = "",
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0
            ) {
            var callerInfo = new CallerInfo(filePath, caller, memberName, lineNumber);
#if __IS_STAGE
            if (!String.IsNullOrEmpty(callerInfo.CallerName)) {
#if DEBUG
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} enter [{callerInfo.CallerName}]");
#else
                CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter [{callerInfo.CallerName}]", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
#endif
            }
            else {
#if DEBUG
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {logMessage} enter");
#else
                CppFeatures.Cx.Logger.LogDebug($"{logMessage} enter", callerInfo.FilePath, callerInfo.MemberName, callerInfo.LineNumber);
#endif
            }
#endif
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
#if DEBUG
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logMessage} exit [{callerInfo.CallerName}]");
#else
                    CppFeatures.Cx.Logger.LogDebug($"{this.logMessage} exit [{this.callerInfo.CallerName}]", this.callerInfo.FilePath, this.callerInfo.MemberName, this.callerInfo.LineNumber);
#endif
                }
                else {
#if DEBUG
                    System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:HH:mm:ss:fff}] {this.logMessage} exit");
#else
                    CppFeatures.Cx.Logger.LogDebug($"{this.logMessage} exit", this.callerInfo.FilePath, this.callerInfo.MemberName, this.callerInfo.LineNumber);
#endif
                }
            }
        }
    }
}