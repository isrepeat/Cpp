using System;
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


namespace TabsManagerExtension {
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

        private WindowWin32Controller(IntPtr hwnd) {
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




    [Guid("D8D6ACF4-93A3-4B90-9633-079C00E5F97E")]
    public class EarlyPackageLoadHackToolWindow : ToolWindowPane {
        private static EarlyPackageLoadHackToolWindow? _instance;
        public static EarlyPackageLoadHackToolWindow? Instance => _instance;

        public static event Action OnLoaded;
        private static AsyncPackage? _package;

        private bool _suppressAutoHide = false;

        public static void Initialize(AsyncPackage package) {
            _package = package ?? throw new ArgumentNullException(nameof(package));

            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                _ = Task.Run(async () => {
                    OnLoaded?.Invoke();

                    var window = await _package.FindToolWindowAsync(
                        typeof(EarlyPackageLoadHackToolWindow),
                        0,
                        create: true,
                        CancellationToken.None);
                });

                var dte = await _package.GetServiceAsync(typeof(EnvDTE.DTE)) as EnvDTE80.DTE2;
                Assumes.Present(dte);

                dte.Events.SolutionEvents.BeforeClosing += () => {
                    Instance._suppressAutoHide = true;

                    ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                        if (Instance.Frame is IVsWindowFrame frame) {
                            frame.Show();
                            var vsShell = await _package.GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell;
                            Assumes.Present(vsShell);

                            var vsShellWindowWin32Controller = WindowWin32Controller.TryCreateFromVsShell(vsShell);
                            if (vsShellWindowWin32Controller.GetRect(out var ideRect)) {
                                int ideWidth = ideRect.Right - ideRect.Left;
                                int ideHeight = ideRect.Bottom - ideRect.Top;

                                int width = 400;
                                int height = 300;

                                int x = ideRect.Left + (ideWidth - width) / 2;
                                int y = ideRect.Top + (ideHeight - height) / 2;

                                var _toolWindowWin32Controller = WindowWin32Controller.TryCreateFromToolWindow(Instance);
                                if (_toolWindowWin32Controller != null) {
                                    _toolWindowWin32Controller.SetPosition(x, y, width, height);
                                    _toolWindowWin32Controller.Hide();
                                }
                            }
                        }
                    });
                };
            });
        }

        public EarlyPackageLoadHackToolWindow() : base(null) {
            _instance = this;

            this.Caption = "EarlyPackageLoadHackToolWindow";

            var textBlock = new TextBlock {
                Text = "Trigger for early AsyncPackage load",
                Margin = new Thickness(10),
            };

            textBlock.Loaded += OnLoadedRootContent;
            this.Content = textBlock;
        }

        private void OnLoadedRootContent(object sender, RoutedEventArgs e) {
            if (_suppressAutoHide) {
                return;
            }
            _suppressAutoHide = false;

            if (sender is DependencyObject d) {
                d.Dispatcher.BeginInvoke(new Action(() => {
                    var _toolWindowWin32Controller = WindowWin32Controller.TryCreateFromToolWindow(this);
                    if (_toolWindowWin32Controller != null) {
                        _toolWindowWin32Controller.Hide();
                    }
                }), DispatcherPriority.Loaded); // или ContextIdle
            }

            //ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
            //    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

            //    if (this.Frame is IVsWindowFrame frame) {
            //        frame.Hide();
            //    }
            //});
        }

        public void TEST_MoveToSmth() {
            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                if (this.Frame is IVsWindowFrame frame) {
                    _suppressAutoHide = true;
                    frame.Show();

                    var vsShell = await _package.GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell;
                    Assumes.Present(vsShell);

                    var vsShellWindowWin32Controller = WindowWin32Controller.TryCreateFromVsShell(vsShell);

                    if (vsShellWindowWin32Controller.GetRect(out var ideRect)) {
                        int ideWidth = ideRect.Right - ideRect.Left;
                        int ideHeight = ideRect.Bottom - ideRect.Top;

                        int width = 400;
                        int height = 300;

                        int x = ideRect.Left + (ideWidth - width) / 2;
                        int y = ideRect.Top + (ideHeight - height) / 2;

                        var _toolWindowWin32Controller = WindowWin32Controller.TryCreateFromToolWindow(this);
                        if (_toolWindowWin32Controller != null) {
                            _toolWindowWin32Controller.SetPosition(x, y, width, height);
                        }
                    }
                }
            });
        }
    }
}