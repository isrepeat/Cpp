using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;
using System.Linq;


namespace TabsManagerExtension.VsShell.Project {
    public class ShellProject {
        public EnvDTE.Project Project { get; private set; }

        public ShellProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            this.Project = project;
        }


        /// <summary>
        /// Возвращает список директорий из MSBuild-свойства (например, "PublicIncludeDirectories", "AdditionalLibraryDirectories").
        /// 
        /// ⚠️ Значение извлекается только для активной конфигурации (например, "Debug|x64") —
        /// именно так, как это делает Visual Studio при сборке проекта.
        /// </summary>
        public List<string> GetMsBuildProjectPropertyList(string propertyName) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<string>();
            string? rawValue = null;

            var hierarchy = Utils.EnvDteUtils.GetVsHierarchyFromDteProject(this.Project);
            if (hierarchy is not IVsBuildPropertyStorage storage) {
                return result;
            }

            int hr = storage.GetPropertyValue(propertyName, null, (uint)_PersistStorageType.PST_PROJECT_FILE, out rawValue);
            if (!ErrorHandler.Succeeded(hr) || string.IsNullOrWhiteSpace(rawValue)) {
                return result;
            }

            foreach (var dir in rawValue.Split(';')) {
                string trimmed = dir.Trim();
                if (string.IsNullOrEmpty(trimmed)) {
                    continue;
                }

                string expanded = ExpandMsBuildVariables(trimmed, this.Project);

                if (!expanded.Contains("%")) {
                    try {
                        result.Add(Path.GetFullPath(expanded));
                    }
                    catch {
                        // skip invalid
                    }
                }
            }

            return result.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        }

        public List<string> GetAdditionalIncludeDirectories() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope("GetProjectIncludeDirectories()");
            Helpers.Diagnostic.Logger.LogDebug($"  [Project.Name] = {this.Project.Name}");

            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<string>();

            if (this.Project.Object is Microsoft.VisualStudio.VCProjectEngine.VCProject vcProject) {
                var configs = vcProject.Configurations as Microsoft.VisualStudio.VCProjectEngine.IVCCollection;
                if (configs == null) {
                    return result;
                }

                Helpers.Diagnostic.Logger.LogDebug($"  [configs.Count] = {configs.Count}");

                foreach (Microsoft.VisualStudio.VCProjectEngine.VCConfiguration config in configs) {
                    Helpers.Diagnostic.Logger.LogDebug($"    [config.Name] = {config.Name}");
                    var tools = config.Tools as Microsoft.VisualStudio.VCProjectEngine.IVCCollection;
                    if (tools == null) {
                        continue;
                    }
                    Helpers.Diagnostic.Logger.LogDebug($"    [tools.Count] = {tools.Count}");

                    foreach (object tool in tools) {
                        if (tool is Microsoft.VisualStudio.VCProjectEngine.VCCLCompilerTool cl) {
                            Helpers.Diagnostic.Logger.LogDebug($"      [cl.Name] = {cl.toolName}");

                            var dirs = cl.AdditionalIncludeDirectories?.Split(';') ?? Array.Empty<string>();

                            foreach (var dir in dirs) {
                                Helpers.Diagnostic.Logger.LogDebug($"        [dir] = {dir}");

                                string trimmed = dir.Trim();
                                if (!string.IsNullOrEmpty(trimmed)) {
                                    string expanded = ExpandMsBuildVariables(trimmed, this.Project);

                                    if (!expanded.Contains("%")) {
                                        try {
                                            result.Add(Path.GetFullPath(expanded));
                                        }
                                        catch {
                                            // ignore invalid paths
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        }



        private string ExpandMsBuildVariables(string pathWithVars, EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            string expanded = pathWithVars;

            if (string.IsNullOrEmpty(pathWithVars)) {
                return expanded;
            }

            var hierarchy = Utils.EnvDteUtils.GetVsHierarchyFromDteProject(project);
            if (hierarchy is not IVsBuildPropertyStorage propertyStorage) {
                return expanded;
            }

            // Попробуем найти все $(...) и заменить по очереди
            var matches = System.Text.RegularExpressions.Regex.Matches(pathWithVars, @"\$\(([^)]+)\)");
            foreach (System.Text.RegularExpressions.Match match in matches) {
                string fullVar = match.Groups[0].Value; // $(ProjectDir)
                string name = match.Groups[1].Value;    // ProjectDir

                int hr = VSConstants.S_OK;
                string value = string.Empty;

                if (name == "MSBuildThisFileDirectory") {
                    value = Path.GetDirectoryName(project.FullName);
                }
                else {
                    hr = propertyStorage.GetPropertyValue(name, null, (uint)_PersistStorageType.PST_PROJECT_FILE, out value);
                }

                if (ErrorHandler.Succeeded(hr) && !string.IsNullOrWhiteSpace(value)) {
                    expanded = System.Text.RegularExpressions.Regex.Replace(
                                expanded,
                                System.Text.RegularExpressions.Regex.Escape(fullVar),
                                value,
                                System.Text.RegularExpressions.RegexOptions.IgnoreCase);
                }
            }

            return expanded;
        }
    }
}