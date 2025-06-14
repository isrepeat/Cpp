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
        public IEnumerable<SourceFile> AllSourceFiles => _sourceFileToResolvedIncludeEntriesMap.Keys;

        private readonly Dictionary<SourceFile, List<IncludeEntry>> _sourceFileToIncludeEntriesMap = new();
        private readonly Dictionary<IncludeEntry, List<SourceFile>> _includeEntryToSourceFilesMap = new();
        private readonly Dictionary<SourceFile, List<ResolvedIncludeEntry>> _sourceFileToResolvedIncludeEntriesMap = new();
        private readonly Dictionary<ResolvedIncludeEntry, List<SourceFile>> _resolvedIncludeEntryToSourceFilesMap = new();
        private readonly Dictionary<string, List<SourceFile>> _resolvedIncludePathsToSourceFilesMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, List<SourceFile>> _sourceFileRepresentationsMap = new(StringComparer.OrdinalIgnoreCase);

        private readonly MsBuildSolutionWatcher _msBuildSolutionWatcher;

        public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
            this._msBuildSolutionWatcher = msBuildSolutionWatcher;
        }

        public void AddSourceFileWithIncludes(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
            this.ApplyIncludes(sourceFile, includeEntries);
        }

        public void UpdateSourceFileWithIncludes(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
            this.RemoveOldIndexes(sourceFile);
            this.ApplyIncludes(sourceFile, includeEntries);
        }

        private void ApplyIncludes(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
            // ① Запоминаем сырой список IncludeEntry
            this._sourceFileToIncludeEntriesMap[sourceFile] = includeEntries;

            var resolvedIncludes = new List<ResolvedIncludeEntry>();

            foreach (var includeEntry in includeEntries) {
                // ② Пытаемся резолвить путь
                string? resolvedPath = Service.IncludeResolverService.TryResolveInclude(
                    includeEntry.RawInclude,
                    sourceFile.FilePath,
                    sourceFile.Project,
                    this._msBuildSolutionWatcher
                );

                var resolved = new ResolvedIncludeEntry(includeEntry, resolvedPath);
                resolvedIncludes.Add(resolved);

                // ③ Обновляем словарь: IncludeEntry → SourceFile[]
                if (!this._includeEntryToSourceFilesMap.TryGetValue(includeEntry, out var rawList)) {
                    rawList = new List<SourceFile>();
                    this._includeEntryToSourceFilesMap[includeEntry] = rawList;
                }
                if (!rawList.Contains(sourceFile)) {
                    rawList.Add(sourceFile);
                }

                // ④ Обновляем словарь: ResolvedIncludeEntry → SourceFile[]
                if (!this._resolvedIncludeEntryToSourceFilesMap.TryGetValue(resolved, out var resolvedList)) {
                    resolvedList = new List<SourceFile>();
                    this._resolvedIncludeEntryToSourceFilesMap[resolved] = resolvedList;
                }
                if (!resolvedList.Contains(sourceFile)) {
                    resolvedList.Add(sourceFile);
                }

                // ⑤ Обновляем: ResolvedPath (если есть) → SourceFile[]
                if (resolvedPath is not null) {
                    if (!this._resolvedIncludePathsToSourceFilesMap.TryGetValue(resolvedPath, out var pathList)) {
                        pathList = new List<SourceFile>();
                        this._resolvedIncludePathsToSourceFilesMap[resolvedPath] = pathList;
                    }
                    if (!pathList.Contains(sourceFile)) {
                        pathList.Add(sourceFile);
                    }
                }
            }

            // ⑥ Запоминаем список ResolvedIncludeEntry для SourceFile
            this._sourceFileToResolvedIncludeEntriesMap[sourceFile] = resolvedIncludes;

            // ⑦ Обновляем представления файлов
            if (!this._sourceFileRepresentationsMap.TryGetValue(sourceFile.FilePath, out var reps)) {
                reps = new List<SourceFile>();
                this._sourceFileRepresentationsMap[sourceFile.FilePath] = reps;
            }

            reps.RemoveAll(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.ProjectId, sourceFile.ProjectId));
            reps.Add(sourceFile);
        }




        private void RemoveOldIndexes(SourceFile sourceFile) {
            if (!this._sourceFileToResolvedIncludeEntriesMap.TryGetValue(sourceFile, out var oldResolved)) {
                return;
            }

            foreach (var resolved in oldResolved) {
                var raw = resolved.IncludeEntry;

                // ① Remove from IncludeEntry → SourceFile[]
                if (this._includeEntryToSourceFilesMap.TryGetValue(raw, out var rawList)) {
                    rawList.Remove(sourceFile);
                    if (rawList.Count == 0) {
                        this._includeEntryToSourceFilesMap.Remove(raw);
                    }
                }

                // ② Remove from ResolvedIncludeEntry → SourceFile[]
                if (this._resolvedIncludeEntryToSourceFilesMap.TryGetValue(resolved, out var resolvedList)) {
                    resolvedList.Remove(sourceFile);
                    if (resolvedList.Count == 0) {
                        this._resolvedIncludeEntryToSourceFilesMap.Remove(resolved);
                    }
                }

                // ③ Remove from ResolvedPath (если он есть) → SourceFile[]
                if (resolved.ResolvedPath is string path &&
                    this._resolvedIncludePathsToSourceFilesMap.TryGetValue(path, out var pathList)) {
                    pathList.Remove(sourceFile);
                    if (pathList.Count == 0) {
                        this._resolvedIncludePathsToSourceFilesMap.Remove(path);
                    }
                }
            }

            // ④ Удаляем SourceFile → ResolvedIncludeEntry[]
            this._sourceFileToResolvedIncludeEntriesMap.Remove(sourceFile);

            // ⑤ Удаляем SourceFile → IncludeEntry[]
            this._sourceFileToIncludeEntriesMap.Remove(sourceFile);
        }





        public void Clear() {
            this._sourceFileToIncludeEntriesMap.Clear();
            this._includeEntryToSourceFilesMap.Clear();
            this._sourceFileToResolvedIncludeEntriesMap.Clear();
            this._resolvedIncludeEntryToSourceFilesMap.Clear();
            this._resolvedIncludePathsToSourceFilesMap.Clear();
            this._sourceFileRepresentationsMap.Clear();
        }

        public IEnumerable<IncludeEntry> GetRawIncludes(SourceFile file) {
            if (this._sourceFileToIncludeEntriesMap.TryGetValue(file, out var list)) {
                return list;
            }

            return Array.Empty<IncludeEntry>();
        }

        public IEnumerable<ResolvedIncludeEntry> GetResolvedIncludes(SourceFile file) {
            if (this._sourceFileToResolvedIncludeEntriesMap.TryGetValue(file, out var list)) {
                return list;
            }

            return Array.Empty<ResolvedIncludeEntry>();
        }

        public IEnumerable<SourceFile> GetSourceFilesByInclude(IncludeEntry entry) {
            if (this._includeEntryToSourceFilesMap.TryGetValue(entry, out var list)) {
                return list;
            }

            return Array.Empty<SourceFile>();
        }

        public IEnumerable<SourceFile> GetSourceFilesByResolved(ResolvedIncludeEntry entry) {
            if (this._resolvedIncludeEntryToSourceFilesMap.TryGetValue(entry, out var list)) {
                return list;
            }

            return Array.Empty<SourceFile>();
        }

        public IEnumerable<SourceFile> GetSourceFilesByResolvedPath(string resolvedPath) {
            if (this._resolvedIncludePathsToSourceFilesMap.TryGetValue(resolvedPath, out var list)) {
                return list;
            }

            return Array.Empty<SourceFile>();
        }

        public bool TryGetSourceFileRepresentations(string filePath, out IReadOnlyList<SourceFile> result) {
            if (this._sourceFileRepresentationsMap.TryGetValue(filePath, out var list)) {
                result = list;
                return true;
            }

            result = Array.Empty<SourceFile>();
            return false;
        }
    }



//    public class SolutionSourceFileGraph {
//        public IEnumerable<SourceFile> AllSourceFiles => _sourceFileToResolvedIncludesMap.Keys;

//        private readonly Dictionary<SourceFile, List<ResolvedIncludeEntry>> _sourceFileToResolvedIncludesMap = new();
//        private readonly Dictionary<IncludeEntry, List<SourceFile>> _includeEntryToSourceFilesMap = new();

//        private readonly Dictionary<string, List<SourceFile>> _sourceFileRepresentationsMap = new(StringComparer.OrdinalIgnoreCase);
//        private readonly Dictionary<string, List<SourceFile>> _resolvedIncludeSourcesMap = new(StringComparer.OrdinalIgnoreCase);

//        private readonly MsBuildSolutionWatcher _msBuildSolutionWatcher;

//        public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
//            _msBuildSolutionWatcher = msBuildSolutionWatcher;
//        }

//        public void AddSourceFileWithIncludes(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
//            var resolvedIncludes = this.ResolveIncludesAndUpdateMaps(sourceFile, includeEntries);
//            this.ApplyResolvedIncludes(sourceFile, resolvedIncludes);
//        }

//        public void UpdateSourceFileWithIncludes(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
//            this.RemoveOldIndexes(sourceFile);

//            var resolvedIncludes = this.ResolveIncludesAndUpdateMaps(sourceFile, includeEntries);
//            this.ApplyResolvedIncludes(sourceFile, resolvedIncludes);
//        }



//        private List<ResolvedIncludeEntry> ResolveIncludesAndUpdateMaps(SourceFile sourceFile, List<IncludeEntry> includeEntries) {
//            var resolvedIncludes = new List<ResolvedIncludeEntry>();

//            foreach (var includeEntry in includeEntries) {
//                var resolvedPath = Service.IncludeResolverService.TryResolveInclude(
//                    includeEntry.RawInclude,
//                    sourceFile.FilePath,
//                    sourceFile.Project,
//                    _msBuildSolutionWatcher
//                );

//                var resolved = new ResolvedIncludeEntry(includeEntry, resolvedPath);
//                resolvedIncludes.Add(resolved);

//                // Добавляем в _includeEntryToSourceFilesMap
//                if (!_includeEntryToSourceFilesMap.TryGetValue(includeEntry, out var sourceFilesList)) {
//                    sourceFilesList = new List<SourceFile>();
//                    _includeEntryToSourceFilesMap[includeEntry] = sourceFilesList;
//                }

//                if (!sourceFilesList.Contains(sourceFile)) {
//                    sourceFilesList.Add(sourceFile);
//                }

//                // Добавляем в _resolvedIncludeSourcesMap
//                if (resolvedPath != null) {
//                    if (!_resolvedIncludeSourcesMap.TryGetValue(resolvedPath, out var resolvedSourceFilesList)) {
//                        resolvedSourceFilesList = new List<SourceFile>();
//                        _resolvedIncludeSourcesMap[resolvedPath] = resolvedSourceFilesList;
//                    }

//                    if (!resolvedSourceFilesList.Contains(sourceFile)) {
//                        resolvedSourceFilesList.Add(sourceFile);
//                    }
//                }
//            }

//            return resolvedIncludes;
//        }


//        private void ApplyResolvedIncludes(SourceFile sourceFile, List<ResolvedIncludeEntry> resolvedIncludes) {
//            _sourceFileToResolvedIncludesMap[sourceFile] = resolvedIncludes;

//            if (!_sourceFileRepresentationsMap.TryGetValue(sourceFile.FilePath, out var sourceFileRepresentations)) {
//                sourceFileRepresentations = new List<SourceFile>();
//                _sourceFileRepresentationsMap[sourceFile.FilePath] = sourceFileRepresentations;
//            }

//            sourceFileRepresentations.RemoveAll(sourceFile =>
//                StringComparer.OrdinalIgnoreCase.Equals(
//                    sourceFile.Project.ShellProject.Project.UniqueName,
//                    sourceFile.Project.ShellProject.Project.UniqueName));

//            sourceFileRepresentations.Add(sourceFile);
//        }


//        private void RemoveOldIndexes(SourceFile sourceFile) {
//            if (!_sourceFileToResolvedIncludesMap.TryGetValue(sourceFile, out var oldResolvedIncludes)) {
//                return;
//            }

//            foreach (var resolvedInclude in oldResolvedIncludes) {
//                var raw = resolvedInclude.IncludeEntry;

//                if (_includeEntryToSourceFilesMap.TryGetValue(raw, out var sources)) {
//                    sources.Remove(sourceFile);
//                    if (sources.Count == 0) {
//                        _includeEntryToSourceFilesMap.Remove(raw);
//                    }
//                }

//                if (resolvedInclude.ResolvedPath is string path &&
//                    _resolvedIncludeSourcesMap.TryGetValue(path, out var resolvedList)) {
//                    resolvedList.Remove(sourceFile);
//                    if (resolvedList.Count == 0) {
//                        _resolvedIncludeSourcesMap.Remove(path);
//                    }
//                }
//            }
//        }


//        public IEnumerable<ResolvedIncludeEntry> GetResolvedIncludesFor(SourceFile sourceFile) {
//            if (_sourceFileToResolvedIncludesMap.TryGetValue(sourceFile, out var list)) {
//                return list;
//            }

//            return Enumerable.Empty<ResolvedIncludeEntry>();
//        }

//        public IEnumerable<SourceFile> GetSourceFilesWhoInclude(IncludeEntry includeEntry) {
//            if (_includeEntryToSourceFilesMap.TryGetValue(includeEntry, out var list)) {
//                return list;
//            }

//            return Enumerable.Empty<SourceFile>();
//        }

//        public IEnumerable<SourceFile> GetSourceFilesWhoIncludeResolved(string resolvedPath) {
//            if (_resolvedIncludeSourcesMap.TryGetValue(resolvedPath, out var list)) {
//                return list;
//            }

//            return Enumerable.Empty<SourceFile>();
//        }


//        public bool TryGetSourceFileRepresentationsForPath(string filePath, out IReadOnlyList<SourceFile> sources) {
//            if (_sourceFileRepresentationsMap.TryGetValue(filePath, out var list)) {
//                sources = list;
//                return true;
//            }

//            sources = Array.Empty<SourceFile>();
//            return false;
//        }

//        public void Clear() {
//            _sourceFileToResolvedIncludesMap.Clear();
//            _includeEntryToSourceFilesMap.Clear();
//            _sourceFileRepresentationsMap.Clear();
//        }
//    }
}


/*
TestIncludeSolution/
├── Game/
│   ├── Game.cpp                 // #include "Logger.h", "Config.h", "Missing.h"
│   ├── GameShared.cpp          // #include "../Helpers.Shared/SharedUtils.h"
│   └── LocalIncludes/Logger.h  // ← shadow Logger.h
├── Editor/
│   ├── Editor.cpp              // #include "../Helpers.Shared/Logger.h"
│   └── GameLink.cpp            // #include "Logger.h"
├── Engine/
│   ├── Engine.cpp              // #include "SharedUtils.h"
│   └── Nested/Inner.cpp       // #include "../../Helpers.Shared/Logger.h"
└── Helpers.Shared/
    ├── Config.h
    ├── Logger.h
    └── SharedUtils.h


1. Dictionary<SourceFile, List<IncludeEntry>> [_sourceFileToIncludeEntiresMap]
{
    Game.cpp       => [ "Logger.h", "Config.h", "Missing.h" ],
    GameShared.cpp => [ "../Helpers.Shared/SharedUtils.h" ],
    Editor.cpp     => [ "../Helpers.Shared/Logger.h" ],
    GameLink.cpp   => [ "Logger.h" ],
    Engine.cpp     => [ "SharedUtils.h" ],
    Inner.cpp      => [ "../../Helpers.Shared/Logger.h" ]
}

2. Dictionary<IncludeEntry, List<SourceFile>> [_includeEntiresSourceFilesMap]
{
    "Logger.h"                          => [ Game.cpp, GameLink.cpp ],
    "Config.h"                          => [ Game.cpp ],
    "Missing.h"                         => [ Game.cpp ],
    "../Helpers.Shared/SharedUtils.h"  => [ GameShared.cpp ],
    "../Helpers.Shared/Logger.h"       => [ Editor.cpp ],
    "SharedUtils.h"                     => [ Engine.cpp ],
    "../../Helpers.Shared/Logger.h"    => [ Inner.cpp ]
}

3. Dictionary<SourceFile, List<ResolvedIncludeEntry>> [_sourceFileToResolvedIncludeEntriesMap]
{
    Game.cpp => [
        ("Logger.h" → "Game/LocalIncludes/Logger.h"),
        ("Config.h" → "Helpers.Shared/Config.h"),
        ("Missing.h" → null)
    ],
    GameShared.cpp => [
        ("../Helpers.Shared/SharedUtils.h" → "Helpers.Shared/SharedUtils.h")
    ],
    Editor.cpp => [
        ("../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")
    ],
    GameLink.cpp => [
        ("Logger.h" → "Helpers.Shared/Logger.h")
    ],
    Engine.cpp => [
        ("SharedUtils.h" → "Helpers.Shared/SharedUtils.h")
    ],
    Inner.cpp => [
        ("../../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")
    ]
}

4. Dictionary<ResolvedIncludeEntry, List<SourceFile>> [_resolvedIncludeEntriesToSourceFilesMap]
{
    ("Logger.h" → "Game/LocalIncludes/Logger.h")                      => [ Game.cpp ],
    ("Logger.h" → "Helpers.Shared/Logger.h")                          => [ GameLink.cpp ],
    ("../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")       => [ Editor.cpp ],
    ("../../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")    => [ Inner.cpp ],
    ("Config.h" → "Helpers.Shared/Config.h")                          => [ Game.cpp ],
    ("SharedUtils.h" → "Helpers.Shared/SharedUtils.h")               => [ Engine.cpp ],
    ("../Helpers.Shared/SharedUtils.h" → "Helpers.Shared/SharedUtils.h") => [ GameShared.cpp ],
    ("Missing.h" → null)                                              => [ Game.cpp ]
}

5. Dictionary<string, List<SourceFile>> [_resolvedIncludePathsToSourceFilesMap]
{
    "Game/LocalIncludes/Logger.h"     => [ Game.cpp ],
    "Helpers.Shared/Logger.h"         => [ Editor.cpp, GameLink.cpp, Inner.cpp ],
    "Helpers.Shared/Config.h"         => [ Game.cpp ],
    "Helpers.Shared/SharedUtils.h"    => [ Engine.cpp, GameShared.cpp ]
}

6. Dictionary<string, List<SourceFile>> [_sourceFileRepresentationsMap]
{
    "Helpers.Shared/Logger.h" => [
        SourceFile { ..., Project = Editor },
        SourceFile { ..., Project = GameLink },
        SourceFile { ..., Project = Engine }
    ],
    "Helpers.Shared/Config.h" => [
        SourceFile { ..., Project = Game }
    ],
    "Helpers.Shared/SharedUtils.h" => [
        SourceFile { ..., Project = Game },
        SourceFile { ..., Project = Engine }
    ],
    "Game/LocalIncludes/Logger.h" => [
        SourceFile { ..., Project = Game }
    ]
}
 */