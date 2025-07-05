using System;
using System.Windows.Threading;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.ComponentModelHost;


namespace TabsManagerExtension._EventArgs {
    public sealed class DocumentNavigationEventArgs : EventArgs {
        public string? PreviousDocumentFullName { get; }
        public string CurrentDocumentFullName { get; }

        public DocumentNavigationEventArgs(string? previousDocumentFullName, string currentDocumentFullName) {
            this.PreviousDocumentFullName = previousDocumentFullName;
            this.CurrentDocumentFullName = currentDocumentFullName;
        }
    }
}


namespace TabsManagerExtension.VsShell.Document.Services {
    /// <summary>
    /// Отслеживает событие что конкретный документ (файл) был активирован (показан в окне редактора) 
    /// </summary>
    public sealed class VsDocumentActivationTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<VsDocumentActivationTrackerService>,
        TabsManagerExtension.Services.IExtensionService,
        IVsRunningDocTableEvents {

        /// <summary>
        /// Глобальное событие активации документа (по пути файла).
        /// </summary>
        public event Action<_EventArgs.DocumentNavigationEventArgs>? OnDocumentActivated;

        private uint _cookie;
        private string _lastActivatedDocumentFullname = string.Empty;

        public VsDocumentActivationTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            int hr = PackageServices.VsRunningDocumentTable.AdviseRunningDocTableEvents(this, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);

            Helpers.Diagnostic.Logger.LogDebug("[DocumentActivationTracker] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_cookie != 0) {
                PackageServices.VsRunningDocumentTable.UnadviseRunningDocTableEvents(_cookie);
                _cookie = 0;
            }

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[DocumentActivationTracker] Shutdown complete.");
        }

        //
        // IVsRunningDocTableEvents
        //
        public int OnBeforeDocumentWindowShow(uint docCookie, int fFirstShow, IVsWindowFrame pFrame) {
            ThreadHelper.ThrowIfNotOnUIThread();

            PackageServices.VsRunningDocumentTable.GetDocumentInfo(
                docCookie,
                out _,
                out _,
                out _,
                out string documentFullName,
                out _,
                out _,
                out _);

            if (!string.IsNullOrEmpty(documentFullName) &&
                !string.Equals(documentFullName, _lastActivatedDocumentFullname, StringComparison.OrdinalIgnoreCase)
                ) {
                this.OnDocumentActivated?.Invoke(new _EventArgs.DocumentNavigationEventArgs(_lastActivatedDocumentFullname, documentFullName));
                _lastActivatedDocumentFullname = documentFullName;
            }

            return VSConstants.S_OK;
        }

        public int OnAfterDocumentWindowHide(uint docCookie, IVsWindowFrame pFrame) {
            return VSConstants.S_OK;
        }

        public int OnAfterSave(uint docCookie) {
            return VSConstants.S_OK;
        }

        public int OnBeforeSave(uint docCookie) {
            return VSConstants.S_OK;
        }

        public int OnAfterAttributeChange(uint docCookie, uint grfAttribs) {
            return VSConstants.S_OK;
        }

        public int OnAfterFirstDocumentLock(uint docCookie, uint lockType, uint readLocks, uint editLocks) {
            return VSConstants.S_OK;
        }

        public int OnBeforeLastDocumentUnlock(uint docCookie, uint lockType, uint readLocks, uint editLocks) {
            return VSConstants.S_OK;
        }
    }
}