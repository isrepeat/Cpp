using System;
using System.Drawing;
using System.IO.Packaging;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Media3D;
using System.Windows.Threading;
using Microsoft;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;


namespace TabsManagerExtension.Win32 {
    public class WindowWin32Controller {
        private readonly IntPtr _hwnd;
        public IntPtr Hwnd => _hwnd;


        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        private const int SW_HIDE = 0;
        private const int SW_SHOW = 5;


        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool SetWindowPos(
            IntPtr hWnd, IntPtr hWndInsertAfter,
            int X, int Y, int cx, int cy, uint uFlags);

        private const uint SWP_NOZORDER = 0x0004;
        private const uint SWP_NOACTIVATE = 0x0010;
        private const uint SWP_SHOWWINDOW = 0x0040;


        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT {
            public int Left;
            public int Top;
            public int Right;
            public int Bottom;
        }

        public WindowWin32Controller(IntPtr hwnd) {
            _hwnd = hwnd;
        }

        public static WindowWin32Controller? TryCreateFromVsShell(IVsUIShell vsShell) {
            vsShell.GetDialogOwnerHwnd(out IntPtr hwnd);
            return new WindowWin32Controller(hwnd);
        }
        public static WindowWin32Controller? TryCreateFromToolWindow(ToolWindowPane window) {
            if (window?.Content is Visual visual &&
                PresentationSource.FromVisual(visual) is HwndSource source &&
                source.Handle != IntPtr.Zero) {
                return new WindowWin32Controller(source.Handle);
            }
            return null;
        }

        public static float GetSystemDpiScale() {
            using var g = Graphics.FromHwnd(IntPtr.Zero);
            return g.DpiX / 96f;
        }

        public void Show() {
            ShowWindow(_hwnd, SW_SHOW);
        }

        public void Hide() {
            ShowWindow(_hwnd, SW_HIDE);
        }

        public void SetPosition(int x, int y, int width, int height) {
            SetWindowPos(_hwnd, IntPtr.Zero, x, y, width, height,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }

        public void SetPositionWithoutShow(int x, int y, int width, int height) {
            SetWindowPos(_hwnd, IntPtr.Zero, x, y, width, height,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }

        public bool GetRect(out RECT rect) {
            return GetWindowRect(_hwnd, out rect);
        }
    }
}