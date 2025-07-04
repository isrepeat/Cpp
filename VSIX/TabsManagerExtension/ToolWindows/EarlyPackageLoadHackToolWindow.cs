//#define OLD_LOGIC
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

namespace TabsManagerExtension.ToolWindows {
    /// <summary>
    /// Вспомогательное скрытое окно, используемое как хак для принудительной инициализации VSIX-пакета при запуске Visual Studio.
    ///
    /// <para>
    /// <b>Назначение:</b><br/>
    /// Visual Studio по умолчанию откладывает инициализацию AsyncPackage, пока пользователь явно не откроет связанный ToolWindow.
    /// Чтобы обойти это поведение, данный класс используется как техническое окно, которое автоматически открывается при старте IDE
    /// (через атрибуты <c>[ProvideToolWindow]</c> и <c>[ProvideAutoLoad]</c>) и сразу же скрывается.
    /// </para>
    ///
    /// <para>
    /// <b>Поведение:</b><br/>
    /// После первого открытия Visual Studio закеширует это окно в .suo-файле и будет открывать его автоматически при последующих запусках.
    /// Это позволяет гарантированно инициализировать VSIX-пакет ещё до загрузки решения.
    /// Через событие <c>Loaded</c> выполняется внедрение пользовательского контрола в визуальное дерево IDE (см. <c>VsixVisualTreeHelper</c>).
    /// </para>
    ///
    /// <para>
    /// <b>Внимание:</b><br/>
    /// Контент окна заменён на заглушку (<c>TextBlock</c>), а само окно принудительно скрывается после инициализации.
    /// Реализация использует Win32 API для управления положением и видимостью окна, избегая мерцания и побочных эффектов.
    /// </para>
    /// </summary>
    [Guid("D8D6ACF4-93A3-4B90-9633-079C00E5F97E")]
    public class EarlyPackageLoadHackToolWindow : ToolWindowPane {
        private static EarlyPackageLoadHackToolWindow? _instance;
        public static EarlyPackageLoadHackToolWindow? Instance => _instance;

        private static AsyncPackage? _package;
        private bool _suppressAutoHide = false;

        public static void Initialize(AsyncPackage package) {
            _package = package ?? throw new ArgumentNullException(nameof(package));

#if OLD_LOGIC
            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                var dte = await _package.GetServiceAsync(typeof(EnvDTE.DTE)) as EnvDTE80.DTE2;
                Assumes.Present(dte);

                dte.Events.SolutionEvents.BeforeClosing += () => {
                    Instance._suppressAutoHide = true;

                    ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                        if (Instance.Frame is IVsWindowFrame frame) {
                            frame.Show();

                            var toolWindowWin32Controller = WindowWin32Controller.TryCreateFromToolWindow(Instance);
                            if (toolWindowWin32Controller != null) {
                                toolWindowWin32Controller.Hide();

                                var ideRectInfo = await Instance.GetCenteredIdeRectInfoAsync();
                                toolWindowWin32Controller.SetPositionWithoutShow(
                                    ideRectInfo.X,
                                    ideRectInfo.Y,
                                    ideRectInfo.Width,
                                    ideRectInfo.Height
                                    );
                            }
                        }
                    });
                };
            });
#endif

            _ = Task.Run(async () => {
                var window = await _package.FindToolWindowAsync(
                    typeof(EarlyPackageLoadHackToolWindow),
                    0,
                    create: true,
                    CancellationToken.None);
#if !OLD_LOGIC
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                // Call after FindToolWindowAsync to avoid deadlock.
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var vsShell = await _package.GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell;
                Assumes.Present(vsShell);

                if (Instance.Frame is IVsWindowFrame frame) {
                    bool isVisible = frame.IsVisible() == VSConstants.S_OK; // frame.IsVisible() return HRESULT.
                    if (!isVisible) {
                        // Show tool window once to make VS cache it (in .suo).
                        frame.Show();

                        var ideRectInfo = await Instance.GetRightCornerIdeRectInfoAsync();

                        var toolWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromToolWindow(Instance);
                        if (toolWindowWin32Controller != null) {
                            toolWindowWin32Controller.SetPositionWithoutShow(
                                ideRectInfo.X,
                                ideRectInfo.Y,
                                ideRectInfo.Width,
                                ideRectInfo.Height
                                );
                            toolWindowWin32Controller.Hide();
                        }
                    }
                }
#endif
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
#if OLD_LOGIC
            if (_suppressAutoHide) {
                return;
            }
            _suppressAutoHide = false;

            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                if (this.Frame is IVsWindowFrame frame) {
                    frame.Hide();
                }
            });
#else
            if (sender is DependencyObject d) {
                d.Dispatcher.BeginInvoke(new Action(() => {
                    // Hide as soon as possible.
                    var _toolWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromToolWindow(this);
                    if (_toolWindowWin32Controller != null) {
                        _toolWindowWin32Controller.Hide();
                    }

                    // Update window position to be placed in corner of IDE.
                    ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                        await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                        var ideRectInfo = await this.GetRightCornerIdeRectInfoAsync();

                        var toolWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromToolWindow(this);
                        if (toolWindowWin32Controller != null) {
                            toolWindowWin32Controller.SetPositionWithoutShow(
                                ideRectInfo.X,
                                ideRectInfo.Y,
                                ideRectInfo.Width,
                                ideRectInfo.Height
                                );
                        }
                    });
                }), DispatcherPriority.Loaded);
            }
#endif
        }


        public struct RectInfo {
            public int X;
            public int Y;
            public int Width;
            public int Height;
        }
        public async Task<RectInfo> GetCenteredIdeRectInfoAsync() {
            var vsShell = await _package.GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell;
            Assumes.Present(vsShell);

            var vsShellWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromVsShell(vsShell);
            if (vsShellWindowWin32Controller.GetRect(out var ideRect)) {
                int ideWidth = ideRect.Right - ideRect.Left;
                int ideHeight = ideRect.Bottom - ideRect.Top;

                int width = 600;
                int height = 300;

                int x = ideRect.Left + (ideWidth - width) / 2;
                int y = ideRect.Top + (ideHeight - height) / 2;

                return new RectInfo {
                    X = x,
                    Y = y,
                    Width = width,
                    Height = height
                };
            }
            return new RectInfo { };
        }

        public async Task<RectInfo> GetRightCornerIdeRectInfoAsync() {
            var vsShell = await _package.GetServiceAsync(typeof(SVsUIShell)) as IVsUIShell;
            Assumes.Present(vsShell);

            var vsShellWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromVsShell(vsShell);
            if (vsShellWindowWin32Controller.GetRect(out var ideRect)) {
                float dpiScale = Helpers.Win32.WindowWin32Controller.GetSystemDpiScale();

                int widthLogical = 150;
                int heightLogical = 50;

                int width = (int)(widthLogical * dpiScale);
                int height = (int)(heightLogical * dpiScale);

                int x = ideRect.Right - width - (int)(130 * dpiScale); // отступ справа
                int y = ideRect.Top;

                return new RectInfo {
                    X = x,
                    Y = y,
                    Width = width,
                    Height = height
                };
            }
            return new RectInfo { };
        }

        public void TEST_MoveToSmth() {
            ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                if (this.Frame is IVsWindowFrame frame) {
                    _suppressAutoHide = true;
                    frame.Show();
                }

                var toolWindowWin32Controller = Helpers.Win32.WindowWin32Controller.TryCreateFromToolWindow(this);
                if (toolWindowWin32Controller != null) {
                    //toolWindowWin32Controller.Hide();
                    toolWindowWin32Controller.Show();

                    //var ideRectInfo = await this.GetRightCornerIdeRectInfoAsync();
                    //toolWindowWin32Controller.SetPositionWithoutShow(
                    //            ideRectInfo.X,
                    //            ideRectInfo.Y,
                    //            ideRectInfo.Width,
                    //            ideRectInfo.Height
                    //            );
                }
            });
        }
    }
}