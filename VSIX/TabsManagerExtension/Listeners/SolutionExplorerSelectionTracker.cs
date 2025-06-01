using System;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;
using System.Collections.Generic;

namespace TabsManagerExtension {
    public interface ISolutionExplorerSelectionHandler {
        void OnProjectItemSelected(EnvDTE.ProjectItem projectItem);
    }

    public sealed class SolutionExplorerSelectionTracker : IVsSelectionEvents, IDisposable {
        private readonly ISolutionExplorerSelectionHandler _handler;
        private readonly IVsMonitorSelection _monitorSelection;
        private uint _cookie;

        public SolutionExplorerSelectionTracker(ISolutionExplorerSelectionHandler handler) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this._handler = handler ?? throw new ArgumentNullException(nameof(handler));
            this._monitorSelection = (IVsMonitorSelection)Package.GetGlobalService(typeof(SVsShellMonitorSelection))
                ?? throw new InvalidOperationException("Cannot get IVsMonitorSelection");

            int hr = this._monitorSelection.AdviseSelectionEvents(this, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);
        }

        public void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_cookie != 0) {
                int hr = _monitorSelection.UnadviseSelectionEvents(_cookie);
                ErrorHandler.Succeeded(hr);
                _cookie = 0;
            }
        }

        public int OnSelectionChanged(
            IVsHierarchy pHierOld, uint itemidOld,
            IVsMultiItemSelect pMISOld, ISelectionContainer pSCOld,
            IVsHierarchy pHierNew, uint itemidNew,
            IVsMultiItemSelect pMISNew, ISelectionContainer pSCNew) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (pHierNew == null || itemidNew == VSConstants.VSITEMID_NIL) {
                return VSConstants.S_OK;
            }

            try {
                pHierNew.GetProperty(itemidNew, (int)__VSHPROPID.VSHPROPID_ExtObject, out object value);

                if (value is EnvDTE.ProjectItem projectItem) {
                    _handler.OnProjectItemSelected(projectItem);
                }
            }
            catch (Exception ex) {
                System.Diagnostics.Debug.WriteLine($"SolutionExplorerSelectionTracker: {ex}");
            }

            return VSConstants.S_OK;
        }

        public int OnElementValueChanged(uint elementid, object oldValue, object newValue) => VSConstants.S_OK;
        public int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) => VSConstants.S_OK;
    }
}