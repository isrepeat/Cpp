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
        public IEnumerable<Document.SourceFile> AllSourceFiles => _sourceFileToResolvedIncludeEntriesMap.Keys;

        private readonly Dictionary<Document.SourceFile, List<Document.IncludeEntry>> _sourceFileToIncludeEntriesMap = new();
        private readonly Dictionary<Document.IncludeEntry, List<Document.SourceFile>> _includeEntryToSourceFilesMap = new();
        private readonly Dictionary<Document.SourceFile, List<Document.ResolvedIncludeEntry>> _sourceFileToResolvedIncludeEntriesMap = new();
        private readonly Dictionary<Document.ResolvedIncludeEntry, List<Document.SourceFile>> _resolvedIncludeEntryToSourceFilesMap = new();
        private readonly Dictionary<string, List<Document.SourceFile>> _resolvedIncludePathsToSourceFilesMap = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, List<Document.SourceFile>> _sourceFileRepresentationsMap = new(StringComparer.OrdinalIgnoreCase);

        private readonly MsBuildSolutionWatcher _msBuildSolutionWatcher;

        public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
            _msBuildSolutionWatcher = msBuildSolutionWatcher;
        }

        public void AddSourceFileWithIncludes(Document.SourceFile sourceFile, List<Document.IncludeEntry> includeEntries) {
            this.ApplyIncludes(sourceFile, includeEntries);
        }

        public void UpdateSourceFileWithIncludes(Document.SourceFile sourceFile, List<Document.IncludeEntry> includeEntries) {
            this.RemoveSourceFileInternal(sourceFile);
            this.ApplyIncludes(sourceFile, includeEntries);
        }

        public void RemoveSourceFile(Document.SourceFile sourceFile) {
            this.RemoveSourceFileInternal(sourceFile);
        }


        public IEnumerable<Document.IncludeEntry> GetRawIncludes(Document.SourceFile file) {
            if (_sourceFileToIncludeEntriesMap.TryGetValue(file, out var list)) {
                return list;
            }

            return Array.Empty<Document.IncludeEntry>();
        }

        public IEnumerable<KeyValuePair<Document.SourceFile, List<Document.ResolvedIncludeEntry>>> GetAllResolvedIncludeEntries() {
            return _sourceFileToResolvedIncludeEntriesMap;
        }

        public IEnumerable<Document.ResolvedIncludeEntry> GetResolvedIncludes(Document.SourceFile file) {
            if (_sourceFileToResolvedIncludeEntriesMap.TryGetValue(file, out var list)) {
                return list;
            }

            return Array.Empty<Document.ResolvedIncludeEntry>();
        }


        public IEnumerable<Document.SourceFile> GetSourceFilesByInclude(Document.IncludeEntry entry) {
            if (_includeEntryToSourceFilesMap.TryGetValue(entry, out var list)) {
                return list;
            }

            return Array.Empty<Document.SourceFile>();
        }

        public IEnumerable<Document.SourceFile> GetSourceFilesByResolved(Document.ResolvedIncludeEntry entry) {
            if (_resolvedIncludeEntryToSourceFilesMap.TryGetValue(entry, out var list)) {
                return list;
            }

            return Array.Empty<Document.SourceFile>();
        }

        public IEnumerable<Document.SourceFile> GetSourceFilesByResolvedPath(string resolvedPath) {
            if (_resolvedIncludePathsToSourceFilesMap.TryGetValue(resolvedPath, out var list)) {
                return list;
            }

            return Array.Empty<Document.SourceFile>();
        }


        public bool TryGetSourceFileRepresentations(string filePath, out IReadOnlyList<Document.SourceFile> result) {
            if (_sourceFileRepresentationsMap.TryGetValue(filePath, out var list)) {
                result = list;
                return true;
            }

            result = Array.Empty<Document.SourceFile>();
            return false;
        }

        public void Clear() {
            _sourceFileToIncludeEntriesMap.Clear();
            _includeEntryToSourceFilesMap.Clear();
            _sourceFileToResolvedIncludeEntriesMap.Clear();
            _resolvedIncludeEntryToSourceFilesMap.Clear();
            _resolvedIncludePathsToSourceFilesMap.Clear();
            _sourceFileRepresentationsMap.Clear();
        }


        private void ApplyIncludes(Document.SourceFile sourceFile, List<Document.IncludeEntry> includeEntries) {
            if (_sourceFileToIncludeEntriesMap.TryGetValue(sourceFile, out var sourcesIncludesList)) {
                Helpers.Diagnostic.Logger.LogDebug($"[SolutionSourceFileGraph.ApplyIncludes] sourceFile already added: {sourceFile}");
                return;
            }

            // ① Запоминаем сырой список IncludeEntry (то есть просто строки #include с их видами)
            // Это потом используется для получения «сырых» зависимостей без резолвинга путей
            _sourceFileToIncludeEntriesMap[sourceFile] = includeEntries;

            var resolvedIncludes = new List<Document.ResolvedIncludeEntry>();

            foreach (var includeEntry in includeEntries) {
                // ② Резолвим путь (т.е. превращаем #include "Logger.h" в абсолютный/относительный путь если смогли)
                string? resolvedPath = Services.IncludeResolverService.TryResolveInclude(
                    includeEntry.RawInclude,
                    sourceFile.FilePath,
                    sourceFile.LoadedProject,
                    _msBuildSolutionWatcher
                );

                var resolvedInclude = new Document.ResolvedIncludeEntry(includeEntry, resolvedPath);
                resolvedIncludes.Add(resolvedInclude);

                // ③ Строим обратную карту для сырых include'ов:
                // Dictionary<IncludeEntry, List<Document.SourceFile>>
                // Чтобы быстро узнать, какие файлы включают "Logger.h"
                if (!_includeEntryToSourceFilesMap.TryGetValue(includeEntry, out var rawSourcesList)) {
                    rawSourcesList = new List<Document.SourceFile>();
                    _includeEntryToSourceFilesMap[includeEntry] = rawSourcesList;
                }
                if (!rawSourcesList.Contains(sourceFile)) {
                    rawSourcesList.Add(sourceFile);
                }

                // ④ Строим обратную карту для ResolvedIncludeEntry:
                // Dictionary<ResolvedIncludeEntry, List<Document.SourceFile>>
                // Чтобы быстро узнать, какие файлы включают "Logger.h" → "Helpers.Shared/Logger.h"
                if (!_resolvedIncludeEntryToSourceFilesMap.TryGetValue(resolvedInclude, out var resolvedIncludesSourcesList)) {
                    resolvedIncludesSourcesList = new List<Document.SourceFile>();
                    _resolvedIncludeEntryToSourceFilesMap[resolvedInclude] = resolvedIncludesSourcesList;
                }
                if (!resolvedIncludesSourcesList.Contains(sourceFile)) {
                    resolvedIncludesSourcesList.Add(sourceFile);
                }

                // ⑤ Также индексируем по строковому пути (если удалось его получить)
                // Dictionary<string, List<Document.SourceFile>> - по resolvedPath
                if (resolvedPath is not null) {
                    if (!_resolvedIncludePathsToSourceFilesMap.TryGetValue(resolvedPath, out var resolvedIncludePathsSourcesList)) {
                        resolvedIncludePathsSourcesList = new List<Document.SourceFile>();
                        _resolvedIncludePathsToSourceFilesMap[resolvedPath] = resolvedIncludePathsSourcesList;
                    }
                    if (!resolvedIncludePathsSourcesList.Contains(sourceFile)) {
                        resolvedIncludePathsSourcesList.Add(sourceFile);
                    }
                }
            }

            // ⑥ Запоминаем resolved-инклюды для этого sourceFile,
            // чтобы потом можно было быстро очистить индексы при повторном добавлении или удалении файла
            _sourceFileToResolvedIncludeEntriesMap[sourceFile] = resolvedIncludes;

            // ⑦ Обновляем «репрезентации» файла:
            // т.е. для одного физического пути могут быть Document.SourceFile из разных проектов.
            if (!_sourceFileRepresentationsMap.TryGetValue(sourceFile.FilePath, out var sourceFilePathsRepresentationList)) {
                sourceFilePathsRepresentationList = new List<Document.SourceFile>();
                _sourceFileRepresentationsMap[sourceFile.FilePath] = sourceFilePathsRepresentationList;
            }
            if (!sourceFilePathsRepresentationList.Any(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.LoadedProject.UniqueName, sourceFile.LoadedProject.UniqueName))) {
                sourceFilePathsRepresentationList.Add(sourceFile);
            }
        }


        private void RemoveSourceFileInternal(Document.SourceFile sourceFile) {
            if (!_sourceFileToResolvedIncludeEntriesMap.TryGetValue(sourceFile, out var oldResolvedIncludes)) {
                Helpers.Diagnostic.Logger.LogDebug($"[SolutionSourceFileGraph.RemoveSourceFileInternal] sourceFile already removed: {sourceFile}");
                return;
            }

            foreach (var oldResolvedInclude in oldResolvedIncludes) {
                var raw = oldResolvedInclude.IncludeEntry;
                if (_includeEntryToSourceFilesMap.TryGetValue(raw, out var rawSourcesList)) {
                    rawSourcesList.Remove(sourceFile);
                    if (rawSourcesList.Count == 0) {
                        _includeEntryToSourceFilesMap.Remove(raw);
                    }
                }

                if (_resolvedIncludeEntryToSourceFilesMap.TryGetValue(oldResolvedInclude, out var resolvedIncludesSourcesList)) {
                    resolvedIncludesSourcesList.Remove(sourceFile);
                    if (resolvedIncludesSourcesList.Count == 0) {
                        _resolvedIncludeEntryToSourceFilesMap.Remove(oldResolvedInclude);
                    }
                }

                if (oldResolvedInclude.ResolvedPath is string path) {
                    if (_resolvedIncludePathsToSourceFilesMap.TryGetValue(path, out var resolvedIncludePathsSourcesList)) {
                        resolvedIncludePathsSourcesList.Remove(sourceFile);
                        if (resolvedIncludePathsSourcesList.Count == 0) {
                            _resolvedIncludePathsToSourceFilesMap.Remove(path);
                        }
                    }
                }
            }

            _sourceFileToResolvedIncludeEntriesMap.Remove(sourceFile);
            _sourceFileToIncludeEntriesMap.Remove(sourceFile);

            if (_sourceFileRepresentationsMap.TryGetValue(sourceFile.FilePath, out var sourceFilePathsRepresentationList)) {
                sourceFilePathsRepresentationList.RemoveAll(sf => StringComparer.OrdinalIgnoreCase.Equals(sf.LoadedProject.UniqueName, sourceFile.LoadedProject.UniqueName));
                if (sourceFilePathsRepresentationList.Count == 0) {
                    _sourceFileRepresentationsMap.Remove(sourceFile.FilePath);
                }
            }
        }
    }
}


/*
// Editor подключает Helpers.Shared как shared-проект (.vcxitems)
// Game и Engine используют .h-файлы через относительные пути

TestIncludeSolution/
├── Game/
│   ├── Game.cpp                      // #include "Logger.h", "Config.h", "Missing.h"
│   ├── GameShared.cpp               // #include "../Helpers.Shared/SharedUtils.h"
│   └── LocalIncludes/Logger.h
├── Editor/
│   ├── Editor.cpp                   // #include "../Helpers.Shared/Logger.h"
│   └── GameLink.cpp                 // #include "Logger.h"
├── Engine/
│   ├── Engine.cpp                   // #include "SharedUtils.h"
│   └── Nested/Inner.cpp            // #include "../../Helpers.Shared/Logger.h"
└── Helpers.Shared/
    ├── Logger.h
    ├── Config.h
    └── SharedUtils.h


1. Dictionary<SourceFile, List<IncludeEntry>> [_sourceFileToIncludeEntiresMap]
{
    Game.cpp [Game]                 => [ "Logger.h", "Config.h", "Missing.h" ],
    GameShared.cpp [Game]           => [ "../Helpers.Shared/SharedUtils.h" ],
    Logger.h [Game]                 => [ ],

    Editor.cpp [Editor]             => [ "../Helpers.Shared/Logger.h" ],
    GameLink.cpp [Editor]           => [ "Logger.h" ],
    Logger.h [Editor]               => [ ],
    Config.h [Editor]               => [ ],
    SharedUtils.h [Editor]          => [ ],

    Engine.cpp [Engine]             => [ "SharedUtils.h" ],
    Inner.cpp [Engine]              => [ "../../Helpers.Shared/Logger.h" ],
    Logger.h [Engine]               => [ ],

    Logger.h [Helpers.Shared]       => [ ],
    Config.h [Helpers.Shared]       => [ ],
    SharedUtils.h [Helpers.Shared]  => [ ]
}

2. Dictionary<IncludeEntry, List<SourceFile>> [_includeEntryToSourceFilesMap]
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
    Logger.h [Game] => [ ],

    Editor.cpp => [
        ("../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")
    ],
    GameLink.cpp => [
        ("Logger.h" → "Helpers.Shared/Logger.h")
    ],
    Logger.h [Editor] => [ ],
    Config.h [Editor] => [ ],
    SharedUtils.h [Editor] => [ ],

    Engine.cpp => [
        ("SharedUtils.h" → "Helpers.Shared/SharedUtils.h")
    ],
    Inner.cpp => [
        ("../../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")
    ],
    Logger.h [Engine] => [ ],

    Logger.h [Helpers.Shared] => [ ],
    Config.h [Helpers.Shared] => [ ],
    SharedUtils.h [Helpers.Shared] => [ ]
}

4. Dictionary<ResolvedIncludeEntry, List<SourceFile>> [_resolvedIncludeEntryToSourceFilesMap]
{
    ("Logger.h" → "Game/LocalIncludes/Logger.h")                     => [ Game.cpp ],
    ("Logger.h" → "Helpers.Shared/Logger.h")                         => [ GameLink.cpp ],
    ("../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")       => [ Editor.cpp ],
    ("../../Helpers.Shared/Logger.h" → "Helpers.Shared/Logger.h")    => [ Inner.cpp ],
    ("Config.h" → "Helpers.Shared/Config.h")                         => [ Game.cpp ],
    ("SharedUtils.h" → "Helpers.Shared/SharedUtils.h")              => [ Engine.cpp ],
    ("../Helpers.Shared/SharedUtils.h" → "Helpers.Shared/SharedUtils.h") => [ GameShared.cpp ],
    ("Missing.h" → null)                                             => [ Game.cpp ]
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
    "Game/LocalIncludes/Logger.h"     => [ SourceFile { ..., Project = Game } ],
    "Helpers.Shared/Logger.h"         => [
        SourceFile { ..., Project = Helpers.Shared },
        SourceFile { ..., Project = Editor },
        SourceFile { ..., Project = Engine }
    ],
    "Helpers.Shared/Config.h"         => [
        SourceFile { ..., Project = Helpers.Shared },
        SourceFile { ..., Project = Editor }
    ],
    "Helpers.Shared/SharedUtils.h"    => [
        SourceFile { ..., Project = Helpers.Shared },
        SourceFile { ..., Project = Editor }
    ]
}
 */