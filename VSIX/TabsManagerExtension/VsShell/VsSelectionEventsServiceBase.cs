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
using TabsManagerExtension.Services;
using TabsManagerExtension.VsShell.Solution.Services;


namespace TabsManagerExtension.VsShell.Services {
    public abstract class VsSelectionEventsServiceBase<TService> :
        TabsManagerExtension.Services.SingletonServiceBase<TService>,
        IVsSelectionEvents,
        IDisposable
        where TService : VsSelectionEventsServiceBase<TService>, IExtensionService, new() {

        private uint _selectionEventsCookie;
        
        private bool _disposed = false;

        protected VsSelectionEventsServiceBase() {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Подписываем этот класс (реализует IVsSelectionEvents) на события Visual Studio,
            // чтобы получать уведомления о смене selection.
            int hr = PackageServices.VsMonitorSelection.AdviseSelectionEvents(this, out _selectionEventsCookie);
            ErrorHandler.ThrowOnFailure(hr);
        }

        public void Dispose() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_disposed) {
                return;
            }

            if (_selectionEventsCookie != 0) {
                PackageServices.VsMonitorSelection.UnadviseSelectionEvents(_selectionEventsCookie);
                _selectionEventsCookie = 0;
            }

            _disposed = true;
        }

        /// <summary>
        /// Вызывается Visual Studio (через AdviseSelectionEvents), когда пользователь выбирает или меняет выбранный элемент 
        /// в любом иерархическом браузере среды (чаще всего в Solution Explorer).
        /// 
        /// - pHierNew + itemidNew указывают на новый выбранный элемент.
        /// - pHierOld + itemidOld указывают на предыдущий выбранный элемент.
        /// 
        /// Например:
        /// - Было выделено Solution, затем пользователь кликнул на Project1.
        /// - VS вызовет этот метод, передав pHierNew = Project1.
        /// 
        /// Не вызывается при переключении вкладок редактора (для этого используется OnElementValueChanged с SEID_WindowFrame).
        /// </summary>
        public virtual int OnSelectionChanged(
            IVsHierarchy pHierOld, uint itemidOld,
            IVsMultiItemSelect pMISOld, ISelectionContainer pSCOld,
            IVsHierarchy pHierNew, uint itemidNew,
            IVsMultiItemSelect pMISNew, ISelectionContainer pSCNew) {
            return VSConstants.S_OK;
        }

        public virtual int OnElementValueChanged(uint elementid, object oldValue, object newValue) {
            return VSConstants.S_OK;
        }

        public virtual int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) {
            return VSConstants.S_OK;
        }
    }
}

// CmdUIContext — это глобальный механизм Visual Studio для отслеживания состояния среды.
// По сути, каждый CmdUIContext представляет собой флаг (on/off), например:
// - SolutionExists: открыт ли solution
// - Debugging: запущена ли отладка
// - SingleFileMode: открыт ли один файл без решения
//
// Visual Studio поддерживает внутреннюю таблицу этих контекстов, автоматически
// обновляя их при событиях среды (открытие/закрытие решения, запуск/остановка отладки и т.д.).
// Каждый контекст идентифицируется GUID'ом, который через GetCmdUIContextCookie
// преобразуется в более быстрый uint "cookie".
// Этот cookie затем используется:
// - для проверки текущего состояния контекста (IsCmdUIContextActive),
// - для обработки событий изменения состояния (OnCmdUIContextChanged).
//
// Таким образом, CmdUIContext выступает как высокоуровневый глобальный флаг,
// а Visual Studio сама заботится о его актуализации.