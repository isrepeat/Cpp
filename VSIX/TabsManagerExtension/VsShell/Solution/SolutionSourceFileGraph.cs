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
        /// Все SourceFile, обнаруженные в решении.
        private readonly HashSet<SourceFile> _allSourceFiles = new();
        public IEnumerable<SourceFile> AllSourceFiles => _allSourceFiles;

        /// Быстрый поиск SourceFile по абсолютному пути.
        /// Используется, например, при FileSystemWatcher-событиях.
        private readonly Dictionary<string, SourceFile> _pathToSourceFile = new(StringComparer.OrdinalIgnoreCase);

        /// Обратная карта зависимостей:
        /// ключ — файл, который включается;
        /// значение — список файлов, которые его включают.
        private readonly Dictionary<SourceFile, HashSet<SourceFile>> _includedFileToIncluders = new();

        private MsBuildSolutionWatcher _msBuildSolutionWatcher;
        public SolutionSourceFileGraph(MsBuildSolutionWatcher msBuildSolutionWatcher) {
            _msBuildSolutionWatcher = msBuildSolutionWatcher;
        }

        public void Add(SourceFile file) {
            if (_allSourceFiles.Add(file)) {
                _pathToSourceFile[file.FilePath] = file;
            }
        }


        /// <summary>
        /// Автоматически пересчитывает карту зависимостей включения на основе текущих SourceFile.
        /// </summary>
        public void BuildReverseMap() {
            _includedFileToIncluders.Clear();

            foreach (var includer in _allSourceFiles) {
                foreach (var include in includer.Includes) {
                    var resolvedPath = Service.IncludeResolverService.TryResolveInclude(
                        include.RawInclude,
                        includer.FilePath,
                        includer.Project,
                        _msBuildSolutionWatcher
                    );

                    if (resolvedPath != null && this.TryGetSourceFileByPath(resolvedPath, out var includedFile)) {
                        if (!_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
                            includers = new HashSet<SourceFile>();
                            _includedFileToIncluders[includedFile] = includers;
                        }

                        includers.Add(includer);
                    }
                }
            }
        }


        public void UpdateSourceFile(SourceFile updatedFile, Func<IncludeFile, SourceFile, SourceFile?> resolveInclude) {
            // Заменяем или добавляем сам файл
            _pathToSourceFile[updatedFile.FilePath] = updatedFile;
            _allSourceFiles.Remove(updatedFile); // перестраховка (по Equals)
            _allSourceFiles.Add(updatedFile);

            // Удаляем старые зависимости этого файла
            foreach (var entry in _includedFileToIncluders.Values) {
                entry.Remove(updatedFile);
            }

            // Регистрируем новые зависимости
            foreach (var include in updatedFile.Includes) {
                var includedFile = resolveInclude(include, updatedFile);
                if (includedFile != null) {
                    if (!_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
                        includers = new HashSet<SourceFile>();
                        _includedFileToIncluders[includedFile] = includers;
                    }

                    includers.Add(updatedFile);
                }
            }
        }



        public bool TryGetSourceFileByPath(string filePath, out SourceFile? source) {
            return _pathToSourceFile.TryGetValue(filePath, out source);
        }


        /// <summary>
        /// Возвращает все файлы, которые делают #include данного файла.
        /// </summary>
        public IEnumerable<SourceFile> GetSourceFilesWhoInclude(SourceFile includedFile) {
            if (_includedFileToIncluders.TryGetValue(includedFile, out var includers)) {
                return includers;
            }

            return Enumerable.Empty<SourceFile>();
        }


        public void Clear() {
            _allSourceFiles.Clear();
            _pathToSourceFile.Clear();
            _includedFileToIncluders.Clear();
        }
    }
}