using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell;


namespace TabsManagerExtension.VsShell.Document {
    public class ShellWindow {
        public EnvDTE.Window Window { get; private set; }

        public ShellWindow(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Window = window;
        }

        public bool IsTabWindow() {
            return IsTabWindow(this.Window);
        }
        public static bool IsTabWindow(EnvDTE.Window window) {
            if (window == null) {
                return false;
            }

            // Во вкладках редактора окна имеют Linkable == false, tool windows — true.
            return !window.Linkable;
        }


        public string GetWindowId() {
            return GetWindowId(this.Window);
        }
        public static string GetWindowId(EnvDTE.Window window) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                // У каждого Tool Window — свой уникальный ObjectKind,
                // потому что они создаются как отдельные компоненты, зарегистрированные в системе Visual Studio.
                return window.ObjectKind;
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"GetWindowId(ObjectKind) failed: {ex.Message}");
                return string.Empty;
            }
        }
    }
}