using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;
using System.Runtime.InteropServices.WindowsRuntime;
using Helpers.Attributes;


namespace TabsManagerExtension.VsShell.Document {
    //public partial class DocumentNode :
    //    Helpers.ObservableObject,
    //    Helpers.IAttachable<VsShell.Hierarchy.HierarchyItem>,
    //    IDisposable {

    //    public VsShell.Project.ProjectNode ProjectNode { get; }

    //    [InvalidatableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
    //    private VsShell.Hierarchy.HierarchyItem _hierarchyItem;

    //    [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
    //    private bool _isAttachedHierarchyItem = false;

    //    [ObservableProperty(AccessMarker.Get, AccessMarker.PrivateSet)]
    //    private bool _isOppenedWithProjectContext = false;


    //    private bool _isInvalidated = false;
    //    public bool IsInvalidated => _isInvalidated;


    //    private bool _isDisposed = false;
    //    public bool IsDisposed => _isDisposed;

    //    public DocumentNode(
    //        VsShell.Project.ProjectNode projectNode,
    //        VsShell.Hierarchy.HierarchyItem hierarchyItem
    //        ) {
    //        this.ProjectNode = projectNode;
    //        this.Attach(hierarchyItem);
    //    }


    //    public void Invalidate() {
    //        if (_isDisposed) {
    //            return;
    //        }
    //        if (_isInvalidated) {
    //            return;
    //        }

    //        this.InvalidateProperties();

    //        _isInvalidated = true;
    //    }


    //    public void Dispose() {
    //        if (_isDisposed) {
    //            return;
    //        }

    //        this.Invalidate();

    //        foreach (var disposable in Helpers.Reflection.GetPropertiesOf<IDisposable>(this)) {
    //            disposable.Dispose();
    //        }

    //        _isDisposed = true;
    //        base.OnPropertyChanged(nameof(this.IsDisposed));
    //    }


    //    public void Attach(VsShell.Hierarchy.HierarchyItem newHierarchyItem) {
    //        if (this.IsAttachedHierarchyItem) {
    //            return;
    //        }

    //        if (this.HierarchyItem is IDisposable disposable) {
    //            disposable.Dispose();
    //        }

    //        this.HierarchyItem = newHierarchyItem;
    //        this.IsAttachedHierarchyItem = true;
    //    }


    //    public void Detach(Helpers.TypeMarker<VsShell.Hierarchy.HierarchyItem> _) {
    //        if (!this.IsAttachedHierarchyItem) {
    //            return;
    //        }

    //        if (this.HierarchyItem is Helpers.IInvalidatable invalidatable) {
    //            invalidatable.Invalidate();
    //        }

    //        this.IsOppenedWithProjectContext = false;
    //        this.IsAttachedHierarchyItem = false;
    //    }


    //    public override bool Equals(object? obj) {
    //        if (obj is not DocumentNode other) {
    //            return false;
    //        }

    //        return
    //            this.ProjectNode.ProjectGuid == other.ProjectNode.ProjectGuid &&
    //            this.HierarchyItemCached.ItemIdCached == other.HierarchyItemCached.ItemIdCached &&
    //            StringComparer.OrdinalIgnoreCase.Equals(
    //                this.HierarchyItemCached.FilePathCached,
    //                other.HierarchyItemCached.FilePathCached);
    //    }


    //    public override int GetHashCode() {
    //        unchecked {
    //            int hash = 17;

    //            hash = hash * 31 + this.ProjectNode.ProjectGuid.GetHashCode();
    //            hash = hash * 31 + this.HierarchyItemCached.ItemIdCached.GetHashCode();
    //            hash = hash * 31 + (this.HierarchyItemCached.FilePathCached != null
    //                ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.HierarchyItemCached.FilePathCached)
    //                : 0);

    //            return hash;
    //        }
    //    }

    //    public override string ToString() {
    //        return $"DocumentNode({this.ToStringCore()})";
    //    }

    //    protected string ToStringCore() {
    //        return $"FilePath='{this.HierarchyItem.FilePath}', Project='{this.ProjectNode.UniqueName}', ItemId={this.HierarchyItem.ItemId}";
    //    }


    //    protected void OpenWithProjectContext() {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        if (this.IsDisposed) {
    //            System.Diagnostics.Debugger.Break();
    //            return;
    //        }

    //        if (!this.ProjectNode.IsLoaded) {
    //            System.Diagnostics.Debugger.Break();
    //            return;
    //        }

    //        // Сохраняем активный документ до всех действий.
    //        var activeDocumentBefore = PackageServices.Dte2.ActiveDocument;

    //        // Попытка найти первый cpp/h файл проекта,
    //        // чтобы открыть его и "переключить" контекст редактора на нужный проект.
    //        // Это нужно для того, чтобы при открытии внешнего include файла
    //        // Visual Studio знала, что контекстом открытия является именно этот проект.
    //        var includeDependencyAnalyzer = VsShell.Solution.Services.IncludeDependencyAnalyzerService.Instance;
    //        var allTransitiveIncludingFiles = includeDependencyAnalyzer
    //            .GetTransitiveFilesIncludersByIncludePath(this.HierarchyItem.FilePath);

    //        var currentProjectTransitiveIncludingFiles = allTransitiveIncludingFiles
    //            .Where(sf => sf.ProjectNode.Equals(this.ProjectNode))
    //            .ToList();

    //        // Нужно открывать именно .cpp файл который реально включает наш include,
    //        // чтоб сработала смена контекста.
    //        var contextSwitchFile = currentProjectTransitiveIncludingFiles
    //            .FirstOrDefault(sf => System.IO.Path.GetExtension(sf.FilePath) == ".cpp")
    //            ?.FilePath;

    //        if (contextSwitchFile == null) {
    //            return;
    //        }

    //        var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
    //        var contextSwitchDocumentNode = solutionHierarchyAnalyzer.SourcesRepresentationsTable
    //            .GetDocumentByProjectAndDocumentPath(this.ProjectNode, contextSwitchFile);

    //        if (contextSwitchDocumentNode == null) {
    //            System.Diagnostics.Debugger.Break();
    //            return;
    //        }

    //        bool needCloseContextSwitchDocumentNode =
    //            !Utils.EnvDteUtils.IsDocumentOpen(contextSwitchDocumentNode.HierarchyItem.FilePath);

    //        int hr = VSConstants.S_OK;

    //        // Открываем файл в контексте проекта.
    //        hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
    //            this.ProjectNode.ProjectHierarchy.VsHierarchy,
    //            this.HierarchyItem.ItemId);
    //        ErrorHandler.ThrowOnFailure(hr);

    //        // Переключаемся на файл который включает наш файл (для смены activeDocumentFrame)
    //        // иначе IntelliSense не подхватит контекст.
    //        if (contextSwitchDocumentNode != null) {
    //            hr = Utils.VsHierarchyUtils.ClickOnSolutionHierarchyItem(
    //                contextSwitchDocumentNode.ProjectNode.ProjectHierarchy.VsHierarchy,
    //                contextSwitchDocumentNode.HierarchyItem.ItemId);
    //            ErrorHandler.ThrowOnFailure(hr);
    //        }

    //        // Закрываем временный файл переключения контекста.
    //        if (needCloseContextSwitchDocumentNode) {
    //            var doc = PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
    //                .FirstOrDefault(d => string.Equals(d.FullName, contextSwitchDocumentNode.HierarchyItem.FilePath, StringComparison.OrdinalIgnoreCase));

    //            doc?.Close(EnvDTE.vsSaveChanges.vsSaveChangesNo);
    //        }

    //        this.IsOppenedWithProjectContext = true;

    //        // Возвращаем активным предыдущий документ.
    //        VsixThreadHelper.RunOnUiThread(async () => {
    //            await Task.Delay(20);
    //            activeDocumentBefore?.Activate();
    //        });
    //    }
    //}


    ////
    //// SharedItemNode
    ////
    //public sealed class SharedItemNode : DocumentNode {
    //    private IVsHierarchy? _sharedProjectHierarchy;

    //    public SharedItemNode(
    //        VsShell.Project.ProjectNode projectNode,
    //        VsShell.Hierarchy.HierarchyItem hierarchyItem
    //        ) : base(projectNode, hierarchyItem) {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        IVsHierarchy? sharedProjectHierarchy = null;
    //        this.ProjectNode.ProjectHierarchy.VsHierarchy.GetProperty(
    //            hierarchyItem.ItemId,
    //            (int)__VSHPROPID7.VSHPROPID_SharedProjectHierarchy,
    //            out var sharedProjectHierarchyObj);

    //        if (sharedProjectHierarchyObj is IVsHierarchy hierarchy) {
    //            sharedProjectHierarchy = hierarchy;
    //        }

    //        _sharedProjectHierarchy = sharedProjectHierarchy;
    //    }


    //    public new void OpenWithProjectContext() {
    //        if (!this.ProjectNode.IsLoaded) {
    //            Utils.VsHierarchyUtils.ReloadProject(this.ProjectNode.ProjectGuid);
    //        }

    //        // Получаем все sharedItems проекты, которые знают про этот файл.
    //        var solutionHierarchyAnalyzer = VsShell.Solution.Services.SolutionHierarchyAnalyzerService.Instance;
    //        var sharedItemsProjectNodes = solutionHierarchyAnalyzer.SharedItemsRepresentationsTable
    //            .GetProjectsByDocumentPath(this.HierarchyItem.FilePath);

    //        var externalIncludesProjectNodes = solutionHierarchyAnalyzer.ExternalIncludeRepresentationsTable
    //            .GetProjectsByDocumentPath(this.HierarchyItem.FilePath);

    //        // Собираем все проекты на выгрузку, кроме текущего 'this.ProjectNode'
    //        // и оригинального 'IsSharedProject'.
    //        var sharedItemsProjectGuidsToUnload = sharedItemsProjectNodes
    //            .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode) && !p.IsSharedProject)
    //            .Select(p => p.ProjectGuid)
    //            .ToList();

    //        var externalIncludesProjectGuidsToUnload = externalIncludesProjectNodes
    //            .Where(p => p.IsLoaded && !p.Equals(this.ProjectNode))
    //            .Select(p => p.ProjectGuid)
    //            .ToList();

    //        foreach (var projectGuid in sharedItemsProjectGuidsToUnload) {
    //            Utils.VsHierarchyUtils.UnloadProject(projectGuid);
    //        }

    //        // Выгружаем все остальные связанные проекты (т.е. те которые содержат данный файл как External Dependencies)
    //        // только лишь когда на целевой проект ещще не было переключений контекста или когда
    //        // имеются другие sharedItems проект которые еще не выгружены. Это нужно чтобы
    //        // контекст целевого проекта установился наверняка.
    //        bool shouldReloadExternalIncludesProjects =
    //            sharedItemsProjectGuidsToUnload.Count > 0 ||
    //            base.IsOppenedWithProjectContext == false;

    //        if (shouldReloadExternalIncludesProjects) {
    //            foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
    //                Utils.VsHierarchyUtils.UnloadProject(projectGuid);
    //            }
    //        }

    //        base.OpenWithProjectContext();

    //        if (shouldReloadExternalIncludesProjects) {
    //            foreach (var projectGuid in externalIncludesProjectGuidsToUnload) {
    //                Utils.VsHierarchyUtils.ReloadProject(projectGuid);
    //            }
    //        }
    //    }


    //    public override string ToString() {
    //        return $"SharedItemNode({base.ToStringCore()})";
    //    }
    //}


    ////
    //// ExternalInclude
    ////
    //public sealed class ExternalInclude : DocumentNode {
    //    public ExternalInclude(
    //        VsShell.Project.ProjectNode projectNode,
    //        VsShell.Hierarchy.HierarchyItem hierarchyItem
    //        ) : base(projectNode, hierarchyItem) {
    //    }


    //    public new void OpenWithProjectContext() {
    //        base.OpenWithProjectContext();
    //    }


    //    public override string ToString() {
    //        return $"ExternalInclude({base.ToStringCore()})";
    //    }
    //}
}