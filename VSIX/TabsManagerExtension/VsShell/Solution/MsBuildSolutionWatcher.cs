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

    public static class MsBuildEnvironment {
        private static bool _initialized = false;

        public static void EnsureInitialized() {
            if (_initialized) {
                return;
            }

            ThreadHelper.ThrowIfNotOnUIThread();

            bool isRunningInsideVisualStudio = Package.GetGlobalService(typeof(SDTE)) is EnvDTE.DTE;
            if (!isRunningInsideVisualStudio) {
                /// Вызов MSBuildLocator.RegisterDefaults() необходим в обычных .NET приложениях,
                /// чтобы указать путь к используемой версии MSBuild. Однако в Visual Studio (внутри VSIX) MSBuild-сборки
                /// уже загружены, и попытка вызвать RegisterDefaults() вызовет исключение,
                /// т.к. Visual Studio уже настроена на нужную среду MSBuild.
                MSBuildLocator.RegisterDefaults();

                Environment.SetEnvironmentVariable(
                    "VCTargetsPath",
                    @"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\"
                );
            }

            _initialized = true;
        }
    }



    public sealed class MsBuildProjectAnalyzer : IDisposable {

        public event Action<List<string>>? OnReferencesChanged;


        private List<string> _currentReferences = new();
        public IReadOnlyList<string> CurrentReferences {
            get {
                lock (this) {
                    return _currentReferences.ToList();
                }
            }
        }


        private List<string> _additionalIncludeDirs = new();
        public IReadOnlyList<string> AdditionalIncludeDirectories {
            get {
                lock (this) {
                    return _additionalIncludeDirs.ToList();
                }
            }
        }


        private List<string> _publicIncludeDirsFromReferences = new();

        public IReadOnlyList<string> PublicIncludeDirectoriesFromReferences {
            get {
                lock (this) {
                    return _publicIncludeDirsFromReferences.ToList();
                }
            }
        }


        private readonly string _projectFilePath;
        private readonly FileSystemWatcher _watcher;
        private Project? _loadedProject;

        public MsBuildProjectAnalyzer(string projectFilePath) {
            MsBuildEnvironment.EnsureInitialized();

            _projectFilePath = Path.GetFullPath(projectFilePath);

            _watcher = new FileSystemWatcher(Path.GetDirectoryName(_projectFilePath)!) {
                Filter = Path.GetFileName(_projectFilePath),
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Size,
                EnableRaisingEvents = true
            };

            _watcher.Changed += (_, _) => this.ReloadProject();
            _watcher.Created += (_, _) => this.ReloadProject();
            _watcher.Renamed += (_, _) => this.ReloadProject();

            this.ReloadProject();
        }

        public void Dispose() {
            _watcher.Dispose();
            _loadedProject?.ProjectCollection?.UnloadAllProjects();
        }

        private void ReloadProject() {
            try {
                var projectCollection = new ProjectCollection();
                var project = projectCollection.LoadProject(_projectFilePath);
                _loadedProject = project;

                var newRefs = project.GetItems("ProjectReference")
                    .Select(i => Path.GetFullPath(Path.Combine(
                        Path.GetDirectoryName(_projectFilePath)!,
                        i.EvaluatedInclude)))
                    .ToList();

                bool changed;
                lock (this) {
                    changed = !newRefs.SequenceEqual(_currentReferences, StringComparer.OrdinalIgnoreCase);

                    if (changed) {
                        _currentReferences = newRefs;
                        _publicIncludeDirsFromReferences = this.RecalculatePublicIncludeDirectories(newRefs);
                    }

                    _additionalIncludeDirs = this.RecalculateAdditionalIncludeDirectories(_loadedProject);
                }

                if (changed) {
                    this.OnReferencesChanged?.Invoke(newRefs);
                }
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"[MsBuildProjectAnalyzer] Reload failed: {ex}");
            }
        }


        private List<string> RecalculateAdditionalIncludeDirectories(Project project) {
            var result = new List<string>();
            string baseDir = Path.GetDirectoryName(project.FullPath)!;

            if (project.ItemDefinitions.TryGetValue("ClCompile", out var clCompile)) {
                string raw = clCompile.GetMetadataValue("AdditionalIncludeDirectories");

                foreach (var part in raw.Split(';')) {
                    string trimmed = part.Trim();
                    if (string.IsNullOrEmpty(trimmed) || trimmed == "%(AdditionalIncludeDirectories)") {
                        continue;
                    }

                    string resolved = trimmed
                        .Replace("$(MSBuildThisFileDirectory)", baseDir)
                        .Replace("$(ProjectDir)", baseDir);

                    result.Add(Path.GetFullPath(resolved));
                }
            }

            return result;
        }


        private List<string> RecalculatePublicIncludeDirectories(List<string> referencePaths) {
            var result = new List<string>();

            foreach (var refPath in referencePaths) {
                if (!File.Exists(refPath)) {
                    continue;
                }

                try {
                    var project = new Microsoft.Build.Evaluation.Project(refPath);
                    string raw = project.GetPropertyValue("PublicIncludeDirectories");
                    string baseDir = Path.GetDirectoryName(refPath)!;

                    foreach (var part in raw.Split(';')) {
                        string trimmed = part.Trim();
                        if (string.IsNullOrEmpty(trimmed)) {
                            continue;
                        }

                        string resolved = trimmed
                            .Replace("$(MSBuildThisFileDirectory)", baseDir)
                            .Replace("$(ProjectDir)", baseDir);

                        result.Add(Path.GetFullPath(resolved));
                    }

                    project.ProjectCollection.UnloadAllProjects();
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogError($"[MsBuildProjectAnalyzer] Failed to read PublicIncludeDirectories from: {refPath} → {ex}");
                }
            }

            return result;
        }
    }



    public sealed class MsBuildSolutionWatcher : IDisposable {
        public event Action<string>? OnProjectReferencesChanged;    

        private readonly Dictionary<string, MsBuildProjectAnalyzer> _analyzers = new(StringComparer.OrdinalIgnoreCase);

        public MsBuildSolutionWatcher(IEnumerable<EnvDTE.Project> projects) {
            ThreadHelper.ThrowIfNotOnUIThread();
            MsBuildEnvironment.EnsureInitialized();

            foreach (var project in projects) {
                string fullPath = Path.GetFullPath(project.FullName);
                if (File.Exists(fullPath)) {
                    var analyzer = new MsBuildProjectAnalyzer(fullPath);
                    //analyzer.OnReferencesChanged += _ => this.OnProjectReferencesChanged?.Invoke(fullPath);
                    _analyzers[fullPath] = analyzer;
                }
            }
        }

        public IReadOnlyList<string> GetIncludeDirectoriesFor(string projectPath) {
            string fullPath = Path.GetFullPath(projectPath);
            if (_analyzers.TryGetValue(fullPath, out var analyzer)) {
                return analyzer.AdditionalIncludeDirectories
                    .Concat(analyzer.PublicIncludeDirectoriesFromReferences)
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToList();
            }

            return new List<string>();
        }

        public IReadOnlyList<string> GetAllProjectPaths() {
            return _analyzers.Keys.ToList();
        }

        public void Dispose() {
            foreach (var a in _analyzers.Values) {
                a.Dispose();
            }

            _analyzers.Clear();
        }
    }
}