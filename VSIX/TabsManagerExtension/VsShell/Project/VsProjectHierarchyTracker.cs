using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using Helpers.Text.Ex;
using Microsoft.VisualStudio.Package;
using TabsManagerExtension.VsShell.Document;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.VsShell.Project {
    public class ProjectHierarchyTracker :
        IVsHierarchyEvents,
        IDisposable {

        public Helpers.Events.Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? ExternalDependenciesChanged = new();
        public Helpers.Events.Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SharedItemsChanged = new();
        public Helpers.Events.Action<_EventArgs.ProjectHierarchyItemsChangedEventArgs>? SourcesChanged = new();

        private readonly IVsHierarchy _projectHierarchy;
        private uint _cookie;

        private Helpers.Time.DelayedEventsHandler _delayedEventsHandler;

        private ProjectExternalDependenciesAnalyzer _projectExternalDependenciesAnalyzer;
        private ProjectSharedItemsAnalyzer _projectSharedItemsAnalyzer;
        private ProjectSourcesAnalyzer _projectSourcesAnalyzer;

        private bool _disposed = false;

        public ProjectHierarchyTracker(
            IVsHierarchy projectHierarchy
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            _projectHierarchy = projectHierarchy;
            _projectHierarchy.AdviseHierarchyEvents(this, out _cookie);

            _delayedEventsHandler = new Helpers.Time.DelayedEventsHandler(TimeSpan.FromMilliseconds(500));
            _delayedEventsHandler.OnReady += this.OnRefreshAnalyzers;

            _projectExternalDependenciesAnalyzer = new ProjectExternalDependenciesAnalyzer(_projectHierarchy);
            _projectExternalDependenciesAnalyzer.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;

            _projectSharedItemsAnalyzer = new ProjectSharedItemsAnalyzer(_projectHierarchy);
            _projectSharedItemsAnalyzer.SharedItemsChanged += this.OnSharedItemsChanged;

            _projectSourcesAnalyzer = new ProjectSourcesAnalyzer(_projectHierarchy);
            _projectSourcesAnalyzer.SourcesChanged += this.OnSourcesChanged;

            // Анализируем sources и sahredITems сейчас, т.к. к ним уже есть доступ.
            this.OnRefreshAnalyzers();
        }


        //
        // IDisposable
        //
        public void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_disposed) {
                return;
            }

            Helpers.Events.ActionUtils.ClearSubscribers(this);

            _projectSourcesAnalyzer.SourcesChanged -= this.OnSourcesChanged;
            _projectSourcesAnalyzer = null;
            
            _projectSharedItemsAnalyzer.SharedItemsChanged -= this.OnSharedItemsChanged;
            _projectSharedItemsAnalyzer = null;
            
            _projectExternalDependenciesAnalyzer.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
            _projectExternalDependenciesAnalyzer = null;

            _delayedEventsHandler.Clear();
            _delayedEventsHandler = null;

            if (_cookie != 0) {
                _projectHierarchy.UnadviseHierarchyEvents(_cookie);
                _cookie = 0;
            }

            _disposed = true;
        }


        //
        // IVsHierarchyEvents
        //
        public int OnItemAdded(uint itemIdParent, uint itemIdSiblingPrev, uint itemIdAdded) {
            Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnItemAdded: parent={itemIdParent}, added={itemIdAdded}");
            _delayedEventsHandler.Schedule();
            return VSConstants.S_OK;
        }

        public int OnItemsAppended(uint itemIdParent) {
            //Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnItemsAppended: parent={itemIdParent}");
            return VSConstants.S_OK;
        }

        public int OnItemDeleted(uint itemId) {
            Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnItemDeleted: {itemId}");
            _delayedEventsHandler.Schedule();
            return VSConstants.S_OK;
        }

        public int OnPropertyChanged(uint itemId, int propId, uint flags) {
            Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnPropertyChanged: item={itemId}, prop={propId}, flags={flags}");
            return VSConstants.S_OK;
        }

        // Изменения "External Dependencies" папки гарантированно тригерят этот метод.
        // Так же этот метод тригериться через несколько секунд после загрузки решения,
        // это связано с тем что некоторые элемнеты получают статус SharedItems.
        public int OnInvalidateItems(uint itemIdParent) {
            Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnInvalidateItems: parent={itemIdParent}");
           //Console.Beep(frequency: 1000, duration: 150);
            _delayedEventsHandler.Schedule();
            return VSConstants.S_OK;
        }

        public int OnInvalidateIcon(IntPtr hicon) {
            //Helpers.Diagnostic.Logger.LogDebug($"[Watcher] OnInvalidateIcon");
            return VSConstants.S_OK;
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentExternalDependenciesItems() {
            return _projectExternalDependenciesAnalyzer.GetCurrentExternalDependenciesItems();
        }

        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentSharedItems() {
            return _projectSharedItemsAnalyzer.GetCurrentSharedItems();
        }

        public IReadOnlyList<Hierarchy.HierarchyItemEntry> GetCurrentSources() {
            return _projectSourcesAnalyzer.GetCurrentSources();
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnRefreshAnalyzers() {
            _projectExternalDependenciesAnalyzer.Refresh();
            _projectSharedItemsAnalyzer.Refresh();
            _projectSourcesAnalyzer.Refresh();
        }

        private void OnExternalDependenciesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.ExternalDependenciesChanged?.Invoke(e);
        }

        private void OnSharedItemsChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.SharedItemsChanged?.Invoke(e);
        }

        private void OnSourcesChanged(_EventArgs.ProjectHierarchyItemsChangedEventArgs e) {
            this.SourcesChanged?.Invoke(e);
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private bool IsProjectReferenceNode(IVsHierarchy hierarchy, uint itemId) {
            ThreadHelper.ThrowIfNotOnUIThread();

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_BrowseObject, out var browseObj);

            if (browseObj != null) {
                string typeName = browseObj.GetType().FullName;
                if (typeName.Contains("VCProjectReferenceShim")) {
                    return true;
                }
                else if (typeName.Contains("VCSharedProjectReferenceShim")) {
                    return true;
                }
            }

            return false;
        }


        private bool IsExternalDependenciesNode(IVsHierarchy hierarchy, uint itemId) {
            ThreadHelper.ThrowIfNotOnUIThread();

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Caption, out var captionObj);
            var caption = captionObj as string ?? "";

            if (caption == "External Dependencies") {
                return true;
            }

            return false;
        }
    }
}