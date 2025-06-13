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
using EnvDTE;
using Microsoft.VisualStudio.Package;


namespace TabsManagerExtension.VsShell.Solution {

    public class IncludeFile {
        public string RawInclude { get; }
        public string NormalizedName { get; }
        //public EnvDTE.Project OwnerProject { get; }
        public State.Document.TabItemProject OwnerProject { get; }
        public IncludeFile(string rawInclude, EnvDTE.Project ownerProject) {
            this.RawInclude = rawInclude;
            this.NormalizedName = Path.GetFileName(rawInclude);
            //this.OwnerProject = ownerProject;
            this.OwnerProject = new State.Document.TabItemProject(ownerProject);
        }

        public override string ToString() => this.NormalizedName;

        public override int GetHashCode() => StringComparer.OrdinalIgnoreCase.GetHashCode(this.NormalizedName);

        public override bool Equals(object? obj) {
            return obj is IncludeFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.NormalizedName, other.NormalizedName);
        }
    }


    public class SourceFile {
        public string FilePath { get; }
        //public EnvDTE.Project Project { get; }
        public State.Document.TabItemProject Project { get; }
        public List<IncludeFile> Includes { get; } = new();

        public SourceFile(string filePath, EnvDTE.Project project) {
            this.FilePath = filePath;
            //this.Project = project;
            this.Project = new State.Document.TabItemProject(project);
        }

        public override int GetHashCode() {
            int h1 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath);
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.Project.ShellProject.Project.UniqueName);
            //return HashCode.Combine(h1, h2);
            return (h1 * 397) ^ h2;
        }

        public override bool Equals(object? obj) {
            return obj is SourceFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(
                       this.Project.ShellProject.Project.UniqueName,
                       other.Project.ShellProject.Project.UniqueName
                       );
        }

        public override string ToString() => $"{this.FilePath} [{this.Project.ShellProject.Project.UniqueName}]";
    }





    public class IncludeDependencyAnalyzer {
        private readonly EnvDTE80.DTE2 _dte;
        //private readonly Dictionary<string, List<IncludeFile>> _includesMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly HashSet<SourceFile> _sourceFiles = new();

        public IncludeDependencyAnalyzer() {
            ThreadHelper.ThrowIfNotOnUIThread();
            this._dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }

        public void Build() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("Build()");
            ThreadHelper.ThrowIfNotOnUIThread();

            this._sourceFiles.Clear();

            foreach (var project in GetAllProjects(this._dte)) {
                Helpers.Diagnostic.Logger.LogDebug($"[Project.Name] = {project.Name}");
                var stack = new Stack<ProjectItem>();

                foreach (ProjectItem item in project.ProjectItems) {
                    Helpers.Diagnostic.Logger.LogDebug($"[ProjectItem.Name] = {item.Name}");
                    stack.Push(item);
                }

                while (stack.Count > 0) {
                    var current = stack.Pop();
                    Helpers.Diagnostic.Logger.LogDebug($"[current.Name] = {current.Name}");

                    if (current.FileCount > 0 && current.FileCodeModel is VCFileCodeModel) {
                        string filePath = current.FileNames[1];
                        Helpers.Diagnostic.Logger.LogDebug($"[current.filePath] = {filePath}");

                        var includes = ExtractRawIncludes(filePath, project);

                        var source = new SourceFile(filePath, project);
                        source.Includes.AddRange(includes);
                        this._sourceFiles.Add(source);
                    }

                    if (current.ProjectItems != null) {
                        foreach (ProjectItem child in current.ProjectItems) {
                            stack.Push(child);
                        }
                    }
                }
            }
        }

        public void LogIncludeTree() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var source in this._sourceFiles.OrderBy(f => f.FilePath)) {
                Helpers.Diagnostic.Logger.LogDebug($"[File] {source.FilePath}");

                foreach (var include in source.Includes) {
                    string normalized = include.RawInclude.Replace('\\', '/');
                    Helpers.Diagnostic.Logger.LogDebug($"  └─ #include \"{normalized}\"");
                }

                if (source.Includes.Count == 0) {
                    Helpers.Diagnostic.Logger.LogDebug("  └─ (no includes)");
                }
            }
        }

        public IReadOnlyCollection<State.Document.TabItemProject> GetProjectsIncludingTransitive(string includeName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var transitiveFilePaths = this.GetFilesIncludingTransitive(includeName);

            var projects_ = transitiveFilePaths
                .Select(sf => sf.Project)
                .Distinct()
                .ToList();

            //var projects = this._sourceFiles
            //    .Where(sf => transitiveFilePaths.Contains(sf.FilePath))
            //    .Select(sf => sf.Project)
            //    .Distinct()
            //    .Select(p => new State.Document.TabItemProject(p))
            //    .ToList();

            //var projects = this._sourceFiles
            //    .Where(sf => transitiveFilePaths.Contains(sf.FilePath))
            //    .Select(sf => sf.Project.ShellProject.Project)
            //    .Distinct()
            //    .Select(p => new State.Document.TabItemProject(p))
            //    .ToList();

            return projects_;
        }


        public IReadOnlyList<SourceFile> GetFilesIncludingTransitive(string includeName) {
            var result = new HashSet<SourceFile>();
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var stack = new Stack<SourceFile>();

            string normalizedTargetInclude = includeName.Trim().Replace('\\', '/');

            // ① Найти все файлы, которые включают normalizedTargetInclude напрямую
            foreach (var source in this._sourceFiles) {
                foreach (var include in source.Includes) {
                    string normalizedInclude = include.RawInclude.Replace('\\', '/');

                    if (normalizedInclude.Equals(normalizedTargetInclude, StringComparison.OrdinalIgnoreCase)) {
                        if (result.Add(source)) {
                            // TODO: do not add .cpp because .cpp not included by anything
                            stack.Push(source);
                        }
                        break;
                    }
                }
            }

            // ② Обратный обход: кто включает найденные файлы
            while (stack.Count > 0) {
                var current = stack.Pop();

                if (!visited.Add(current.FilePath)) {
                    continue;
                }

                foreach (var source in this._sourceFiles) {
                    if (source.FilePath == "d:\\WORK\\C++\\Cpp\\DirectX\\Desktop\\DxDesktop\\[02] Dx SwapChain simple render\\SimpleSceneRenderer.h") {
                        int xx = 8;
                    }

                    foreach (var include in source.Includes) {
                        if (FilesAreSame(include.RawInclude, source.FilePath, current.FilePath)) {
                            if (result.Add(source)) {
                                stack.Push(source);
                                Helpers.Diagnostic.Logger.LogDebug($"[transitive] {source.FilePath} includes → {current.FilePath}");
                                break;
                            }
                        }
                    }
                }
            }

            return result.ToList();
        }


        //public IReadOnlyList<string> GetFilesIncludingTransitive(string includeName) {
        //    var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        //    var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        //    var stack = new Stack<SourceFile>();

        //    string normalizedTargetInclude = includeName.Trim().Replace('\\', '/');

        //    // ① Найти все файлы, которые включают normalizedTargetInclude напрямую
        //    foreach (var source in this._sourceFiles) {
        //        foreach (var include in source.Includes) {
        //            string normalizedInclude = include.RawInclude.Replace('\\', '/');

        //            if (normalizedInclude.Equals(normalizedTargetInclude, StringComparison.OrdinalIgnoreCase)) {
        //                if (result.Add(source.FilePath)) {
        //                    // TODO: do not add .cpp because .cpp not included by anything
        //                    stack.Push(source);
        //                }
        //                break;
        //            }
        //        }
        //    }

        //    // ② Обратный обход: кто включает найденные файлы
        //    while (stack.Count > 0) {
        //        var current = stack.Pop();

        //        if (!visited.Add(current.FilePath)) {
        //            continue;
        //        }

        //        foreach (var source in this._sourceFiles) {
        //            if (source.FilePath == "d:\\WORK\\C++\\Cpp\\DirectX\\Desktop\\DxDesktop\\[02] Dx SwapChain simple render\\SimpleSceneRenderer.h") {
        //                int xx = 8;
        //            }

        //            foreach (var include in source.Includes) {
        //                if (FilesAreSame(include.RawInclude, source.FilePath, current.FilePath)) {
        //                    if (result.Add(source.FilePath)) {
        //                        stack.Push(source);
        //                        Helpers.Diagnostic.Logger.LogDebug($"[transitive] {source.FilePath} includes → {current.FilePath}");
        //                        break;
        //                    }
        //                }
        //            }
        //        }
        //    }

        //    return result.ToList();
        //}


        private static bool FilesAreSame(string includeRaw, string includingFilePath, string candidateFilePath) {
            try {
                // includeRaw = Helpers/Dx/SwapChainPanel.h
                // includingFilePath = d:\WORK\C++\Cpp\DirectX\Desktop\DxDesktop\[02] Dx SwapChain simple render\SimpleSceneRenderer.h
                // candidateFilePath = d:\WORK\C++\Cpp\UtilityHelpersLib\Helpers\Helpers.Shared\Helpers\Dx\SwapChainPanel.h

                // TODO: improve to work with
                //   <Heplers.Desktop/Desktop.h>
                //   <Desktop.h>
                string parentDir = Path.GetDirectoryName(includingFilePath)!;
                string resolvedInclude = Path.GetFullPath(Path.Combine(parentDir, includeRaw.Replace('/', '\\')));
                string fullCandidate = Path.GetFullPath(candidateFilePath);

                return string.Equals(resolvedInclude, fullCandidate, StringComparison.OrdinalIgnoreCase);
            }
            catch {
                return false;
            }
        }


        //private bool FilesAreSame(string includeRaw, string includingFilePath, string candidateFilePath, EnvDTE.Project ownerProject) {
        //    try {
        //        string candidateFull = Path.GetFullPath(candidateFilePath);

        //        // ① Пробуем как относительный путь от файла
        //        string baseDir = Path.GetDirectoryName(includingFilePath)!;
        //        string resolvedFromLocal = Path.GetFullPath(Path.Combine(baseDir, includeRaw.Replace('/', '\\')));
        //        if (string.Equals(resolvedFromLocal, candidateFull, StringComparison.OrdinalIgnoreCase)) {
        //            return true;
        //        }

        //        // ② Пробуем директории проекта (ограниченно)
        //        foreach (var includeDir in GetProjectIncludeDirectories(ownerProject)) {
        //            string resolved = Path.GetFullPath(Path.Combine(includeDir, includeRaw.Replace('/', '\\')));
        //            if (string.Equals(resolved, candidateFull, StringComparison.OrdinalIgnoreCase)) {
        //                return true;
        //            }
        //        }
        //    }
        //    catch {
        //        // ignore
        //    }

        //    return false;
        //}


        //private List<string> GetProjectIncludeDirectories(EnvDTE.Project project) {
        //    ThreadHelper.ThrowIfNotOnUIThread();

        //    var result = new List<string>();

        //    if (project.Object is Microsoft.VisualStudio.VCProjectEngine.VCProject vcProject) {
        //        foreach (Microsoft.VisualStudio.VCProjectEngine.VCConfiguration config in vcProject.Configurations) {
        //            var clTool = config.Tools as Microsoft.VisualStudio.VCProjectEngine.IVCTool?[]
        //                         ?? config.Tools.Cast<object>().OfType<Microsoft.VisualStudio.VCProjectEngine.VCCLCompilerTool>().Cast<Microsoft.VisualStudio.VCProjectEngine.IVCTool?>().ToArray();

        //            foreach (var tool in clTool) {
        //                if (tool is Microsoft.VisualStudio.VCProjectEngine.VCCLCompilerTool cl) {
        //                    var dirs = cl.AdditionalIncludeDirectories?.Split(';') ?? Array.Empty<string>();
        //                    foreach (var dir in dirs) {
        //                        string trimmed = dir.Trim();
        //                        if (!string.IsNullOrEmpty(trimmed) && !trimmed.Contains("%")) {
        //                            try {
        //                                result.Add(Path.GetFullPath(trimmed));
        //                            }
        //                            catch {
        //                                // skip invalid
        //                            }
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //    }

        //    return result.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        //}





        //public IReadOnlyList<string> GetFilesIncludingTransitive(string includeName) {
        //    var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        //    var stack = new Stack<string>();

        //    string normalized = includeName.Trim().Replace('\\', '/');

        //    // ① Поиск файлов, напрямую включающих normalized
        //    foreach (var kvp in this._includesMap) {
        //        if (kvp.Value.Any(i => i.RawInclude.Replace('\\', '/').Equals(normalized, StringComparison.OrdinalIgnoreCase))) {
        //            if (result.Add(kvp.Key)) {
        //                stack.Push(kvp.Key);
        //            }
        //        }
        //    }

        //    // ② Обратный обход графа: ищем кто включает уже найденные
        //    while (stack.Count > 0) {
        //        var current = stack.Pop();
        //        string currentIncludeName = current.Trim().Replace('\\', '/');
        //        string currentShortName = Path.GetFileName(currentIncludeName);

        //        foreach (var kvp in this._includesMap) {
        //            if (kvp.Value.Any(i => i.RawInclude.Replace('\\', '/').Equals(currentShortName, StringComparison.OrdinalIgnoreCase))) {
        //                if (result.Add(kvp.Key)) {
        //                    stack.Push(kvp.Key);
        //                    Helpers.Diagnostic.Logger.LogDebug($"[transitive] {kvp.Key} includes → {current}");
        //                }
        //            }
        //        }
        //    }

        //    return result.ToList();
        //}

        public static List<IncludeFile> ExtractRawIncludes(string filePath, EnvDTE.Project ownerProject, int maxLinesToRead = 200) {
            var result = new List<IncludeFile>();
            string fileDir = Path.GetDirectoryName(filePath)!;

            using var reader = new StreamReader(filePath);
            int lineCount = 0;

            while (!reader.EndOfStream && lineCount < maxLinesToRead) {
                string? line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                lineCount++;
                string trimmed = line.TrimStart();
                if (!trimmed.StartsWith("#include")) {
                    continue;
                }

                int start = trimmed.IndexOfAny(new[] { '"', '<' });
                int end = trimmed.LastIndexOfAny(new[] { '"', '>' });

                if (start >= 0 && end > start) {
                    string raw = trimmed.Substring(start + 1, end - start - 1).Trim();
                    if (!string.IsNullOrWhiteSpace(raw)) {
                        result.Add(new IncludeFile(raw, ownerProject));
                    }
                }
            }

            return result;
        }




        private IEnumerable<ProjectItem> GetAllProjectItems() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (EnvDTE.Project project in GetAllProjects(this._dte)) {
                var stack = new Stack<ProjectItem>();

                foreach (ProjectItem item in project.ProjectItems) {
                    stack.Push(item);
                }

                while (stack.Count > 0) {
                    var current = stack.Pop();

                    yield return current;

                    if (current.ProjectItems != null) {
                        foreach (ProjectItem child in current.ProjectItems) {
                            stack.Push(child);
                        }
                    }
                }
            }
        }

        private static IEnumerable<EnvDTE.Project> GetAllProjects(EnvDTE80.DTE2 dte) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var queue = new Queue<EnvDTE.Project>();
            foreach (EnvDTE.Project p in dte.Solution.Projects) {
                queue.Enqueue(p);
            }

            while (queue.Count > 0) {
                var proj = queue.Dequeue();

                if (proj.Kind == EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder) {
                    foreach (ProjectItem item in proj.ProjectItems) {
                        if (item.SubProject != null) {
                            queue.Enqueue(item.SubProject);
                        }
                    }
                }
                else {
                    yield return proj;
                }
            }
        }
    }




    //public class IncludeDependencyAnalyzer {
    //    private readonly EnvDTE80.DTE2 _dte;
    //    private readonly Dictionary<string, List<string>> _includesMap = new(StringComparer.OrdinalIgnoreCase);
    //    private readonly Dictionary<string, HashSet<string>> _reverseMap = new(StringComparer.OrdinalIgnoreCase);

    //    public IncludeDependencyAnalyzer() {
    //        ThreadHelper.ThrowIfNotOnUIThread();
    //        _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
    //    }

    //    public void LogAllIncludesInSolution() {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        foreach (EnvDTE.Project project in GetAllProjects(_dte)) {
    //            var stack = new Stack<EnvDTE.ProjectItem>();

    //            foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
    //                stack.Push(item);
    //            }

    //            while (stack.Count > 0) {
    //                var current = stack.Pop();

    //                if (current.FileCount > 0 && current.FileCodeModel is VCFileCodeModel vcFileCodeModel) {
    //                    string filePath = current.FileNames[1];

    //                    Helpers.Diagnostic.Logger.LogDebug($"▶ Includes in: {filePath}");
    //                    foreach (EnvDTE.CodeElement element in vcFileCodeModel.CodeElements) {
    //                        PrintIncludeRecursive(element, 1);
    //                    }
    //                }

    //                if (current.ProjectItems != null) {
    //                    foreach (EnvDTE.ProjectItem child in current.ProjectItems)
    //                        stack.Push(child);
    //                }
    //            }
    //        }
    //    }

    //    private void PrintIncludeRecursive(EnvDTE.CodeElement element, int indent) {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        string pad = new string(' ', indent * 2);

    //        if (element is VCCodeInclude include) {
    //            string name = include.Name?.Trim('"', '<', '>') ?? "(null)";
    //            Helpers.Diagnostic.Logger.LogDebug($"{pad}- include: {name}");
    //        }

    //        if (element.Children is EnvDTE.CodeElements children) {
    //            foreach (EnvDTE.CodeElement child in children) {
    //                PrintIncludeRecursive(child, indent + 1);
    //            }
    //        }
    //    }



    //    public void Build() {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        this._includesMap.Clear();
    //        this._reverseMap.Clear();

    //        foreach (ProjectItem item in GetAllProjectItems()) {
    //            if (item.FileCount > 0 && item.FileCodeModel is VCFileCodeModel vcFileCodeModel) {
    //                string filePath = item.FileNames[1];

    //                var includes = ExtractRawIncludes(filePath).ToList();
    //                this._includesMap[filePath] = includes;

    //                foreach (var include in includes) {
    //                    if (!this._reverseMap.TryGetValue(include, out var parents)) {
    //                        parents = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
    //                        this._reverseMap[include] = parents;
    //                    }

    //                    parents.Add(filePath);
    //                }
    //            }
    //        }
    //    }

    //    public IReadOnlyList<string> GetDirectIncludes(string filePath) {
    //        return this._includesMap.TryGetValue(filePath, out var includes)
    //            ? includes
    //            : Array.Empty<string>();
    //    }

    //    public IReadOnlyList<string> GetFilesIncluding(string includePath) {
    //        return this._reverseMap.TryGetValue(includePath, out var parents)
    //            ? parents.ToList()
    //            : Array.Empty<string>();
    //    }


    //    public IReadOnlyList<string> GetFilesIncludingTransitive(string includeName) {
    //        var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
    //        var stack = new Stack<string>();

    //        string normalizedInclude = Path.GetFileName(includeName);

    //        // ВНИМАНИЕ: includeName = "Logger.h", а ключи в _includesMap — это пути файлов
    //        // Надо искать все файлы, которые где-то пишут: #include "Logger.h"
    //        foreach (var kvp in this._includesMap) {
    //            if (kvp.Value.Contains(normalizedInclude, StringComparer.OrdinalIgnoreCase)) {
    //                if (result.Add(kvp.Key)) {
    //                    stack.Push(kvp.Key);
    //                }
    //            }
    //        }

    //        // теперь вверх по тем, кто включает этот файл, и кто включает их и т.д.
    //        while (stack.Count > 0) {
    //            var current = stack.Pop();
    //            string currentFileName = Path.GetFileName(current);

    //            foreach (var kvp in this._includesMap) {
    //                if (kvp.Value.Contains(currentFileName, StringComparer.OrdinalIgnoreCase)) {
    //                    if (result.Add(kvp.Key)) {
    //                        stack.Push(kvp.Key);
    //                    }
    //                }
    //            }
    //        }

    //        return result.ToList();
    //    }

    //    public static List<string> ExtractRawIncludes(string filePath, int maxLinesToRead = 10) {
    //        var result = new List<string>();

    //        using var reader = new StreamReader(filePath);
    //        int lineCount = 0;

    //        while (!reader.EndOfStream && lineCount < maxLinesToRead) {
    //            string? line = reader.ReadLine();
    //            if (line == null) {
    //                break;
    //            }

    //            lineCount++;
    //            string trimmed = line.TrimStart();
    //            if (!trimmed.StartsWith("#include")) {
    //                continue;
    //            }

    //            int start = trimmed.IndexOfAny(new[] { '"', '<' });
    //            int end = trimmed.LastIndexOfAny(new[] { '"', '>' });

    //            if (start >= 0 && end > start) {
    //                string include = trimmed.Substring(start + 1, end - start - 1).Trim();
    //                string normalized = Path.GetFileName(include);

    //                if (!string.IsNullOrWhiteSpace(normalized)) {
    //                    result.Add(normalized);
    //                }
    //            }
    //        }

    //        return result;
    //    }



    //    private IEnumerable<ProjectItem> GetAllProjectItems() {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        foreach (EnvDTE.Project project in GetAllProjects(this._dte)) {
    //            var stack = new Stack<ProjectItem>();

    //            foreach (ProjectItem item in project.ProjectItems) {
    //                stack.Push(item);
    //            }

    //            while (stack.Count > 0) {
    //                var current = stack.Pop();

    //                yield return current;

    //                if (current.ProjectItems != null) {
    //                    foreach (ProjectItem child in current.ProjectItems) {
    //                        stack.Push(child);
    //                    }
    //                }
    //            }
    //        }
    //    }


    //    /// <summary>
    //    /// Итеративно получить все проекты решения (включая из Solution Folder)
    //    /// </summary>
    //    private static IEnumerable<EnvDTE.Project> GetAllProjects(EnvDTE80.DTE2 dte) {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        var queue = new Queue<EnvDTE.Project>();
    //        foreach (EnvDTE.Project p in dte.Solution.Projects)
    //            queue.Enqueue(p);

    //        while (queue.Count > 0) {
    //            var proj = queue.Dequeue();

    //            if (proj.Kind == EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder) {
    //                foreach (EnvDTE.ProjectItem item in proj.ProjectItems) {
    //                    if (item.SubProject != null)
    //                        queue.Enqueue(item.SubProject);
    //                }
    //            }
    //            else {
    //                yield return proj;
    //            }
    //        }
    //    }
    //}
}