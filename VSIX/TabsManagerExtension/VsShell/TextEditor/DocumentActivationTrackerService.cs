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


namespace TabsManagerExtension.VsShell.TextEditor.Services {
    /// <summary>
    /// Отслеживает активацию документов через Running Document Table.
    /// Может быть создан только из ExtensionServices (singleton-логика вручную).
    /// </summary>
    public sealed class DocumentActivationTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<DocumentActivationTrackerService>,
        TabsManagerExtension.Services.IExtensionService {

        /// <summary>
        /// Глобальное событие активации документа (по пути файла).
        /// </summary>
        public event Action<string>? OnDocumentActivated;

        private RdtEventHandler? _handler;
        private string _lastActivatedFilePath = string.Empty;
        private uint _cookie;

        public DocumentActivationTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _handler = new RdtEventHandler(this);
            int hr = PackageServices.VsRunningDocumentTable.AdviseRunningDocTableEvents(_handler, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);

            Helpers.Diagnostic.Logger.LogDebug("[DocumentActivationTracker] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_cookie != 0) {
                PackageServices.VsRunningDocumentTable.UnadviseRunningDocTableEvents(_cookie);
                _cookie = 0;
            }

            _handler = null;

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[DocumentActivationTracker] Shutdown complete.");
        }


        private class RdtEventHandler : IVsRunningDocTableEvents {
            private readonly DocumentActivationTrackerService _owner;

            public RdtEventHandler(DocumentActivationTrackerService owner) {
                _owner = owner;
            }

            public int OnBeforeDocumentWindowShow(uint docCookie, int fFirstShow, IVsWindowFrame pFrame) {
                ThreadHelper.ThrowIfNotOnUIThread();

                PackageServices.VsRunningDocumentTable.GetDocumentInfo(docCookie, out _, out _, out _, out string moniker, out _, out _, out _);

                if (!string.IsNullOrEmpty(moniker) &&
                    !string.Equals(moniker, _owner._lastActivatedFilePath, StringComparison.OrdinalIgnoreCase)) {
                    _owner._lastActivatedFilePath = moniker;
                    _owner.OnDocumentActivated?.Invoke(moniker);
                }

                return VSConstants.S_OK;
            }

            public int OnAfterDocumentWindowHide(uint docCookie, IVsWindowFrame pFrame) => VSConstants.S_OK;
            public int OnAfterSave(uint docCookie) => VSConstants.S_OK;
            public int OnBeforeSave(uint docCookie) => VSConstants.S_OK;
            public int OnAfterAttributeChange(uint docCookie, uint grfAttribs) => VSConstants.S_OK;
            public int OnAfterFirstDocumentLock(uint docCookie, uint lockType, uint readLocks, uint editLocks) => VSConstants.S_OK;
            public int OnBeforeLastDocumentUnlock(uint docCookie, uint lockType, uint readLocks, uint editLocks) => VSConstants.S_OK;
        }
    }
}