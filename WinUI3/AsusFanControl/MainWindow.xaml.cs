using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using GHelper;
using GHelper.Mode;

namespace AsusFanControl {
    public sealed partial class MainWindow : Window {
        // P/Invoke declarations for low-level file I/O
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern IntPtr CreateFile(string lpFileName, uint dwDesiredAccess,
            uint dwShareMode, IntPtr lpSecurityAttributes, uint dwCreationDisposition,
            uint dwFlagsAndAttributes, IntPtr hTemplateFile);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool DeviceIoControl(IntPtr hDevice, uint dwIoControlCode,
            byte[] lpInBuffer, uint nInBufferSize, byte[] lpOutBuffer, uint nOutBufferSize,
            ref uint lpBytesReturned, IntPtr lpOverlapped);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        // Constants (from G-Helper analysis)
        private const string ATKACPI_PATH = @"\\.\ATKACPI";
        private const uint GENERIC_READ = 0x80000000;
        private const uint GENERIC_WRITE = 0x40000000;
        private const uint FILE_SHARE_READ = 0x1;
        private const uint FILE_SHARE_WRITE = 0x2;
        private const uint OPEN_EXISTING = 3;
        private const uint FILE_ATTRIBUTE_NORMAL = 0x80;
        private const uint IOCTL_ATKACPI = 0x0022240C;         // ASUS ACPI control code
        private const uint METHOD_DEVS = 0x53564544;         // 'DEVS' in ASCII (Set Device)
        private const uint DEVICE_PERF_MODE = 0x00120075;      // PerformanceMode device ID
                                                               // Mode indices as per ASUS/Armoury Crate: 0=Balanced, 1=Turbo(Performance), 2=Silent
        private const int MODE_BALANCED = 0;
        private const int MODE_PERFORMANCE = 1;
        private const int MODE_SILENT = 2;

        //private static AsusACPI acpi;
        //public static ModeControl modeControl = new ModeControl();

        public MainWindow() {
            this.InitializeComponent();

            AsusDevice.modeControl.RunPerformanceModeCycle();

            // Assume there are 3 Buttons in the XAML named BtnSilent, BtnBalanced, BtnPerformance
            BtnSilent.Click += (_, __) => SetPerformanceMode(MODE_SILENT);
            BtnBalanced.Click += (_, __) => SetPerformanceMode(MODE_BALANCED);
            BtnPerformance.Click += (_, __) => SetPerformanceMode(MODE_PERFORMANCE);
        }

        private void SetPerformanceMode(int modeIndex) {
            //// 1. Open handle to ACPI device
            //IntPtr handle = CreateFile(ATKACPI_PATH, GENERIC_READ | GENERIC_WRITE,
            //                           FILE_SHARE_READ | FILE_SHARE_WRITE,
            //                           IntPtr.Zero, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, IntPtr.Zero);
            //if (handle == new IntPtr(-1))  // INVALID_HANDLE_VALUE
            //{
            //    // Handle error (requires running as admin, or device not found)
            //    return;
            //}

            //try {
            //    // 2. Build the ACPI method call buffer for DEVS (Device Set)
            //    byte[] args = new byte[8];
            //    // args[0..3] = PerformanceMode device ID, args[4..7] = desired mode value
            //    Buffer.BlockCopy(BitConverter.GetBytes(DEVICE_PERF_MODE), 0, args, 0, 4);
            //    Buffer.BlockCopy(BitConverter.GetBytes(modeIndex), 0, args, 4, 4);

            //    // Compose full input buffer: [ MethodID (DEVS) | InputSize | args... ]
            //    byte[] inputBuffer = new byte[8 + args.Length];
            //    Buffer.BlockCopy(BitConverter.GetBytes(METHOD_DEVS), 0, inputBuffer, 0, 4);
            //    Buffer.BlockCopy(BitConverter.GetBytes(args.Length), 0, inputBuffer, 4, 4);
            //    Buffer.BlockCopy(args, 0, inputBuffer, 8, args.Length);

            //    // 3. Send IOCTL to ACPI driver
            //    byte[] outputBuffer = new byte[16];  // output buffer (16 bytes as in G-Helper)
            //    uint bytesReturned = 0;
            //    bool success = DeviceIoControl(handle, IOCTL_ATKACPI,
            //                                   inputBuffer, (uint)inputBuffer.Length,
            //                                   outputBuffer, (uint)outputBuffer.Length,
            //                                   ref bytesReturned, IntPtr.Zero);
            //    // Optionally, check outputBuffer[0] for 1 = success

            //    if (!success) {
            //        int err = Marshal.GetLastWin32Error();
            //    }
            //}
            //finally {
            //    // 4. Close handle
            //    CloseHandle(handle);
            //}
        }
    }
}
