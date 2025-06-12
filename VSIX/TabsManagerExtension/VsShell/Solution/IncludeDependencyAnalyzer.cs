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


namespace TabsManagerExtension.VsShell.Solution {
    public class IncludeFile {
        public string RawInclude { get; }       // строка как в коде: "../Shared/Logger.h"
        public string? ResolvedFullPath { get; } // абсолютный путь, если удалось определить

        public string? FileName => System.IO.Path.GetFileName(this.ResolvedFullPath ?? this.RawInclude);

        public IncludeFile(string rawInclude, string? resolvedFullPath) {
            this.RawInclude = rawInclude;
            this.ResolvedFullPath = resolvedFullPath;
        }

        public override string ToString() {
            return this.ResolvedFullPath ?? this.RawInclude;
        }

        public override int GetHashCode() {
            return (this.ResolvedFullPath ?? this.RawInclude).GetHashCode(StringComparison.OrdinalIgnoreCase);
        }

        public override bool Equals(object? obj) {
            if (obj is not IncludeFile other) {
                return false;
            }

            return string.Equals(this.ResolvedFullPath, other.ResolvedFullPath, StringComparison.OrdinalIgnoreCase)
                || string.Equals(this.RawInclude, other.RawInclude, StringComparison.OrdinalIgnoreCase);
        }
    }


    public class IncludeDependencyAnalyzer {
        private readonly EnvDTE80.DTE2 _dte;
        private readonly Dictionary<string, List<string>> _includesMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, HashSet<string>> _reverseMap = new(StringComparer.OrdinalIgnoreCase);

        public IncludeDependencyAnalyzer() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (EnvDTE80.DTE2)Package.GetGlobalService(typeof(EnvDTE.DTE));
        }

        public void LogAllIncludesInSolution() {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (EnvDTE.Project project in GetAllProjects(_dte)) {
                var stack = new Stack<EnvDTE.ProjectItem>();

                foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                    stack.Push(item);
                }

                while (stack.Count > 0) {
                    var current = stack.Pop();

                    if (current.FileCount > 0 && current.FileCodeModel is VCFileCodeModel vcFileCodeModel) {
                        string filePath = current.FileNames[1];

                        Helpers.Diagnostic.Logger.LogDebug($"▶ Includes in: {filePath}");
                        foreach (EnvDTE.CodeElement element in vcFileCodeModel.CodeElements) {
                            PrintIncludeRecursive(element, 1);
                        }
                    }

                    if (current.ProjectItems != null) {
                        foreach (EnvDTE.ProjectItem child in current.ProjectItems)
                            stack.Push(child);
                    }
                }
            }
        }

        private void PrintIncludeRecursive(EnvDTE.CodeElement element, int indent) {
            ThreadHelper.ThrowIfNotOnUIThread();

            string pad = new string(' ', indent * 2);

            if (element is VCCodeInclude include) {
                string name = include.Name?.Trim('"', '<', '>') ?? "(null)";
                Helpers.Diagnostic.Logger.LogDebug($"{pad}- include: {name}");
            }

            if (element.Children is EnvDTE.CodeElements children) {
                foreach (EnvDTE.CodeElement child in children) {
                    PrintIncludeRecursive(child, indent + 1);
                }
            }
        }



        public void Build() {
            ThreadHelper.ThrowIfNotOnUIThread();

            this._includesMap.Clear();
            this._reverseMap.Clear();

            foreach (ProjectItem item in GetAllProjectItems()) {
                if (item.FileCount > 0 && item.FileCodeModel is VCFileCodeModel vcFileCodeModel) {
                    string filePath = item.FileNames[1];

                    var includes = ExtractRawIncludes(filePath).ToList();
                    this._includesMap[filePath] = includes;

                    foreach (var include in includes) {
                        if (!this._reverseMap.TryGetValue(include, out var parents)) {
                            parents = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                            this._reverseMap[include] = parents;
                        }

                        parents.Add(filePath);
                    }
                }
            }
        }

        public IReadOnlyList<string> GetDirectIncludes(string filePath) {
            return this._includesMap.TryGetValue(filePath, out var includes)
                ? includes
                : Array.Empty<string>();
        }

        public IReadOnlyList<string> GetFilesIncluding(string includePath) {
            return this._reverseMap.TryGetValue(includePath, out var parents)
                ? parents.ToList()
                : Array.Empty<string>();
        }


        public IReadOnlyList<string> GetFilesIncludingTransitive(string includeName) {
            var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var stack = new Stack<string>();

            string normalizedInclude = Path.GetFileName(includeName);

            // ВНИМАНИЕ: includeName = "Logger.h", а ключи в _includesMap — это пути файлов
            // Надо искать все файлы, которые где-то пишут: #include "Logger.h"
            foreach (var kvp in this._includesMap) {
                if (kvp.Value.Contains(normalizedInclude, StringComparer.OrdinalIgnoreCase)) {
                    if (result.Add(kvp.Key)) {
                        stack.Push(kvp.Key);
                    }
                }
            }

            // теперь вверх по тем, кто включает этот файл, и кто включает их и т.д.
            while (stack.Count > 0) {
                var current = stack.Pop();
                string currentFileName = Path.GetFileName(current);

                foreach (var kvp in this._includesMap) {
                    if (kvp.Value.Contains(currentFileName, StringComparer.OrdinalIgnoreCase)) {
                        if (result.Add(kvp.Key)) {
                            stack.Push(kvp.Key);
                        }
                    }
                }
            }

            return result.ToList();
        }

        public static List<string> ExtractRawIncludes(string filePath, int maxLinesToRead = 10) {
            var result = new List<string>();

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
                    string include = trimmed.Substring(start + 1, end - start - 1).Trim();
                    string normalized = Path.GetFileName(include);

                    if (!string.IsNullOrWhiteSpace(normalized)) {
                        result.Add(normalized);
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


        /// <summary>
        /// Итеративно получить все проекты решения (включая из Solution Folder)
        /// </summary>
        private static IEnumerable<EnvDTE.Project> GetAllProjects(EnvDTE80.DTE2 dte) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var queue = new Queue<EnvDTE.Project>();
            foreach (EnvDTE.Project p in dte.Solution.Projects)
                queue.Enqueue(p);

            while (queue.Count > 0) {
                var proj = queue.Dequeue();

                if (proj.Kind == EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder) {
                    foreach (EnvDTE.ProjectItem item in proj.ProjectItems) {
                        if (item.SubProject != null)
                            queue.Enqueue(item.SubProject);
                    }
                }
                else {
                    yield return proj;
                }
            }
        }
    }
}