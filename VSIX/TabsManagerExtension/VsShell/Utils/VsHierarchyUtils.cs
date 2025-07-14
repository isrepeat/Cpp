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
    public static class VsHierarchyUtils {
        public sealed class HierarchyItem {
            public IVsHierarchy Hierarchy { get; }
            public uint ItemId { get; }
            public string? Name { get; }
            public string? CanonicalName { get; }
            public string? NormalizedPath { get; private set; }

            public HierarchyItem(IVsHierarchy hierarchy, uint itemId, string? name, string? canonicalName) {
                this.Hierarchy = hierarchy;
                this.ItemId = itemId;
                this.Name = name;
                this.CanonicalName = canonicalName;
            }

            public void CalculateNormilizedPath() {
                var hierarchyItemName = this.CanonicalName ?? this.Name ?? string.Empty;
                this.NormalizedPath = System.IO.Path.GetFullPath(hierarchyItemName)
                    .TrimEnd(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar);
            }

            public override bool Equals(object? obj) {
                return obj is HierarchyItem other &&
                       StringComparer.OrdinalIgnoreCase.Equals(this.CanonicalName, other.CanonicalName);
            }

            public override int GetHashCode() {
                return this.CanonicalName != null
                    ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.CanonicalName)
                    : 0;
            }
            public override string ToString() {
                return $"HierarchyItem(ItemId={this.ItemId}, Name='{this.Name}', CanonicalName='{this.CanonicalName}')";
            }
        }


        /// <summary>
        /// Walker
        /// </summary>
        public static class Walker {
            public static List<uint> GetChildren(IVsHierarchy hierarchy, uint parentId) {
                var result = new List<uint>();

                if (Walker.TryGetFirstChild(hierarchy, parentId, out var childId)) {
                    do {
                        result.Add(childId);
                    }
                    while (Walker.TryGetNextSibling(hierarchy, childId, out childId));
                }

                return result;
            }

            public static bool TryGetFirstChild(IVsHierarchy hierarchy, uint itemId, out uint childId) {
                childId = VSConstants.VSITEMID_NIL;

                if (hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_FirstChild, out var childObj) != VSConstants.S_OK ||
                    !(childObj is int rawChild) ||
                    unchecked((uint)rawChild) == VSConstants.VSITEMID_NIL) {
                    return false;
                }

                childId = unchecked((uint)rawChild);
                return true;
            }

            public static bool TryGetNextSibling(IVsHierarchy hierarchy, uint itemId, out uint siblingId) {
                siblingId = VSConstants.VSITEMID_NIL;

                if (hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_NextSibling, out var nextObj) != VSConstants.S_OK ||
                    !(nextObj is int rawNext) ||
                    unchecked((uint)rawNext) == VSConstants.VSITEMID_NIL) {
                    return false;
                }

                siblingId = unchecked((uint)rawNext);
                return true;
            }
        } // class Walker


        //
        // Api
        //
        public static void UnloadProject(Guid projectGuid) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution4 = (IVsSolution4)PackageServices.VsSolution;
            vsSolution4.UnloadProject(ref projectGuid, (uint)_VSProjectUnloadStatus.UNLOADSTATUS_UnloadedByUser);

            Helpers.Diagnostic.Logger.LogDebug($"[VsHierarchy] Unloaded project {projectGuid}");
        }

        public static void ReloadProject(Guid projectGuid) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution4 = (IVsSolution4)PackageServices.VsSolution;
            vsSolution4.ReloadProject(ref projectGuid);

            Helpers.Diagnostic.Logger.LogDebug($"[VsHierarchy] Reloaded project {projectGuid}");
        }

        
        public static void UnloadProjects(IEnumerable<Guid> projectGuids) {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var guid in projectGuids) {
                VsHierarchyUtils.UnloadProject(guid);
            }
        }

        public static void ReloadProjects(IEnumerable<Guid> projectGuids) {
            ThreadHelper.ThrowIfNotOnUIThread();

            foreach (var guid in projectGuids) {
                VsHierarchyUtils.ReloadProject(guid);
            }
        }


        public static List<HierarchyItem> CollectItemsRecursive(
            IVsHierarchy hierarchy,
            uint itemId,
            Func<HierarchyItem, bool> predicate
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var result = new List<HierarchyItem>();
            VsHierarchyUtils.CollectItemsRecursiveInternal(hierarchy, itemId, predicate, result);
            return result;
        }


        public static int ClickOnSolutionHierarchyItem(IVsHierarchy hierarchy, uint itemId) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Используем ExecCommand для эмуляции DoubleClick в Solution Explorer,
            // чтобы Visual Studio открыла файл так, как если бы пользователь дважды кликнул
            // по нему именно в контексте этого проекта в папке External Dependencies.
            if (hierarchy is IVsUIHierarchy uiHierarchy) {
                Guid cmdGroup = VSConstants.CMDSETID.UIHierarchyWindowCommandSet_guid;
                const uint cmdId = (uint)VSConstants.VsUIHierarchyWindowCmdIds.UIHWCMDID_DoubleClick;

                hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string ?? "(null)";
                
                hierarchy.GetCanonicalName(itemId, out var canonicalName);
                Helpers.Diagnostic.Logger.LogDebug($"[ClickOnSolutionHierarchyItem] Try open ItemId={itemId}, Name='{name}' ({canonicalName})");

                int hr = uiHierarchy.ExecCommand(
                    itemId,
                    ref cmdGroup,
                    cmdId,
                    0,
                    IntPtr.Zero,
                    IntPtr.Zero);

                if (ErrorHandler.Succeeded(hr)) {
                }
                else {
                    Helpers.Diagnostic.Logger.LogError($"[ClickOnSolutionHierarchyItem] Failed uiHierarchy.ExecCommand for itemId =  '{itemId}', hr=0x{hr:X8}");
                }

                return hr;
            }

            Helpers.Diagnostic.Logger.LogError($"[ClickOnSolutionHierarchyItem] Provided hierarchy does not implement IVsUIHierarchy.");
            return VSConstants.E_FAIL;
        }


        public static void LogSolutionHierarchy() {
            ThreadHelper.ThrowIfNotOnUIThread();

            var vsSolution = PackageServices.VsSolution;
            vsSolution.GetProjectEnum((uint)__VSENUMPROJFLAGS.EPF_LOADEDINSOLUTION, Guid.Empty, out var enumHierarchies);

            var hierarchies = new IVsHierarchy[1];
            uint fetched;

            while (enumHierarchies.Next(1, hierarchies, out fetched) == VSConstants.S_OK && fetched == 1) {
                var hierarchy = hierarchies[0];

                string projectName = Utils.EnvDteUtils.GetDteProjectUniqueNameFromVsHierarchy(hierarchy);
                Helpers.Diagnostic.Logger.LogDebug($"[Hierarchy] {projectName} (VSITEMID_ROOT)");

                VsHierarchyUtils.LogSolutionHierarchyRecursive(hierarchy, VSConstants.VSITEMID_ROOT, 0);
            }
        }


        public static void LogAllHierarchyProperties<TPropId>(
            IVsHierarchy hierarchy,
            uint itemid,
            string logPrefix = ""
            )
            where TPropId : Enum {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (typeof(TPropId) != typeof(__VSHPROPID) &&
                typeof(TPropId) != typeof(__VSHPROPID2) &&
                typeof(TPropId) != typeof(__VSHPROPID3) &&
                typeof(TPropId) != typeof(__VSHPROPID4) &&
                typeof(TPropId) != typeof(__VSHPROPID5) &&
                typeof(TPropId) != typeof(__VSHPROPID6) &&
                typeof(TPropId) != typeof(__VSHPROPID7) &&
                typeof(TPropId) != typeof(__VSHPROPID8) &&
                typeof(TPropId) != typeof(__VSHPROPID9) &&
                typeof(TPropId) != typeof(__VSHPROPID10) &&
                typeof(TPropId) != typeof(__VSHPROPID11)) {
                Helpers.Diagnostic.Logger.LogDebug($"Тип {typeof(TPropId).Name} не поддерживается для логирования.");
                return;
            }

            // __VSHPROPID содержит много повторяющихся значений (VSHPROPID_Name == VSHPROPID_ProjectName)
            // поэтому можно пропустить дубликаты через HashSet если нужно.

            foreach (TPropId prop in Enum.GetValues(typeof(TPropId))) {
                try {
                    int propId = Convert.ToInt32(prop);
                    hierarchy.GetProperty(itemid, propId, out object value);

                    string display = value switch {
                        null => "null",
                        string s => $"\"{s}\"",
                        bool b => b.ToString(),
                        Guid g => g.ToString(),
                        int i => i.ToString(),
                        _ => value.ToString()
                    };

                    Helpers.Diagnostic.Logger.LogDebug($"{logPrefix}[{itemid}] {typeof(TPropId).Name}.{prop} = {display}");
                }
                catch (NotImplementedException) {
                    Helpers.Diagnostic.Logger.LogDebug($"{logPrefix}[{itemid}] {typeof(TPropId).Name}.{prop} = Not implemented");
                }
                catch (Exception ex) {
                    Helpers.Diagnostic.Logger.LogDebug($"{logPrefix}[{itemid}] {typeof(TPropId).Name}.{prop} = Error: {ex.Message}");
                }
            }
        }


        //
        // Internal logic
        //
        private static void CollectItemsRecursiveInternal(
            IVsHierarchy hierarchy,
            uint itemId,
            Func<HierarchyItem, bool> predicate,
            List<HierarchyItem> result
            ) {
            ThreadHelper.ThrowIfNotOnUIThread();

            hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
            var name = nameObj as string;

            hierarchy.GetCanonicalName(itemId, out var canonicalName);
            var hierarchyItem = new HierarchyItem(hierarchy, itemId, name, canonicalName);

            if (predicate(hierarchyItem)) {
                result.Add(hierarchyItem);
            }

            foreach (var childId in Walker.GetChildren(hierarchy, itemId)) {
                VsHierarchyUtils.CollectItemsRecursiveInternal(hierarchy, childId, predicate, result);
            }
        }

        private static void LogSolutionHierarchyRecursive(IVsHierarchy hierarchy, uint itemId, int indent) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (itemId != VSConstants.VSITEMID_ROOT) {
                hierarchy.GetProperty(itemId, (int)__VSHPROPID.VSHPROPID_Name, out var nameObj);
                var name = nameObj as string ?? "(null)";

                hierarchy.GetCanonicalName(itemId, out var canonicalName);

                string itemMarker = "";
                
                bool isSharedItem = false;
                hierarchy.GetProperty(itemId,(int)__VSHPROPID7.VSHPROPID_IsSharedItem, out var isSharedItemObj);
                if (isSharedItemObj is bool boolVal) {
                    isSharedItem = boolVal;
                }

                if (isSharedItem) {
                    itemMarker = " | Shared";
                }

                string indentStr = new string(' ', indent * 2);
                Helpers.Diagnostic.Logger.LogDebug($"[{itemId}]{indentStr}{name} ({canonicalName}){itemMarker}");
            }

            foreach (var childId in Walker.GetChildren(hierarchy, itemId)) {
                VsHierarchyUtils.LogSolutionHierarchyRecursive(hierarchy, childId, indent + 1);
            }
        }
    }
}