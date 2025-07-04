using System;
using System.IO;
using System.Windows;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Utilities;

namespace TabsManagerExtension.VsShell.Solution.Services {

    public sealed class VsSolutionExplorerSelectionTrackerService :
        VsShell.Services.VsSelectionEventsServiceBase<VsSolutionExplorerSelectionTrackerService>,
        TabsManagerExtension.Services.IExtensionService {

        public event Action<EnvDTE.ProjectItem>? ProjectItemSelected;

        public VsSolutionExplorerSelectionTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();
            Helpers.Diagnostic.Logger.LogDebug("[VsSolutionExplorerSelectionTrackerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            base.Dispose();

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsSolutionExplorerSelectionTrackerService] Disposed.");
        }

        //
        // VsSelectionEventsServiceBase
        //
        public override int OnSelectionChanged(
            IVsHierarchy pHierOld, uint itemidOld,
            IVsMultiItemSelect pMISOld, ISelectionContainer pSCOld,
            IVsHierarchy pHierNew, uint itemidNew,
            IVsMultiItemSelect pMISNew, ISelectionContainer pSCNew) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем: что-то действительно выбрано?
            if (pHierNew == null || itemidNew == VSConstants.VSITEMID_NIL) {
                // Если новый selection пустой (кликнули в пустое место), выходим.
                return VSConstants.S_OK;
            }

            try {
                // Извлекаем ExtObject — это обычно EnvDTE.ProjectItem, связанный с выбранным элементом в Solution Explorer.
                pHierNew.GetProperty(itemidNew, (int)__VSHPROPID.VSHPROPID_ExtObject, out object value);

                // Если это ProjectItem, значит выбрали файл или элемент проекта.
                if (value is EnvDTE.ProjectItem projectItem) {
                    // Вызываем событие для подписчиков (например, чтобы подсветить или обработать выбор).
                    this.ProjectItemSelected?.Invoke(projectItem);
                }
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"[VsSolutionItemsSelectionTrackerService] OnSelectionChanged error: {ex}");
            }

            return VSConstants.S_OK;
        }
    }
}