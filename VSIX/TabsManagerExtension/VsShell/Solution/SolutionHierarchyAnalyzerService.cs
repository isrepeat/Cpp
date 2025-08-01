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


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// SolutionHierarchyRepresentationsTable
    /// </summary>
    public sealed class SolutionHierarchyRepresentationsTable
        : Helpers.RepresentationsTableBase<Document.DocumentEntryBase> {

        private readonly WeakReference<IReadOnlyList<Project.ProjectEntry>> _weakSolutionProjects;

        private Dictionary<
             string,
             List<Project.ProjectEntry>
             > _mapFilePathToListProject = new();

        private Dictionary<
            Project.ProjectCommonStateViewModel,
            Project.ProjectEntry
            > _mapProjectViewModelToProjectEntry = new();

        private Dictionary<
            Project.ProjectEntry,
            Dictionary<string, Document.DocumentEntryBase>
            > _mapProjectToDictFilePathToDocument = new();

        public SolutionHierarchyRepresentationsTable(IReadOnlyList<Project.ProjectEntry> solutionProjects) {
            _weakSolutionProjects = new WeakReference<IReadOnlyList<Project.ProjectEntry>>(solutionProjects);
        }

        public override void BuildRepresentations() {
            _mapFilePathToListProject.Clear();
            _mapProjectViewModelToProjectEntry.Clear();
            _mapProjectToDictFilePathToDocument.Clear();

            if (!_weakSolutionProjects.TryGetTarget(out var currentProjects)) {
                return;
            }

            _mapProjectViewModelToProjectEntry = currentProjects
                .ToDictionary(p => p.BaseViewModel, p => p);

            _mapFilePathToListProject = base.Records
                .GroupBy(
                    record => record.BaseViewModel.HierarchyItemEntry.BaseViewModel.FilePath,
                    StringComparer.OrdinalIgnoreCase
                )
                .ToDictionary(
                    g => g.Key,
                    g => g
                        .Select(record => {
                            _mapProjectViewModelToProjectEntry.TryGetValue(record.BaseViewModel.ProjectBaseViewModel, out var projectEntry);
                            Helpers.ThrowableAssert.Require(projectEntry != null);
                            return projectEntry;
                        })
                        .Distinct()
                        .ToList(),
                    StringComparer.OrdinalIgnoreCase
                );

            foreach (var record in base.Records) {
                _mapProjectViewModelToProjectEntry.TryGetValue(record.BaseViewModel.ProjectBaseViewModel, out var projectEntry);
                Helpers.ThrowableAssert.Require(projectEntry != null);

                var documentPath = record.BaseViewModel.HierarchyItemEntry.BaseViewModel.FilePath;

                if (!_mapProjectToDictFilePathToDocument.TryGetValue(projectEntry, out var dict)) {
                    dict = new Dictionary<string, Document.DocumentEntryBase>(StringComparer.OrdinalIgnoreCase);
                    _mapProjectToDictFilePathToDocument[projectEntry] = dict;
                }

                dict[documentPath] = record;
            }
        }

        public IReadOnlyCollection<Project.ProjectEntry> GetAllProjects() {
            return _mapProjectToDictFilePathToDocument.Keys;
        }

        public IReadOnlyList<Project.ProjectEntry> GetProjectsByDocumentPath(string documentPath) {
            if (_mapFilePathToListProject.TryGetValue(documentPath, out var projectsList)) {
                return projectsList;
            }
            return Array.Empty<Project.ProjectEntry>();
        }

        public Document.DocumentEntryBase? GetDocumentByProjectAndDocumentPath(
            Project.ProjectEntry projectEntry,
            string documentPath
            ) {
            if (_mapProjectToDictFilePathToDocument.TryGetValue(projectEntry, out var dictFilePathToDocument)) {
                if (dictFilePathToDocument.TryGetValue(documentPath, out var documentEntryBase)) {
                    return documentEntryBase;
                }
            }
            return null;
        }

        public Document.DocumentEntryBase? GetDocumentByProjectAndDocumentPath(
            Project.ProjectCommonStateViewModel projectViewModel,
            string documentPath
            ) {
            if (_mapProjectViewModelToProjectEntry.TryGetValue(projectViewModel, out var projectEntry)) {
                return this.GetDocumentByProjectAndDocumentPath(projectEntry, documentPath);
            }
            return null;
        }
    }


    /// <summary>
    /// SolutionHierarchyAnalyzerService
    /// </summary>
    // TODO: перед использованием класса убедись что анализ projectEntrys закончен.
    // Сделай что-то типа отложнного выполнений дальнешей логики в caller после завершения анализа, 
    // например когда происходит выгрузка / загрука проекта.
    public class SolutionHierarchyAnalyzerService :
        TabsManagerExtension.Services.SingletonServiceBase<SolutionHierarchyAnalyzerService>,
        TabsManagerExtension.Services.IExtensionService {

        private readonly Helpers.Collections.DisposableList<VsShell.Project.ProjectEntry> _solutionProjects = new();
        public IReadOnlyList<VsShell.Project.ProjectEntry> SolutionProjects => _solutionProjects;

        public IReadOnlyList<VsShell.Project.LoadedProject> LoadedProjects =>
            _solutionProjects
                .Select(p => p.MultiState.Current)
                .OfType<VsShell.Project.LoadedProject>()
                .ToList();

        public IReadOnlyList<VsShell.Project.UnloadedProject> UnloadedProjects =>
            _solutionProjects
                .Select(p => p.MultiState.Current)
                .OfType<VsShell.Project.UnloadedProject>()
                .ToList();


        public SolutionHierarchyRepresentationsTable ExternalIncludeRepresentationsTable { get; }
        public SolutionHierarchyRepresentationsTable SharedItemsRepresentationsTable { get; }
        public SolutionHierarchyRepresentationsTable SourcesRepresentationsTable { get; }



        //private Helpers.DirectoryWatcher? _solutionDirectoryWatcher;
        //private DispatcherTimer _delayedFileChangeTimer;

        private readonly HashSet<Helpers.DirectoryChangedEventArgs> _pendingChangedFiles = new();
        private string _lastLoadedSolutionName;
        private bool _analyzingInProcess = false;

        public SolutionHierarchyAnalyzerService() {
            this.ExternalIncludeRepresentationsTable = new SolutionHierarchyRepresentationsTable(_solutionProjects);
            this.SharedItemsRepresentationsTable = new SolutionHierarchyRepresentationsTable(_solutionProjects);
            this.SourcesRepresentationsTable = new SolutionHierarchyRepresentationsTable(_solutionProjects);
        }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return new[] {
                typeof(VsShell.Services.VsIDEStateFlagsTrackerService),
                typeof(VsShell.Solution.Services.VsSolutionEventsTrackerService),
            };
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.Add(this.OnSolutionLoaded);
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.InvokeForLastHandlerIfTriggered();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.Add(this.OnSolutionClosed);
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.InvokeForLastHandlerIfTriggered();

            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded += this.OnProjectLoaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded += this.OnProjectUnloaded;

            //string? solutionDir = Path.GetDirectoryName(PackageServices.Dte2.Solution.FullName);
            //if (solutionDir != null && Directory.Exists(solutionDir)) {
            //    _solutionDirectoryWatcher = new Helpers.DirectoryWatcher(solutionDir);
            //    _solutionDirectoryWatcher.DirectoryChanged += this.OnSolutionDirectoryChanged;
            //}

            //// Используем таймер для отложенной обработки изменённых файлов:
            //// обработка произойдёт только через заданный интервал после последнего события.
            //_delayedFileChangeTimer = new DispatcherTimer {
            //    Interval = TimeSpan.FromMilliseconds(500)
            //};
            //_delayedFileChangeTimer.Tick += (_, _) => this.OnDelayedFileChangeTimerTick();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            //_delayedFileChangeTimer.Stop();
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectUnloaded -= this.OnProjectUnloaded;
            VsShell.Solution.Services.VsSolutionEventsTrackerService.Instance.ProjectLoaded -= this.OnProjectLoaded;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.Remove(this.OnSolutionClosed);
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.Remove(this.OnSolutionLoaded);

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[ExternalDependenciesGraphService] Disposed.");
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public void AnalyzeDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _analyzingInProcess = true;
            this.SourcesRepresentationsTable.Clear();
            this.SharedItemsRepresentationsTable.Clear();

            foreach (var projectEntry in _solutionProjects) {
                this.SourcesRepresentationsTable.AddRange(projectEntry.BaseViewModel.Sources);
                this.SharedItemsRepresentationsTable.AddRange(projectEntry.BaseViewModel.SharedItems);
            }

            this.SourcesRepresentationsTable.BuildRepresentations();
            this.SharedItemsRepresentationsTable.BuildRepresentations();
            _analyzingInProcess = false;
        }


        // Держим отдельно ExternalIncludeRepresentationsTable чтобы не было пересечения filePath с SharedItems.
        // Да и к тому же ExternalIncludes часто приходится обновлять при запросе, в то время как projectEntry.Sources и
        // projectEntry.SharedItems обновлять можно по ивентам загрузки / выгрузки решения или изменения файлов проекта.
        public void AnalyzeExternalIncludes() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _analyzingInProcess = true;
            this.ExternalIncludeRepresentationsTable.Clear();

            foreach (var projectEntry in _solutionProjects) {
                this.ExternalIncludeRepresentationsTable.AddRange(projectEntry.BaseViewModel.ExternalIncludes);
            }

            this.ExternalIncludeRepresentationsTable.BuildRepresentations();
            _analyzingInProcess = false;
        }


        public bool IsReady() {
            return !_analyzingInProcess;
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnSolutionLoaded(string solutionName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionLoaded()");
            ThreadHelper.ThrowIfNotOnUIThread();

            if (solutionName == _lastLoadedSolutionName) {
                return;
            }
            _lastLoadedSolutionName = solutionName;

            this.AnalyzeSolutionProjects();
            this.AnalyzeDocuments();
            this.AnalyzeExternalIncludes();
        }

        private void OnSolutionClosed(string solutionName) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnSolutionClosed()");
            ThreadHelper.ThrowIfNotOnUIThread();

            _solutionProjects.ClearAndDispose();
        }


        private void OnProjectLoaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("OnProjectLoaded()");
            ThreadHelper.ThrowIfNotOnUIThread();
            
            var projectRealHierarchy = e.NewHierarchy.MultiState.As<Hierarchy.RealHierarchyItem>();
            
            PackageServices.VsSolution.GetGuidOfProject(projectRealHierarchy.VsRealHierarchy, out var projectGuid);
            var existingSolutionProject = _solutionProjects.FirstOrDefault(p => p.BaseViewModel.ProjectGuid == projectGuid);
            if (existingSolutionProject == null) {
                return;
            }

            existingSolutionProject.MultiState.SwitchTo<Project.LoadedProject>(e.NewHierarchy); // RealHierarchy

            this.AnalyzeDocuments();
            this.AnalyzeExternalIncludes();
        }

        private void OnProjectUnloaded(_EventArgs.ProjectHierarchyChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var projectRealHierarchy = e.OldHierarchy.MultiState.As<Hierarchy.RealHierarchyItem>();

            PackageServices.VsSolution.GetGuidOfProject(projectRealHierarchy.VsRealHierarchy, out var projectGuid);
            var existingSolutionProject = _solutionProjects.FirstOrDefault(p => p.BaseViewModel.ProjectGuid == projectGuid);
            if (existingSolutionProject == null) {
                return;
            }

            existingSolutionProject.MultiState.SwitchTo<Project.UnloadedProject>(e.NewHierarchy); // StubHierarchy

            this.AnalyzeDocuments();
            this.AnalyzeExternalIncludes();
        }


        private void OnSolutionDirectoryChanged(Helpers.DirectoryChangedEventArgs e) {
            lock (_pendingChangedFiles) {
                _pendingChangedFiles.Add(e);
            }

            //_delayedFileChangeTimer.Stop();
            //_delayedFileChangeTimer.Start();
        }


        //private void OnDelayedFileChangeTimerTick() {
        //    _delayedFileChangeTimer.Stop();

        //    List<Helpers.DirectoryChangedEventArgs> changedFiles;
        //    lock (_pendingChangedFiles) {
        //        changedFiles = _pendingChangedFiles.ToList();
        //        _pendingChangedFiles.Clear();
        //    }

        //    foreach (var changedFile in changedFiles) {
        //        this.ProcessChangedFile(changedFile);
        //    }
        //}


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void AnalyzeSolutionProjects() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _solutionProjects.ClearAndDispose();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_ALLINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var vsHierarchy = hierarchies[0];
                if (vsHierarchy == null) {
                    continue;
                }

                Project.ProjectEntry projectEntry = null;

                if (Utils.VsHierarchyUtils.IsRealHierarchy(vsHierarchy)) {
                    var realHierarchyItemEntry = Hierarchy.HierarchyItemEntry.CreateWithState<Hierarchy.RealHierarchyItem>(
                        new Hierarchy.HierarchyItemMultiStateElement(
                            vsHierarchy,
                            VSConstants.VSITEMID_ROOT
                            ));

                    projectEntry = new Project.ProjectEntry(
                        new Project.ProjectMultiStateElement(
                            realHierarchyItemEntry
                            ));

                    projectEntry.MultiState.SwitchTo<Project.LoadedProject>();
                }
                else if (Utils.VsHierarchyUtils.IsStubHierarchy(vsHierarchy)) {
                    var stubHierarchyItemEntry = Hierarchy.HierarchyItemEntry.CreateWithState<Hierarchy.StubHierarchyItem>(
                        new Hierarchy.HierarchyItemMultiStateElement(
                            vsHierarchy,
                            VSConstants.VSITEMID_ROOT
                            ));

                    projectEntry = new Project.ProjectEntry(
                        new Project.ProjectMultiStateElement(
                            stubHierarchyItemEntry
                            ));

                    projectEntry.MultiState.SwitchTo<Project.UnloadedProject>();
                }

                _solutionProjects.Add(projectEntry);
            }
        }


        private void ProcessChangedFile(Helpers.DirectoryChangedEventArgs changedFile) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var ext = Path.GetExtension(changedFile.FullPath);
            switch (ext) {
                case ".h":
                case ".hpp":
                case ".cpp":
                    break;

                default:
                    return;
            }

            Helpers.Diagnostic.Logger.LogDebug($"changedFile = {changedFile.FullPath}");

            switch (changedFile.ChangeType) {
                case Helpers.DirectoryChangeType.Changed:
                case Helpers.DirectoryChangeType.Created:
                case Helpers.DirectoryChangeType.Renamed:
                case Helpers.DirectoryChangeType.Deleted:
                    //this.Analyze();
                    break;
            }
        }
    }
}