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


namespace TabsManagerExtension.VsShell.Solution.Services {
    public static class IncludeResolverService {
        /// <summary>
        /// Пробует разрешить include-строку в абсолютный путь.
        /// Возвращает путь к файлу, если он существует; иначе — null.
        /// </summary>
        public static string? TryResolveInclude(
            string includeRaw,
            string includingFilePath,
            VsShell.Project.ProjectNode ownerProject,
            MsBuildSolutionWatcher msBuildSolutionWatcher) {
            try {
                // ① Пробуем как относительный путь от файла
                string baseDir = Path.GetDirectoryName(includingFilePath)!;
                string resolvedLocal = Path.GetFullPath(Path.Combine(baseDir, includeRaw.Replace('/', '\\')));
                if (File.Exists(resolvedLocal)) {
                    return resolvedLocal;
                }

                // ② Пробуем директории проекта
                var projectIncludeDirs = msBuildSolutionWatcher.GetIncludeDirectoriesFor(ownerProject.FullName);
                foreach (var dir in projectIncludeDirs) {
                    string resolved = Path.GetFullPath(Path.Combine(dir, includeRaw.Replace('/', '\\')));
                    if (File.Exists(resolved)) {
                        return resolved;
                    }
                }
            }
            catch {
                // ignore
            }

            return null;
        }

        ///// <summary>
        ///// Проверяет, разрешается ли includeRaw из includingFilePath в конкретный файл candidateFilePath.
        ///// </summary>
        //public static bool IncludeResolvesToFile(
        //    string includeRaw,
        //    string includingFilePath,
        //    string candidateFilePath,
        //    VsShell.Project.ShellProject ownerProject,
        //    MsBuildSolutionWatcher msBuildSolutionWatcher) {

        //    var resolved = TryResolveInclude(includeRaw, includingFilePath, ownerProject, msBuildSolutionWatcher);
        //    if (resolved == null) {
        //        return false;
        //    }

        //    return string.Equals(Path.GetFullPath(candidateFilePath), resolved, StringComparison.OrdinalIgnoreCase);
        //}
    }
}