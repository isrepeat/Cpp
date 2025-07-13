using System;
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


namespace TabsManagerExtension.VsShell.Utils {
    public static class EnvDteUtils {
        static EnvDteUtils() {
            ThreadHelper.ThrowIfNotOnUIThread();
        }


        /// <summary>
        /// Возвращает все проекты в решении, включая проекты во вложенных Solution Folders.
        /// </summary>
        public static List<EnvDTE.Project> GetAllProjects() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<EnvDTE.Project>();
            var queue = new Queue<EnvDTE.Project>();

            foreach (EnvDTE.Project p in PackageServices.Dte2.Solution.Projects) {
                queue.Enqueue(p);
            }

            while (queue.Count > 0) {
                var project = queue.Dequeue();

                if (EnvDteUtils.IsMiscProject(project)) {
                    continue;
                }

                if (string.Equals(project.Kind, EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder, StringComparison.OrdinalIgnoreCase)) {
                    foreach (EnvDTE.ProjectItem item in project.ProjectItems) {
                        if (item.SubProject != null) {
                            queue.Enqueue(item.SubProject);
                        }
                    }
                }
                else {
                    result.Add(project);
                }
            }

            return result;
        }


        public static EnvDTE.Project? GetDteProjectFromHierarchy(IVsHierarchy hierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (hierarchy == null) {
                throw new ArgumentNullException(nameof(hierarchy));
            }

            // Пробуем через VSHPROPID_ExtObject (быстрее)
            hierarchy.GetProperty(
                VSConstants.VSITEMID_ROOT,
                (int)__VSHPROPID.VSHPROPID_ExtObject,
                out var extObject);

            if (extObject is EnvDTE.Project extProject) {
                return extProject;
            }

            // Fallback через GUID
            hierarchy.GetGuidProperty(
                VSConstants.VSITEMID_ROOT,
                (int)__VSHPROPID.VSHPROPID_ProjectIDGuid,
                out var projectGuid);

            foreach (var project in EnvDteUtils.GetAllProjects()) {
                if (EnvDteUtils.TryGetProjectGuid(project, out var guid) && guid == projectGuid) {
                    return project;
                }
            }

            return null;
        }


        public static IVsHierarchy? GetVsHierarchyFromDteProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.TryGetVsSolution();
            if (vsSolution == null) {
                return null;
            }

            int hr = vsSolution.GetProjectOfUniqueName(project.UniqueName, out IVsHierarchy hierarchy);
            return ErrorHandler.Succeeded(hr) ? hierarchy : null;
        }


        public static string GetDteProjectUniqueNameFromVsHierarchy(IVsHierarchy hierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.TryGetVsSolution();
            if (vsSolution == null) {
                return null;
            }

            vsSolution.GetUniqueNameOfProject(hierarchy, out var uniqueName);
            return uniqueName ?? "<unknown>";
        }




        public static bool IsDocumentOpen(string fullPath) {
            ThreadHelper.ThrowIfNotOnUIThread();

            return PackageServices.Dte2.Documents.Cast<EnvDTE.Document>()
                .Any(doc => string.Equals(doc.FullName, fullPath, StringComparison.OrdinalIgnoreCase));
        }


        public static bool IsFileInProject(string filePath, EnvDTE.Project dteProject) {
            ThreadHelper.ThrowIfNotOnUIThread();
            return EnvDteUtils.IsFileInProjectItemsRecursive(filePath, dteProject.ProjectItems);
        }


        public static bool IsMiscProject(EnvDTE.Project project) {
            // Это GUID для "Miscellaneous Files Project" — 
            // специальный виртуальный проект в Solution Explorer,
            // который появляется, если открыть файл напрямую в Visual Studio
            // (например через File → Open → File), не добавляя его в решение.
            // Такие проекты не имеют отношения к настоящим проектам решения,
            // их нужно всегда игнорировать при обходе.
            const string VsProjectKindMisc = "{66A2671D-8FB5-11D2-AA7E-00C04F688DDE}";

            if (string.Equals(project.Kind, VsProjectKindMisc, StringComparison.OrdinalIgnoreCase)) {
                return true;
            }
            return false;
        }

        /// <summary>
        /// Проверяет, является ли проект C++ (VCProject), включая C++/CLI, C++/CX, C++/WinRT.
        /// <br/> Все C++ проекты реализуют интерфейс VCProject.
        /// <br/> Проверка `project.Object is VCProject` — надёжный способ отфильтровать C++.
        /// <br/> Некоторые не-C++ проекты могут выбрасывать исключение при доступе к .Object — это игнорируется.
        /// </summary>
        public static bool IsCppProject(EnvDTE.Project project) {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                return project?.Object is Microsoft.VisualStudio.VCProjectEngine.VCProject;
            }
            catch {
                // Например SDK-проекты иногда выбрасывают при доступе к Object
                return false;
            }
        }



        //
        // Internal logic
        //
        private static bool TryGetProjectGuid(EnvDTE.Project project, out Guid projectGuid) {
            ThreadHelper.ThrowIfNotOnUIThread();

            projectGuid = Guid.Empty;

            try {
                var prop = project.Properties?.Item("ProjectGuid");
                if (prop != null && Guid.TryParse(prop.Value?.ToString(), out var parsedGuid)) {
                    projectGuid = parsedGuid;
                    return true;
                }
            }
            catch {
                // Проект может не содержать свойство ProjectGuid
            }

            return false;
        }

        private static bool IsFileInProjectItemsRecursive(string filePath, EnvDTE.ProjectItems items) {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (EnvDTE.ProjectItem item in items) {
                try {
                    for (short i = 1; i <= item.FileCount; i++) {
                        if (StringComparer.OrdinalIgnoreCase.Equals(item.FileNames[i], filePath)) {
                            return true;
                        }
                    }

                    if (item.ProjectItems != null && EnvDteUtils.IsFileInProjectItemsRecursive(filePath, item.ProjectItems)) {
                        return true;
                    }
                }
                catch {
                    // игнорируем странные COM-ошибки от DTE
                }
            }
            return false;
        }
    }
}