using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.ComponentModel.Design;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.TextManager.Interop;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension {
    /// <summary>
    /// Централизованный сервис-локатор для получения COM-сервисов Visual Studio (IVsSolution, IVsUIShell, EnvDTE и др).
    /// Все свойства используют ленивую (lazy) инициализацию: сервис получается через Package.GetGlobalService только при первом доступе,
    /// и затем кешируется для повторных вызовов. При вызове Invalidate() кеш сбрасывается и сервисы будут получены заново при следующем доступе.
    /// </summary>
    public static class PackageServices {
        private static EnvDTE80.DTE2? _dte2;
        public static EnvDTE80.DTE2 Dte2 {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_dte2 == null) {
                    _dte2 = GetService<EnvDTE80.DTE2>(typeof(SDTE));
                }
                return _dte2;
            }
        }
        public static EnvDTE80.DTE2? TryGetDte2() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<EnvDTE80.DTE2>(typeof(SDTE));
        }


        private static IVsShell? _vsShell;
        public static IVsShell VsShell {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsShell == null) {
                    _vsShell = GetService<IVsShell>(typeof(SVsShell));
                }
                return _vsShell;
            }
        }
        public static IVsShell? TryGetVsShell() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsShell>(typeof(SVsShell));
        }


        private static IVsUIShell? _vsUIShell;
        public static IVsUIShell VsUIShell {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsUIShell == null) {
                    _vsUIShell = GetService<IVsUIShell>(typeof(SVsUIShell));
                }
                return _vsUIShell;
            }
        }
        public static IVsUIShell? TryGetVsUIShell() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsUIShell>(typeof(SVsUIShell));
        }


        private static IVsUIShellOpenDocument? _vsUIShellOpenDocument;
        public static IVsUIShellOpenDocument VsUIShellOpenDocument {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsUIShellOpenDocument == null) {
                    _vsUIShellOpenDocument = GetService<IVsUIShellOpenDocument>(typeof(SVsUIShellOpenDocument));
                }
                return _vsUIShellOpenDocument;
            }
        }
        public static IVsUIShellOpenDocument? TryGetVsUIShellOpenDocument() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsUIShellOpenDocument>(typeof(SVsUIShellOpenDocument));
        }


        private static IVsSolution? _vsSolution;
        public static IVsSolution VsSolution {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsSolution == null) {
                    _vsSolution = GetService<IVsSolution>(typeof(SVsSolution));
                }
                return _vsSolution;
            }
        }
        public static IVsSolution? TryGetVsSolution() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsSolution>(typeof(SVsSolution));
        }


        private static IVsTextManager? _vsTextManager;
        public static IVsTextManager VsTextManager {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsTextManager == null) {
                    _vsTextManager = GetService<IVsTextManager>(typeof(SVsTextManager));
                }
                return _vsTextManager;
            }
        }
        public static IVsTextManager? TryGetVsTestManager() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsTextManager>(typeof(SVsTextManager));
        }


        private static IVsMonitorSelection? _vsMonitorSelection;
        public static IVsMonitorSelection VsMonitorSelection {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsMonitorSelection == null) {
                    _vsMonitorSelection = GetService<IVsMonitorSelection>(typeof(SVsShellMonitorSelection));
                }
                return _vsMonitorSelection;
            }
        }
        public static IVsMonitorSelection? TryGetVsMonitorSelection() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsMonitorSelection>(typeof(SVsShellMonitorSelection));
        }


        private static IVsRunningDocumentTable? _vsRunningDocumentTable;
        public static IVsRunningDocumentTable VsRunningDocumentTable {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsRunningDocumentTable == null) {
                    _vsRunningDocumentTable = GetService<IVsRunningDocumentTable>(typeof(SVsRunningDocumentTable));
                }
                return _vsRunningDocumentTable;
            }
        }
        public static IVsRunningDocumentTable? TryGetVsRunningDocumentTable() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsRunningDocumentTable>(typeof(SVsRunningDocumentTable));
        }


        private static IVsTrackProjectDocuments2? _vsTrackProjectDocuments2;
        public static IVsTrackProjectDocuments2 VsTrackProjectDocuments2 {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsTrackProjectDocuments2 == null) {
                    _vsTrackProjectDocuments2 = GetService<IVsTrackProjectDocuments2>(typeof(SVsTrackProjectDocuments));
                }
                return _vsTrackProjectDocuments2;
            }
        }
        public static IVsTrackProjectDocuments2? TryGetVsTrackProjectDocuments2() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsTrackProjectDocuments2>(typeof(SVsTrackProjectDocuments));
        }



        /// <summary>
        /// Сбрасывает все кешированные ссылки, при следующем доступе они будут получены заново через Package.GetGlobalService.
        /// Использовать например при событиях загрузки / выгрузки решения.
        /// </summary>
        public static void Invalidate() {
            _dte2 = null;
            _vsShell = null;
            _vsUIShell = null;
            _vsUIShellOpenDocument = null;
            _vsSolution = null;
            _vsTextManager = null;
            _vsMonitorSelection = null;
            _vsRunningDocumentTable = null;
            _vsTrackProjectDocuments2 = null;
        }


        private static T GetService<T>(Type type) where T : class {
            ThreadHelper.ThrowIfNotOnUIThread();

            var service = Package.GetGlobalService(type) as T;
            if (service == null) {
                throw new InvalidOperationException($"Cannot get service {type.FullName}");
            }
            return service;
        }

        private static T? TryGetService<T>(Type type) where T : class {
            ThreadHelper.ThrowIfNotOnUIThread();
            return Package.GetGlobalService(type) as T;
        }
    }
}