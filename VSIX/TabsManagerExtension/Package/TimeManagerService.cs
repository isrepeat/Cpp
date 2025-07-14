using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.ComponentModel.Design;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.TextManager.Interop;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension {
    namespace Enums {
        public enum TimeSlot {
            _100ms,
            _300ms,
            _500ms,
            _1s,
            _3s,
            _5s
        }
    }
}


namespace TabsManagerExtension.Services {
    public class TimeManagerService :
        SingletonServiceBase<TimeManagerService>,
        IExtensionService {

        private Helpers.Time.TimerManager<TimerTypeConfig, Enums.TimeSlot> _timerManager;
        
        public TimeManagerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _timerManager = new Helpers.Time.TimerManager<TimerTypeConfig, Enums.TimeSlot>();
            Helpers.Diagnostic.Logger.LogDebug("[TimeManagerService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[TimeManagerService] Disposed.");
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public void Subscribe(Enums.TimeSlot timeSlot, Action handler) {
            _timerManager.Subscribe(timeSlot, handler);
        }

        public void Unsubscribe(Enums.TimeSlot timeSlot, Action handler) {
            _timerManager.Unsubscribe(timeSlot, handler);
        }


        //
        // ░ Config
        //
        public class TimerTypeConfig : Helpers.Time.ITimerConfig<Enums.TimeSlot> {
            public TimeSpan BaseInterval => TimeSpan.FromMilliseconds(100);

            public Dictionary<Enums.TimeSlot, int> GetMultipliers() {
                return new Dictionary<Enums.TimeSlot, int> {
                    { Enums.TimeSlot._100ms, 1 },
                    { Enums.TimeSlot._300ms, 3 },
                    { Enums.TimeSlot._500ms, 5 },
                    { Enums.TimeSlot._1s, 10 },
                    { Enums.TimeSlot._3s, 30 },
                    { Enums.TimeSlot._5s, 50 }
                };
            }
        }
    }
}