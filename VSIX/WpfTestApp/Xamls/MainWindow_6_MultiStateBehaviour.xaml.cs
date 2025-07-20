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
using System.Runtime.InteropServices;
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
using WpfTestApp.States.MultistateBehaviour;


public static class ObjectExtensions {
    public static string ex_GetObjectAddressHex(this object obj) {
        GCHandle handle = GCHandle.Alloc(obj, GCHandleType.WeakTrackResurrection);
        try {
            IntPtr ptr = GCHandle.ToIntPtr(handle);
            return ptr.ToString("X");
        }
        finally {
            handle.Free();
        }
    }
}

namespace WpfTestApp {
    public class MultiStateBehaviourViewModel : Helpers.ObservableObject {
        public HelpersV4.Collections.MultiStateContainer<ProjectCommonState, LoadedProject, UnloadedProject> ProjectMultiState { get; }
        public object? ProjectObj => this.ProjectMultiState.Current;

        
        States.MultistateBehaviour.LoadedProject? _lastLoadedProject;
        public States.MultistateBehaviour.LoadedProject? LastLoadedProject => _lastLoadedProject;

        
        States.MultistateBehaviour.UnloadedProject? _lastUnloadedProject;
        public States.MultistateBehaviour.UnloadedProject? LastUnloadedProject => _lastUnloadedProject;


        public ICommand OnUnloadProjectCommand { get; }
        public ICommand OnReloadProjectCommand { get; }

        public MultiStateBehaviourViewModel() {
            this.OnUnloadProjectCommand = new Helpers.RelayCommand<object>(this.OnUnloadProject);
            this.OnReloadProjectCommand = new Helpers.RelayCommand<object>(this.OnReloadProject);

            this.ProjectMultiState = new(
                new ProjectCommonState("vsHierarchy_0"),
                commonState => new LoadedProject(commonState),
                commonState => new UnloadedProject(commonState)
                );
            this.OnReloadProjectCommand.Execute(null);
        }


        private void OnUnloadProject(object parameter) {
            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.UnloadedProject>();
            base.OnPropertyChanged(nameof(this.ProjectObj));
            
            _lastUnloadedProject = this.ProjectMultiState.Current as States.MultistateBehaviour.UnloadedProject;
            base.OnPropertyChanged(nameof(this.LastUnloadedProject));
        }

        private void OnReloadProject(object parameter) {
            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.LoadedProject>();
            base.OnPropertyChanged(nameof(this.ProjectObj));

            _lastLoadedProject = this.ProjectMultiState.Current as States.MultistateBehaviour.LoadedProject;
            base.OnPropertyChanged(nameof(this.LastLoadedProject));
        }
    }


    public partial class MainWindow_6_MultiStateBehaviour : Window {

        private MultiStateBehaviourViewModel _viewModel = new();

        public MainWindow_6_MultiStateBehaviour() {
            this.InitializeComponent();
            this.DataContext = _viewModel;
        }
    }
}