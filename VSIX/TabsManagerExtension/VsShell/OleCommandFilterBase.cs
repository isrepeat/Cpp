using System;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;


namespace TabsManagerExtension.VsShell {
    public abstract class OleCommandFilterBase : IOleCommandTarget {
        private IOleCommandTarget? _next;

        //
        // IOleCommandTarget
        //
        public int QueryStatus(ref Guid pguidCmdGroup, uint cCmds, OLECMD[] prgCmds, IntPtr pCmdText) {
            return _next?.QueryStatus(ref pguidCmdGroup, cCmds, prgCmds, pCmdText) ?? VSConstants.E_FAIL;
        }

        public int Exec(ref Guid pguidCmdGroup, uint nCmdID, uint nCmdexecopt, IntPtr pvaIn, IntPtr pvaOut) {
            if (this.TryHandleCommand(pguidCmdGroup, nCmdID)) {
                this.OnCommandIntercepted(pguidCmdGroup, nCmdID);
                return VSConstants.S_OK;
            }

            this.OnCommandPassedThrough(pguidCmdGroup, nCmdID);
            return _next?.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut) ?? VSConstants.E_FAIL;
        }

        //
        // Api
        //
        public void SetNext(IOleCommandTarget next) {
            _next = next;
        }

        protected abstract bool TryHandleCommand(Guid cmdGroup, uint cmdId);

        protected virtual void OnCommandIntercepted(Guid cmdGroup, uint cmdId) { }

        protected virtual void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) { }
    }
}