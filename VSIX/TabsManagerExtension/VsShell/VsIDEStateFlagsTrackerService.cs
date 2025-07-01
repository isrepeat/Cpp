using System;
using System.IO;
using System.Windows;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Utilities;
using System.Windows.Threading;
using TabsManagerExtension;

namespace TabsManagerExtension.VsShell.Services {
    public sealed class VsIDEStateFlagsTrackerService :
        VsShell.Services.VsSelectionEventsServiceBase<VsIDEStateFlagsTrackerService>,
        TabsManagerExtension.Services.IExtensionService {
        
        public static readonly Guid SolutionExistsGuid = new Guid(UIContextGuids80.SolutionExists);

        public Helpers.Events.TriggeredAction<Guid, bool> IDEStateFlagsChanged = new();
        public Helpers.Events.TriggeredAction SolutionLoaded = new();
        public Helpers.Events.TriggeredAction SolutionClosed = new();

        private IVsMonitorSelection _monitorSelection;
        private readonly Dictionary<Guid, bool> _contextStateMap = new();
        private readonly Dictionary<uint, Guid> _contextCookiesMap = new();

        public VsIDEStateFlagsTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _monitorSelection = (IVsMonitorSelection)Package.GetGlobalService(typeof(SVsShellMonitorSelection))
                ?? throw new InvalidOperationException("Cannot get IVsMonitorSelection");

            IEnumerable<Guid> trackedContextGuids = new List<Guid> {
                SolutionExistsGuid,
            };

            foreach (var guid in trackedContextGuids) {
                var contextGuid = guid; // make copy for pass by ref

                int hr = _monitorSelection.GetCmdUIContextCookie(ref contextGuid, out uint cookie);
                ErrorHandler.ThrowOnFailure(hr);

                _contextCookiesMap[cookie] = contextGuid;

                // Проверяем состояние прямо сейчас для каждого контекста
                hr = _monitorSelection.IsCmdUIContextActive(cookie, out int pfActive);
                if (ErrorHandler.Succeeded(hr)) {
                    this.HandleContextState(contextGuid, pfActive != 0);
                }
            }

            Helpers.Diagnostic.Logger.LogDebug("[VsIDEStateFlagsTrackerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            base.Dispose();

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsIDEStateFlagsTrackerService] Disposed.");
        }

        //
        // VsSelectionEventsServiceBase
        //
        public override int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_contextCookiesMap.TryGetValue(dwCmdUICookie, out Guid guid)) {
                this.HandleContextState(guid, fActive != 0);
            }

            return VSConstants.S_OK;
        }


        //
        // Api
        //
        public bool IsContextActive(Guid contextGuid) {
            return _contextStateMap.TryGetValue(contextGuid, out bool isActive) && isActive;
        }

        //
        // Internal logic
        //
        private void HandleContextState(Guid contextGuid, bool isActive) {
            _contextStateMap[contextGuid] = isActive;

            if (contextGuid == SolutionExistsGuid) {
                if (isActive) {
                    this.SolutionLoaded.Invoke();
                }
                else {
                    this.SolutionClosed.Invoke();
                }
            }
            else {
                this.IDEStateFlagsChanged.Invoke(contextGuid, isActive);
            }
        }
    }
}