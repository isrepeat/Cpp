using System;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Linq;

namespace VideoPlayerPrototype {
    public delegate void DynamicRenderResolutionChanged(int resolutionPercent);

    public class DynamicRenderResolutionScale : INotifyPropertyChanged {
        public event DynamicRenderResolutionChanged DynamicRenderResolutionChangedEvent;
        public event PropertyChangedEventHandler PropertyChanged;
        public void NotifyPropertyChanged(String propertyName = "") {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public DynamicRenderResolutionScale() {
        }

        public static int MinScaleValue {
            get {
                return 20;
            }
        }
        public static int MaxScaleValue {
            get {
                return 100;
            }
        }

        private int scaleValue = MaxScaleValue;
        public int ScaleValue {
            get {
                return scaleValue;
            }
            set {
                scaleValue = value; // Must be set from UI
                DynamicRenderResolutionChangedEvent(scaleValue);
                NotifyPropertyChanged("ScaleValue");
            }
        }

        public string Units {
            get {
                return "%";
            }
        }

        private bool isEnabled = true;
        public bool IsEnabled {
            get {
                return isEnabled;
            }
            set {
                isEnabled = value;
                NotifyPropertyChanged("IsEnabled");
            }
        }
    }
} 