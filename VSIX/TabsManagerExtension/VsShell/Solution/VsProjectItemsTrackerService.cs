using System;
using System.IO;
using System.Windows;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Utilities;

namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// Сервис отслеживания добавления и удаления файлов в проектах.
    /// Использует низкоуровневый интерфейс IVsTrackProjectDocumentsEvents2.
    /// </summary>
    public sealed class VsProjectItemsTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<VsProjectItemsTrackerService>,
        TabsManagerExtension.Services.IExtensionService,
        IVsTrackProjectDocumentsEvents2 {

        public event Action<string, EnvDTE.Project>? ProjectFileAdded;
        public event Action<string, EnvDTE.Project>? ProjectFileRemoved;

        public static readonly HashSet<string> TrackedFileExtensions = new(StringComparer.OrdinalIgnoreCase) {
            ".h",
            ".hpp",
            ".cpp",
        };

        private IVsTrackProjectDocuments2? _tracker;
        private uint _cookie;

        public VsProjectItemsTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _tracker = PackageServices.VsTrackProjectDocuments2;
            if (_tracker == null) {
                throw new InvalidOperationException("Cannot get SVsTrackProjectDocuments");
            }

            ErrorHandler.ThrowOnFailure(_tracker.AdviseTrackProjectDocumentsEvents(this, out _cookie));
            Helpers.Diagnostic.Logger.LogDebug("[VsProjectFileTrackerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_tracker != null && _cookie != 0) {
                _tracker.UnadviseTrackProjectDocumentsEvents(_cookie);
                _cookie = 0;
            }

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsProjectFileTrackerService] Disposed.");
        }

        //
        // IVsTrackProjectDocumentsEvents2
        //
        public int OnQueryAddFiles(IVsProject pProject, int cFiles, string[] rgpszMkDocuments,
            VSQUERYADDFILEFLAGS[] rgFlags, VSQUERYADDFILERESULTS[] pSummaryResult,
            VSQUERYADDFILERESULTS[] rgResults) {
            return VSConstants.S_OK;
        }
        public int OnQueryRemoveFiles(IVsProject pProject, int cFiles, string[] rgpszMkDocuments,
            VSQUERYREMOVEFILEFLAGS[] rgFlags, VSQUERYREMOVEFILERESULTS[] pSummaryResult,
            VSQUERYREMOVEFILERESULTS[] rgResults) {
            return VSConstants.S_OK;
        }
        
        public int OnQueryRenameFiles(IVsProject pProject, int cFiles,
            string[] rgszMkOldNames, string[] rgszMkNewNames,
            VSQUERYRENAMEFILEFLAGS[] rgFlags, VSQUERYRENAMEFILERESULTS[] pSummaryResult,
            VSQUERYRENAMEFILERESULTS[] rgResults) {
            return VSConstants.S_OK;
        }

        public int OnQueryAddDirectories(IVsProject pProject, int cDirectories,
            string[] rgpszMkDocuments, VSQUERYADDDIRECTORYFLAGS[] rgFlags,
            VSQUERYADDDIRECTORYRESULTS[] pSummaryResult, VSQUERYADDDIRECTORYRESULTS[] rgResults) {
            return VSConstants.S_OK;
        }

        public int OnQueryRemoveDirectories(IVsProject pProject, int cDirectories,
            string[] rgpszMkDocuments, VSQUERYREMOVEDIRECTORYFLAGS[] rgFlags,
            VSQUERYREMOVEDIRECTORYRESULTS[] pSummaryResult, VSQUERYREMOVEDIRECTORYRESULTS[] rgResults) {
            return VSConstants.S_OK;
        }

        public int OnQueryRenameDirectories(IVsProject pProject, int cDirs,
            string[] rgszMkOldNames, string[] rgszMkNewNames,
            VSQUERYRENAMEDIRECTORYFLAGS[] rgFlags, VSQUERYRENAMEDIRECTORYRESULTS[] pSummaryResult,
            VSQUERYRENAMEDIRECTORYRESULTS[] rgResults) {
            return VSConstants.S_OK;
        }

        public int OnAfterAddFilesEx(int cProjects, int cFiles, IVsProject[] rgpProjects, 
            int[] rgFirstIndices, string[] rgpszMkDocuments, VSADDFILEFLAGS[] rgFlags) {
            ThreadHelper.ThrowIfNotOnUIThread();

            for (int i = 0; i < rgpszMkDocuments.Length; i++) {
                string path = rgpszMkDocuments[i];
                if (!TrackedFileExtensions.Contains(Path.GetExtension(path))) {
                    continue;
                }

                int projIndex = this.ProjectIndexFor(i, rgFirstIndices);
                var vsProject = rgpProjects[projIndex];
                var dteProject = TryGetDteProjectFromIVsProject(vsProject);

                this.ProjectFileAdded?.Invoke(path, dteProject);
                Helpers.Diagnostic.Logger.LogDebug($"[VsProjectFileTrackerService] Added: {path} [{dteProject.UniqueName}]");
            }

            return VSConstants.S_OK;
        }

        public int OnAfterRemoveFiles(int cProjects, int cFiles, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgpszMkDocuments, VSREMOVEFILEFLAGS[] rgFlags) {
            ThreadHelper.ThrowIfNotOnUIThread();

            for (int i = 0; i < rgpszMkDocuments.Length; i++) {
                string path = rgpszMkDocuments[i];
                if (!TrackedFileExtensions.Contains(Path.GetExtension(path))) {
                    continue;
                }

                int projIndex = this.ProjectIndexFor(i, rgFirstIndices);
                var vsProject = rgpProjects[projIndex];
                var dteProject = TryGetDteProjectFromIVsProject(vsProject);

                this.ProjectFileRemoved?.Invoke(path, dteProject);
                Helpers.Diagnostic.Logger.LogDebug($"[VsProjectFileTrackerService] Removed: {path} [{dteProject.UniqueName}]");
            }

            return VSConstants.S_OK;
        }

        public int OnAfterRenameFiles(int cProjects, int cFiles, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgszMkOldNames, string[] rgszMkNewNames,
            VSRENAMEFILEFLAGS[] rgFlags) {
            return VSConstants.S_OK;
        }
        
        public int OnAfterAddDirectoriesEx(int cProjects, int cDirectories, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgpszMkDocuments, VSADDDIRECTORYFLAGS[] rgFlags) {
            return VSConstants.S_OK;
        }
        
        public int OnAfterRemoveDirectories(int cProjects, int cDirectories, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgpszMkDocuments, VSREMOVEDIRECTORYFLAGS[] rgFlags) {
            return VSConstants.S_OK;
        }
        
        public int OnAfterRenameDirectories(int cProjects, int cDirs, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgszMkOldNames, string[] rgszMkNewNames,
            VSRENAMEDIRECTORYFLAGS[] rgFlags) {
            return VSConstants.S_OK;
        }
        
        public int OnAfterSccStatusChanged(int cProjects, int cFiles, IVsProject[] rgpProjects,
            int[] rgFirstIndices, string[] rgpszMkDocuments, uint[] rgdwSccStatus) {
            return VSConstants.S_OK;
        }


        //
        // Internal
        //
        private int ProjectIndexFor(int fileIndex, int[] firstIndices) {
            for (int i = 0; i < firstIndices.Length; i++) {
                if (fileIndex < firstIndices[i]) {
                    return i - 1;
                }
            }
            return firstIndices.Length - 1;
        }

        EnvDTE.Project? TryGetDteProjectFromIVsProject(IVsProject vsProject) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var hierarchy = vsProject as IVsHierarchy;
            if (hierarchy == null) {
                return null;
            }

            hierarchy.GetProperty(
                VSConstants.VSITEMID_ROOT,
                (int)__VSHPROPID.VSHPROPID_ExtObject,
                out object value);

            return value as EnvDTE.Project;
        }
    }
}