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

        public void SetNext(IOleCommandTarget next) {
            _next = next;
        }

        public int QueryStatus(ref Guid pguidCmdGroup, uint cCmds, OLECMD[] prgCmds, IntPtr pCmdText) {
            return _next?.QueryStatus(ref pguidCmdGroup, cCmds, prgCmds, pCmdText) ?? VSConstants.E_FAIL;
        }

        public int Exec(ref Guid pguidCmdGroup, uint nCmdID, uint nCmdexecopt, IntPtr pvaIn, IntPtr pvaOut) {
            if (TryHandleCommand(pguidCmdGroup, nCmdID)) {
                this.OnCommandIntercepted(pguidCmdGroup, nCmdID);
                return VSConstants.S_OK;
            }

            this.OnCommandPassedThrough(pguidCmdGroup, nCmdID);
            return _next?.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut) ?? VSConstants.E_FAIL;
        }

        protected abstract bool TryHandleCommand(Guid cmdGroup, uint cmdId);

        protected virtual void OnCommandIntercepted(Guid cmdGroup, uint cmdId) { }

        protected virtual void OnCommandPassedThrough(Guid cmdGroup, uint cmdId) { }
    }




    public static class VsCommandMapper {
        private static readonly Dictionary<VSConstants.VSStd2KCmdID, VSConstants.VSStd97CmdID> _2kTo97 = new();
        private static readonly Dictionary<VSConstants.VSStd97CmdID, VSConstants.VSStd2KCmdID> _97To2k = new();

        static VsCommandMapper() {
            AddMapping(VSConstants.VSStd2KCmdID.DELETE, VSConstants.VSStd97CmdID.Delete);
        }

        public static string FormatCommand(VSConstants.VSStd2KCmdID cmd) {
            if (Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), cmd)) {
                var name = ((VSConstants.VSStd2KCmdID)cmd).ToString();
                return $"VSStd2K::{name}";
            }
            return $"VSStd2K::Unknown({cmd})";
        }

        public static IReadOnlyCollection<VSConstants.VSStd97CmdID> GetMappedStd97FromStd2kCommands(IEnumerable<VSConstants.VSStd2KCmdID> std2kCommands) {
            return std2kCommands
                .Select(std2kCmd => _2kTo97.TryGetValue(std2kCmd, out var std97Cmd) ? std97Cmd : (VSConstants.VSStd97CmdID?)null)
                .Where(std97Cmd => std97Cmd.HasValue)
                .Select(std97Cmd => std97Cmd.Value)
                .ToHashSet();
        }

        //public static Key? TryResolveKeyFromStd2kCommands(Guid cmdGroup, uint cmdId, IEnumerable<VSConstants.VSStd2KCmdID> std2kCommands) {
        //    if (cmdGroup == VSConstants.VSStd2K &&
        //        Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), (int)cmdId)) {

        //        var std2kCmd = (VSConstants.VSStd2KCmdID)cmdId;
        //        return TryMapToKey(std2kCmd);
        //    }

        //    if (cmdGroup == VSConstants.GUID_VSStandardCommandSet97 &&
        //        Enum.IsDefined(typeof(VSConstants.VSStd97CmdID), (int)cmdId)) {

        //        var std97Cmd = (VSConstants.VSStd97CmdID)cmdId;
        //        var mappedSet = GetMappedStd97FromStd2kCommands(std2kCommands);
        //        if (mappedSet.Contains(std97Cmd) && TryMapStd97ToStd2kCommand(std97Cmd, out var std2kCmd)) {
        //            return TryMapToKey(std2kCmd);
        //        }
        //    }

        //    return null;
        //}



        public static bool TryMapStd2kToStd97Command(VSConstants.VSStd2KCmdID cmd2k, out VSConstants.VSStd97CmdID std97) {
            return _2kTo97.TryGetValue(cmd2k, out std97);
        }

        public static bool TryMapStd97ToStd2kCommand(VSConstants.VSStd97CmdID std97, out VSConstants.VSStd2KCmdID cmd2k) {
            return _97To2k.TryGetValue(std97, out cmd2k);
        }


        public static Key? TryMapToKey(VSConstants.VSStd2KCmdID cmd) {
            return cmd switch {
                VSConstants.VSStd2KCmdID.TAB => Key.Tab,
                VSConstants.VSStd2KCmdID.UP => Key.Up,
                VSConstants.VSStd2KCmdID.DOWN => Key.Down,
                VSConstants.VSStd2KCmdID.LEFT => Key.Left,
                VSConstants.VSStd2KCmdID.RIGHT => Key.Right,
                VSConstants.VSStd2KCmdID.RETURN => Key.Enter,
                VSConstants.VSStd2KCmdID.DELETE => Key.Delete,
                VSConstants.VSStd2KCmdID.BACKSPACE => Key.Back,
                _ => null
            };
        }

        private static void AddMapping(VSConstants.VSStd2KCmdID cmd2k, VSConstants.VSStd97CmdID cmd97) {
            _2kTo97[cmd2k] = cmd97;
            _97To2k[cmd97] = cmd2k;
        }
    }
}