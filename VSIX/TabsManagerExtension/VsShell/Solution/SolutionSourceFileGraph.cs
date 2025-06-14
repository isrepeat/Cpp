using System;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.Build.Locator;
using Microsoft.Build.Evaluation;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.Package;


namespace TabsManagerExtension.VsShell.Solution {
    /// <summary>
    /// Хранит граф включений исходных файлов (SourceFile) и их обратные зависимости.
    /// <br/> Предоставляет:
    /// <br/> 1. Быстрый доступ по абсолютному пути к файлу;
    /// <br/> 2. Прямой список всех SourceFile;
    /// <br/> 3. Обратную карту зависимостей: кто включает какой файл (reverse include map).
    /// </summary>
    public class SolutionSourceFileGraph {
        public IEnumerable<SourceFile> AllSourceFiles => _sourceFileIncludesMap.Keys;

        private readonly Dictionary<SourceFile, List<ResolvedIncludeEntry>> _sourceFileIncludesMap = new();
        private readonly Dictionary<IncludeEntry, List<SourceFile>> _includeFileSourcesMap = new();
        private readonly Dictionary<string, List<SourceFile>> _sourceFileRepresentationsMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, List<SourceFile>> _resolvedIncludeSourcesMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly MsBuildSolutionWatcher _msBuildSolutionWatcher;

        public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
            _msBuildSolutionWatcher = msBuildSolutionWatcher;
        }

        public void AddWithIncludes(SourceFile file, List<IncludeEntry> inclideEntries) {
            if (!_sourceFileIncludesMap.ContainsKey(file)) {
                if (!_sourceFileRepresentationsMap.TryGetValue(file.FilePath, out var list)) {
                    list = new List<SourceFile>();
                    _sourceFileRepresentationsMap[file.FilePath] = list;
                }

                list.Add(file);
            }

            var resolvedIncludes = this.ResolveIncludesAndUpdateMaps(file, inclideEntries);
            _sourceFileIncludesMap[file] = resolvedIncludes;
        }


        public void UpdateSourceFile(SourceFile file, List<IncludeEntry> newIncludeEntries) {
            if (_sourceFileIncludesMap.TryGetValue(file, out var oldResolvedIncludes)) {
                foreach (var resolvedInclude in oldResolvedIncludes) {
                    var raw = resolvedInclude.IncludeEntry;

                    if (_includeFileSourcesMap.TryGetValue(raw, out var sources)) {
                        sources.Remove(file);
                        if (sources.Count == 0) {
                            _includeFileSourcesMap.Remove(raw);
                        }
                    }

                    if (resolvedInclude.ResolvedPath is string path && _resolvedIncludeSourcesMap.TryGetValue(path, out var resolvedList)) {
                        resolvedList.Remove(file);
                        if (resolvedList.Count == 0) {
                            _resolvedIncludeSourcesMap.Remove(path);
                        }
                    }
                }
            }

            var resolvedIncludes = this.ResolveIncludesAndUpdateMaps(file, newIncludeEntries);
            _sourceFileIncludesMap[file] = resolvedIncludes;

            if (!_sourceFileRepresentationsMap.TryGetValue(file.FilePath, out var reps)) {
                reps = new List<SourceFile>();
                _sourceFileRepresentationsMap[file.FilePath] = reps;
            }

            reps.RemoveAll(f => StringComparer.OrdinalIgnoreCase.Equals(
                f.Project.ShellProject.Project.UniqueName,
                file.Project.ShellProject.Project.UniqueName));

            reps.Add(file);
        }


        private List<ResolvedIncludeEntry> ResolveIncludesAndUpdateMaps(SourceFile file, List<IncludeEntry> includeEntries) {
            var resolvedIncludes = new List<ResolvedIncludeEntry>();

            foreach (var includeEntry in includeEntries) {
                var resolvedPath = Service.IncludeResolverService.TryResolveInclude(
                    includeEntry.RawInclude,
                    file.FilePath,
                    file.Project,
                    _msBuildSolutionWatcher
                );

                var resolved = new ResolvedIncludeEntry(includeEntry, resolvedPath);
                resolvedIncludes.Add(resolved);

                // Добавляем в _includeFileSourcesMap
                if (!_includeFileSourcesMap.TryGetValue(includeEntry, out var sourcesList)) {
                    sourcesList = new List<SourceFile>();
                    _includeFileSourcesMap[includeEntry] = sourcesList;
                }

                if (!sourcesList.Contains(file)) {
                    sourcesList.Add(file);
                }

                // Добавляем в _resolvedIncludeSourcesMap
                if (resolvedPath != null) {
                    if (!_resolvedIncludeSourcesMap.TryGetValue(resolvedPath, out var resolvedList)) {
                        resolvedList = new List<SourceFile>();
                        _resolvedIncludeSourcesMap[resolvedPath] = resolvedList;
                    }

                    if (!resolvedList.Contains(file)) {
                        resolvedList.Add(file);
                    }
                }
            }

            return resolvedIncludes;
        }





        public IEnumerable<ResolvedIncludeEntry> GetResolvedIncludesFor(SourceFile file) {
            if (_sourceFileIncludesMap.TryGetValue(file, out var list)) {
                return list;
            }

            return Enumerable.Empty<ResolvedIncludeEntry>();
        }

        public IEnumerable<SourceFile> GetSourceFilesWhoInclude(IncludeEntry includeEntry) {
            if (_includeFileSourcesMap.TryGetValue(includeEntry, out var list)) {
                return list;
            }

            return Enumerable.Empty<SourceFile>();
        }

        public IEnumerable<SourceFile> GetSourceFilesWhoIncludeResolved(string resolvedPath) {
            if (_resolvedIncludeSourcesMap.TryGetValue(resolvedPath, out var list)) {
                return list;
            }

            return Enumerable.Empty<SourceFile>();
        }


        public bool TryGetSourceFileRepresentationForPathAndProject(string filePath, string projectUniqueName, out SourceFile? result) {
            if (this.TryGetSourceFileRepresentationsForPath(filePath, out var sources)) {
                foreach (var s in sources) {
                    if (StringComparer.OrdinalIgnoreCase.Equals(s.Project.ShellProject.Project.UniqueName, projectUniqueName)) {
                        result = s;
                        return true;
                    }
                }
            }

            result = null;
            return false;
        }

        public bool TryGetSourceFileRepresentationsForPath(string filePath, out IReadOnlyList<SourceFile> sources) {
            if (_sourceFileRepresentationsMap.TryGetValue(filePath, out var list)) {
                sources = list;
                return true;
            }

            sources = Array.Empty<SourceFile>();
            return false;
        }

        public void Clear() {
            _sourceFileIncludesMap.Clear();
            _includeFileSourcesMap.Clear();
            _sourceFileRepresentationsMap.Clear();
        }
    }





    //public class SolutionSourceFileGraph {
    //    private readonly HashSet<SourceFile> _allSourceFiles = new();
    //    public IEnumerable<SourceFile> AllSourceFiles => _allSourceFiles;

    //    /// <summary>
    //    /// Хранит все представления файла (SourceFile) по его абсолютному пути.
    //    /// <br/> Один путь может соответствовать нескольким проектам.
    //    /// <br/> Используется для разрешения include-зависимостей и обработки изменений файлов.
    //    /// </summary>
    //    private readonly Dictionary<string, List<SourceFile>> _sourceFileRepresentationsMap = new(StringComparer.OrdinalIgnoreCase);

    //    /// <summary>
    //    /// Обратная карта зависимостей:
    //    /// ключ — файл, который включается;
    //    /// значение — список файлов, которые его включают.
    //    /// </summary>
    //    private readonly Dictionary<SourceFile, HashSet<SourceFile>> _includedFileToIncluders = new();

    //    private MsBuildSolutionWatcher _msBuildSolutionWatcher;

    //    public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
    //        _msBuildSolutionWatcher = msBuildSolutionWatcher;
    //    }

    //    public void Add(SourceFile file) {
    //        if (_allSourceFiles.Add(file)) {
    //            if (!_sourceFileRepresentationsMap.TryGetValue(file.FilePath, out var list)) {
    //                list = new List<SourceFile>();
    //                _sourceFileRepresentationsMap[file.FilePath] = list;
    //            }

    //            list.Add(file);
    //        }
    //    }


    //    /// <summary>
    //    /// Автоматически пересчитывает карту зависимостей включения на основе текущих SourceFile.
    //    /// </summary>
    //    public void BuildReverseMap() {
    //        _includedFileToIncluders.Clear();

    //        foreach (var includer in _allSourceFiles) {
    //            foreach (var include in includer.Includes) {
    //                var resolvedPath = Service.IncludeResolverService.TryResolveInclude(
    //                    include.RawInclude,
    //                    includer.FilePath,
    //                    includer.Project,
    //                    _msBuildSolutionWatcher
    //                );

    //                if (resolvedPath != null && this.TryGetSourceFileRepresentationForPathAndProject(resolvedPath, includer.Project.ShellProject.Project.UniqueName, out var includedFile)) {
    //                    if (!_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
    //                        includers = new HashSet<SourceFile>();
    //                        _includedFileToIncluders[includedFile] = includers;
    //                    }

    //                    includers.Add(includer);
    //                }
    //            }
    //        }
    //    }



    //    public void UpdateSourceFile(SourceFile updatedFile) {
    //        // Обновляем индекс
    //        if (!_sourceFileRepresentationsMap.TryGetValue(updatedFile.FilePath, out var list)) {
    //            list = new List<SourceFile>();
    //            _sourceFileRepresentationsMap[updatedFile.FilePath] = list;
    //        }

    //        list.RemoveAll(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.Project.ShellProject.Project.UniqueName, updatedFile.Project.ShellProject.Project.UniqueName));
    //        list.Add(updatedFile);

    //        _allSourceFiles.Remove(updatedFile);
    //        _allSourceFiles.Add(updatedFile);

    //        // Удаляем старые зависимости этого файла
    //        foreach (var entry in _includedFileToIncluders.Values) {
    //            entry.Remove(updatedFile);
    //        }

    //        // Регистрируем новые зависимости
    //        foreach (var include in updatedFile.Includes) {
    //            var resolvedPath = Service.IncludeResolverService.TryResolveInclude(
    //                include.RawInclude,
    //                updatedFile.FilePath,
    //                updatedFile.Project,
    //                _msBuildSolutionWatcher
    //            );

    //            if (resolvedPath != null && this.TryGetSourceFileRepresentationForPathAndProject(resolvedPath, updatedFile.Project.ShellProject.Project.UniqueName, out var includedFile)) {
    //                if (!_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
    //                    includers = new HashSet<SourceFile>();
    //                    _includedFileToIncluders[includedFile] = includers;
    //                }

    //                includers.Add(updatedFile);
    //            }
    //        }
    //    }




    //    public bool TryGetSourceFileRepresentationForPathAndProject(string filePath, string projectUniqueName, out SourceFile? result) {
    //        if (TryGetSourceFileRepresentationsForPath(filePath, out var sources)) {
    //            foreach (var s in sources) {
    //                if (StringComparer.OrdinalIgnoreCase.Equals(s.Project.ShellProject.Project.UniqueName, projectUniqueName)) {
    //                    result = s;
    //                    return true;
    //                }
    //            }
    //        }

    //        result = null;
    //        return false;
    //    }

    //    public bool TryGetSourceFileRepresentationsForPath(string filePath, out IReadOnlyList<SourceFile> sources) {
    //        if (_sourceFileRepresentationsMap.TryGetValue(filePath, out var list)) {
    //            sources = list;
    //            return true;
    //        }

    //        sources = Array.Empty<SourceFile>();
    //        return false;
    //    }


    //    /// <summary>
    //    /// Возвращает все файлы, которые делают #include данного файла.
    //    /// </summary>
    //    public IEnumerable<SourceFile> GetSourceFilesWhoInclude(SourceFile includedFile) {
    //        if (_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
    //            return includers;
    //        }

    //        return Enumerable.Empty<SourceFile>();
    //    }


    //    public void Clear() {
    //        _allSourceFiles.Clear();
    //        _sourceFileRepresentationsMap.Clear();
    //        _includedFileToIncluders.Clear();
    //    }
    //}
}