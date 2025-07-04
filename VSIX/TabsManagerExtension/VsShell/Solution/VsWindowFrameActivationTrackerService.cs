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
using TabsManagerExtension.VsShell.Services;

namespace TabsManagerExtension.VsShell.Solution.Services {
    public sealed class VsWindowFrameActivationTrackerService :
         VsShell.Services.VsSelectionEventsServiceBase<VsWindowFrameActivationTrackerService>,
         TabsManagerExtension.Services.IExtensionService {

        public event Action<IVsWindowFrame>? VsWindowFrameActivated;

        public VsWindowFrameActivationTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();
            Helpers.Diagnostic.Logger.LogDebug("[VsWindowFrameActivationTrackerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();
            
            base.Dispose();
            
            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsWindowFrameActivationTrackerService] Disposed.");
        }

        //
        // VsSelectionEventsServiceBase
        //
        public override int OnElementValueChanged(uint elementid, object oldValue, object newValue) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (elementid == (uint)VSConstants.VSSELELEMID.SEID_WindowFrame && newValue is IVsWindowFrame frame) {
                this.VsWindowFrameActivated?.Invoke(frame);
            }

            return VSConstants.S_OK;
        }
    }
}