using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;


namespace Helpers.Time {
    public interface ITimerConfig<TEnum>
        where TEnum : struct, Enum {

        TimeSpan BaseInterval { get; }
        Dictionary<TEnum, int> GetMultipliers();
    }


    public sealed class TimerManager<TConfig, TEnum>
        where TConfig : ITimerConfig<TEnum>, new()
        where TEnum : struct, Enum {

        private readonly DispatcherTimer _dispatcherTimer;
        private readonly Dictionary<TEnum, System.Action?> _tickEvents = new();
        private readonly Dictionary<TEnum, int> _counters = new();
        private readonly Dictionary<TEnum, int> _multipliers;

        public TimerManager() {
            var config = new TConfig();

            _multipliers = config.GetMultipliers();

            foreach (var key in _multipliers.Keys) {
                _tickEvents[key] = null;
                _counters[key] = 0;
            }

            _dispatcherTimer = new DispatcherTimer {
                Interval = config.BaseInterval
            };

            _dispatcherTimer.Tick += this.OnTimerTick;
            _dispatcherTimer.Start();
        }

        //
        // API
        //
        public void Subscribe(TEnum timeSlot, System.Action handler) {
            _tickEvents[timeSlot] += handler;
        }

        public void Unsubscribe(TEnum timeSlot, System.Action handler) {
            _tickEvents[timeSlot] -= handler;
        }


        //
        // Internal logic
        //
        private void OnTimerTick(object? sender, EventArgs e) {
            foreach (var key in _multipliers.Keys) {
                _counters[key]++;

                if (_counters[key] >= _multipliers[key]) {
                    _counters[key] = 0;
                    _tickEvents[key]?.Invoke();
                }
            }
        }
    }
}