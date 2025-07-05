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
    public static class VsCommandMapper {
        private static readonly Dictionary<VSConstants.VSStd2KCmdID, VSConstants.VSStd97CmdID> _2kTo97 = new();
        private static readonly Dictionary<VSConstants.VSStd97CmdID, VSConstants.VSStd2KCmdID> _97To2k = new();

        static VsCommandMapper() {
            VsCommandMapper.AddMapping(VSConstants.VSStd2KCmdID.DELETE, VSConstants.VSStd97CmdID.Delete);
        }

        public static IReadOnlyCollection<VSConstants.VSStd97CmdID> GetMappedStd97FromStd2kCommands(IEnumerable<VSConstants.VSStd2KCmdID> std2kCommands) {
            return std2kCommands
                .Select(std2kCmd => _2kTo97.TryGetValue(std2kCmd, out var std97Cmd) ? std97Cmd : (VSConstants.VSStd97CmdID?)null)
                .Where(std97Cmd => std97Cmd.HasValue)
                .Select(std97Cmd => std97Cmd.Value)
                .ToHashSet();
        }

        public static bool TryMapStd2kToStd97Command(VSConstants.VSStd2KCmdID cmd2k, out VSConstants.VSStd97CmdID std97) {
            return _2kTo97.TryGetValue(cmd2k, out std97);
        }

        public static bool TryMapStd97ToStd2kCommand(VSConstants.VSStd97CmdID std97, out VSConstants.VSStd2KCmdID cmd2k) {
            return _97To2k.TryGetValue(std97, out cmd2k);
        }


        public static Key? TryMapStd2kToKey(VSConstants.VSStd2KCmdID cmd) {
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

        public static string ConnandToString(VSConstants.VSStd2KCmdID std2kCmd) {
            if (Enum.IsDefined(typeof(VSConstants.VSStd2KCmdID), std2kCmd)) {
                var name = ((VSConstants.VSStd2KCmdID)std2kCmd).ToString();
                return $"VSStd2K::{name}";
            }
            return $"VSStd2K::Unknown({std2kCmd})";
        }

        private static void AddMapping(VSConstants.VSStd2KCmdID std2kCmd, VSConstants.VSStd97CmdID std97Cmd) {
            _2kTo97[std2kCmd] = std97Cmd;
            _97To2k[std97Cmd] = std2kCmd;
        }
    }
}