#define ENABLE_VERBOSE_LOGGING
using Microsoft.Build.Evaluation;
using Microsoft.Build.Locator;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Threading;
using System.Xml;
using WpfTestApp.__Local;



//using Local = WpfTestApp.__Local;
using Local = Helpers;

namespace WpfTestApp {
    namespace __Local {

        public static class MsBuildEnvironment {
            private static bool _initialized = false;

            public static void EnsureInitialized() {
                if (_initialized) {
                    return;
                }

                MSBuildLocator.RegisterDefaults();

                Environment.SetEnvironmentVariable(
                    "VCTargetsPath",
                    @"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\"
                );

                _initialized = true;
            }
        }



        public sealed class MsBuildProjectAnalyzer : IDisposable {

            public event Action<List<string>>? OnReferencesChanged;

            
            private List<string> _currentReferences = new();
            public IReadOnlyList<string> CurrentReferences {
                get {
                    lock (this) {
                        return this._currentReferences.ToList();
                    }
                }
            }


            private List<string> _additionalIncludeDirs = new();
            public IReadOnlyList<string> AdditionalIncludeDirectories {
                get {
                    lock (this) {
                        return this._additionalIncludeDirs.ToList();
                    }
                }
            }


            private List<string> _publicIncludeDirsFromReferences = new();

            public IReadOnlyList<string> PublicIncludeDirectoriesFromReferences {
                get {
                    lock (this) {
                        return this._publicIncludeDirsFromReferences.ToList();
                    }
                }
            }


            private readonly string _projectFilePath;
            private readonly FileSystemWatcher _watcher;
            private Project? _loadedProject;

            public MsBuildProjectAnalyzer(string projectFilePath) {
                MsBuildEnvironment.EnsureInitialized();

                this._projectFilePath = Path.GetFullPath(projectFilePath);

                this._watcher = new FileSystemWatcher(Path.GetDirectoryName(this._projectFilePath)!) {
                    Filter = Path.GetFileName(this._projectFilePath),
                    NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Size,
                    EnableRaisingEvents = true
                };

                this._watcher.Changed += (_, _) => this.ReloadProject();
                this._watcher.Created += (_, _) => this.ReloadProject();
                this._watcher.Renamed += (_, _) => this.ReloadProject();

                this.ReloadProject();
            }

            public void Dispose() {
                this._watcher.Dispose();
                this._loadedProject?.ProjectCollection?.UnloadAllProjects();
            }

            private void ReloadProject() {
                try {
                    //var project = new Project(this._projectFilePath);
                    //this._loadedProject = project;

                    var projectCollection = new ProjectCollection();
                    var project = projectCollection.LoadProject(this._projectFilePath);
                    this._loadedProject = project;


                    var newRefs = project.GetItems("ProjectReference")
                        .Select(i => Path.GetFullPath(Path.Combine(
                            Path.GetDirectoryName(this._projectFilePath)!,
                            i.EvaluatedInclude)))
                        .ToList();

                    bool changed;
                    lock (this) {
                        changed = !newRefs.SequenceEqual(this._currentReferences, StringComparer.OrdinalIgnoreCase);

                        if (changed) {
                            this._currentReferences = newRefs;
                            this._publicIncludeDirsFromReferences = this.RecalculatePublicIncludeDirectories(newRefs);
                        }

                        this._additionalIncludeDirs = this.RecalculateAdditionalIncludeDirectories(this._loadedProject);
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
            public event Action<string>? OnProjectReferencesChanged; // параметр — путь к .vcxproj

            private readonly Dictionary<string, MsBuildProjectAnalyzer> _analyzers = new(StringComparer.OrdinalIgnoreCase);

            public MsBuildSolutionWatcher(IEnumerable<string> vcxprojPaths) {
                MsBuildEnvironment.EnsureInitialized();

                foreach (string path in vcxprojPaths) {
                    string fullPath = Path.GetFullPath(path);
                    if (File.Exists(fullPath)) {
                        var analyzer = new MsBuildProjectAnalyzer(fullPath);
                        analyzer.OnReferencesChanged += _ => this.OnProjectReferencesChanged?.Invoke(fullPath);
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
    } // namespace __Local



    public partial class MainWindow_5_ProjectParser : Window {
        //private ProjectReferenceWatcher _projectReferenceWatcher;
        private MsBuildProjectAnalyzer _msBuildProjectAnalyzer;
        private MsBuildSolutionWatcher _msBuildSolutionWatcher;
        public MainWindow_5_ProjectParser() {
            this.InitializeComponent();
            this.DataContext = this;

            //_msBuildProjectAnalyzer = new MsBuildProjectAnalyzer(@"d:\WORK\TEST\Extensions\SimpleSolution\DxDesktop\DxDesktop.vcxproj");
            //this.LogProjectReferences();

            //_msBuildProjectAnalyzer.OnReferencesChanged += refs => {
            //    this.LogProjectReferences();
            //};


            string dxDesktopProject = @"d:\WORK\TEST\Extensions\SimpleSolution\DxDesktop\DxDesktop.vcxproj";
            string helpersRawProject = @"d:\WORK\TEST\Extensions\SimpleSolution\Helpers.Raw\Helpers.Raw.vcxproj";

            _msBuildSolutionWatcher = new MsBuildSolutionWatcher(new[] {
                dxDesktopProject,
                helpersRawProject,
            });

            _msBuildSolutionWatcher.OnProjectReferencesChanged += _ => this.LogIncludes();


            this.LogIncludes();
        }

        private void OnNavigateToSmth(object sender, RoutedEventArgs e) {
            //foreach (var reference in _projectReferenceWatcher.CurrentReferences) {
            //    Helpers.Diagnostic.Logger.LogDebug($"reference = {reference}");
            //}
            //foreach (var publicIncludeDir in _projectReferenceWatcher.GetDirectPublicIncludeDirectories()) {
            //    Helpers.Diagnostic.Logger.LogDebug($"publicIncludeDir = {publicIncludeDir}");
            //}
        }

        private void LogIncludes() {
            //Helpers.Diagnostic.Logger.LogDebug($"CurrentReferences:");
            //foreach (var reference in _msBuildProjectAnalyzer.CurrentReferences) {
            //    Helpers.Diagnostic.Logger.LogDebug($"  reference = {reference}");
            //}
            //Helpers.Diagnostic.Logger.LogDebug($"AdditionalIncludeDirectories:");
            //foreach (var additionalIncludeDir in _msBuildProjectAnalyzer.AdditionalIncludeDirectories) {
            //    Helpers.Diagnostic.Logger.LogDebug($"  additionalIncludeDir = {additionalIncludeDir}");
            //}
            //Helpers.Diagnostic.Logger.LogDebug($"PublicIncludeDirectoriesFromReferences:");
            //foreach (var publicIncludeDirFromReference in _msBuildProjectAnalyzer.PublicIncludeDirectoriesFromReferences) {
            //    Helpers.Diagnostic.Logger.LogDebug($"  publicIncludeDirFromReference = {publicIncludeDirFromReference}");
            //}

            string dxDesktopProject = @"d:\WORK\TEST\Extensions\SimpleSolution\DxDesktop\DxDesktop.vcxproj";
            string helpersRawProject = @"d:\WORK\TEST\Extensions\SimpleSolution\Helpers.Raw\Helpers.Raw.vcxproj";

            Helpers.Diagnostic.Logger.LogDebug($"IncludeDirectories [DxDesktop]:");
            foreach (var publicDir in _msBuildSolutionWatcher.GetIncludeDirectoriesFor(dxDesktopProject)) {
                Helpers.Diagnostic.Logger.LogDebug($"  publicDir = {publicDir}");
            }
            Helpers.Diagnostic.Logger.LogDebug($"IncludeDirectories [Helpers.Raw]:");
            foreach (var publicDir in _msBuildSolutionWatcher.GetIncludeDirectoriesFor(helpersRawProject)) {
                Helpers.Diagnostic.Logger.LogDebug($"  publicDir = {publicDir}");
            }
            Helpers.Diagnostic.Logger.LogDebug($"");
        }
    }
}