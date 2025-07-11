using System;
using System.IO;
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
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.VCProjectEngine;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.VsShell.Solution {
    /// <summary>
    /// ExternalDependenciesMonitor
    /// </summary>
    public sealed class ExternalDependenciesMonitor {
        private readonly IVsHierarchy _projectHierarchy;
        private readonly uint _externalDependenciesItemId;

        private readonly HashSet<string> _currentIncludes = new(StringComparer.OrdinalIgnoreCase);

        public event Action<string>? ExternalIncludeAdded;
        public event Action<string>? ExternalIncludeRemoved;

        public ExternalDependenciesMonitor(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this._projectHierarchy = projectHierarchy;

            this._externalDependenciesItemId = this.FindExternalDependenciesItemId();
        }

        private uint FindExternalDependenciesItemId() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var childId in Utils.VsHierarchyUtils.Walker.GetChildren(this._projectHierarchy, VSConstants.VSITEMID_ROOT)) {
                this._projectHierarchy.GetProperty(childId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string;

                if (this.IsGuidName(name)) {
                    return childId;
                }
            }

            return VSConstants.VSITEMID_NIL;
        }

        private bool IsGuidName(string? name) {
            if (string.IsNullOrWhiteSpace(name)) {
                return false;
            }

            return Guid.TryParse(name, out _);
        }

        public void Refresh() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (this._externalDependenciesItemId == VSConstants.VSITEMID_NIL) {
                return;
            }

            var newIncludes = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var includeId in Utils.VsHierarchyUtils.Walker.GetChildren(this._projectHierarchy, this._externalDependenciesItemId)) {
                this._projectHierarchy.GetCanonicalName(includeId, out var canonicalName);

                if (!string.IsNullOrEmpty(canonicalName)) {
                    newIncludes.Add(canonicalName);

                    if (!this._currentIncludes.Contains(canonicalName)) {
                        this.ExternalIncludeAdded?.Invoke(canonicalName);
                    }
                }
            }

            // Определяем удалённые
            foreach (var oldInclude in this._currentIncludes) {
                if (!newIncludes.Contains(oldInclude)) {
                    this.ExternalIncludeRemoved?.Invoke(oldInclude);
                }
            }

            this._currentIncludes.Clear();
            this._currentIncludes.UnionWith(newIncludes);
        }
    }


    //private void StartAnalyzeRoutineInBackground() {
    //        ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
    //            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

    //            int attempts = 5;
    //            do {
    //                await Task.Delay(TimeSpan.FromSeconds(3));
    //                this.Analyze();
    //            } while (--attempts > 0);
    //        });
    //    }
    //}
}