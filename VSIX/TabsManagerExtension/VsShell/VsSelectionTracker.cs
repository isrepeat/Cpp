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
    /// <summary>
    /// Универсальный трекер, подписывающийся на события выбора и активации окон в Visual Studio.
    /// Позволяет отслеживать:
    /// - смену активного окна (frame)
    /// - выбор элементов в Solution Explorer
    /// </summary>
    public sealed class VsSelectionTracker : IVsSelectionEvents, IDisposable {
        private readonly IVsMonitorSelection _monitorSelection;
        private readonly IVsUIShell _uiShell;
        private uint _cookie;

        /// <summary>
        /// Событие вызывается при смене активного окна (IVsWindowFrame).
        /// </summary>
        public event Action<IVsWindowFrame>? VsWindowFrameActivated;

        /// <summary>
        /// Событие вызывается при выборе ProjectItem в Solution Explorer.
        /// </summary>
        public event Action<EnvDTE.ProjectItem>? ProjectItemSelected;

        public VsSelectionTracker() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _monitorSelection = (IVsMonitorSelection)Package.GetGlobalService(typeof(SVsShellMonitorSelection))
                ?? throw new InvalidOperationException("Cannot get IVsMonitorSelection");

            _uiShell = (IVsUIShell)Package.GetGlobalService(typeof(SVsUIShell))
                ?? throw new InvalidOperationException("Cannot get IVsUIShell");

            int hr = _monitorSelection.AdviseSelectionEvents(this, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);
        }

        public void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_cookie != 0) {
                _monitorSelection.UnadviseSelectionEvents(_cookie);
                _cookie = 0;
            }
        }

        public int OnElementValueChanged(uint elementid, object oldValue, object newValue) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (elementid == (uint)VSConstants.VSSELELEMID.SEID_WindowFrame && newValue is IVsWindowFrame frame) {
                this.VsWindowFrameActivated?.Invoke(frame);
            }

            return VSConstants.S_OK;
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
                    this.ProjectItemSelected?.Invoke(projectItem);
                }
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"[VsSelectionTracker] OnSelectionChanged error: {ex}");
            }

            return VSConstants.S_OK;
        }

        public int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) => VSConstants.S_OK;
    }
}