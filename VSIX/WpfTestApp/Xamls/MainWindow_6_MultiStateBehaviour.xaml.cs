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

[assembly: Helpers.Attributes.CodeAnalyzerEnableLogs]


namespace WpfTestApp {
    public class MultiStateBehaviourViewModel : Helpers.ObservableObject {
        public Helpers.MultiState.MultiStateContainer<ProjectCommonState, LoadedProject, UnloadedProject> ProjectMultiState { get; }
        public object? ProjectObj => this.ProjectMultiState.Current;

        
        States.MultistateBehaviour.LoadedProject? _lastLoadedProject;
        public States.MultistateBehaviour.LoadedProject? LastLoadedProject => _lastLoadedProject;

        
        States.MultistateBehaviour.UnloadedProject? _lastUnloadedProject;
        public States.MultistateBehaviour.UnloadedProject? LastUnloadedProject => _lastUnloadedProject;


        States.MultistateBehaviour.Document? _lastDocument;
        public States.MultistateBehaviour.Document? LastDocument => _lastDocument;



        public ICommand OnUnloadProjectCommand { get; }
        public ICommand OnReloadProjectCommand { get; }

        public MultiStateBehaviourViewModel() {
            this.OnUnloadProjectCommand = new Helpers.RelayCommand<object>(this.OnUnloadProject);
            this.OnReloadProjectCommand = new Helpers.RelayCommand<object>(this.OnReloadProject);

            this.ProjectMultiState = new(new ProjectCommonState("vsHierarchy_0"));
            this.OnReloadProjectCommand.Execute(null);
        }


        private void OnUnloadProject(object parameter) {
            _lastDocument = _lastLoadedProject?.DocumentObj as States.MultistateBehaviour.Document;

            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.UnloadedProject>();
            _lastUnloadedProject = this.ProjectMultiState.Current as States.MultistateBehaviour.UnloadedProject;

            base.OnPropertyChanged(nameof(this.ProjectObj));
            base.OnPropertyChanged(nameof(this.LastUnloadedProject));
            base.OnPropertyChanged(nameof(this.LastLoadedProject));
            base.OnPropertyChanged(nameof(this.LastDocument));
        }

        private void OnReloadProject(object parameter) {
            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.LoadedProject>();
            _lastLoadedProject = this.ProjectMultiState.Current as States.MultistateBehaviour.LoadedProject;

            base.OnPropertyChanged(nameof(this.ProjectObj));
            base.OnPropertyChanged(nameof(this.LastUnloadedProject));
            base.OnPropertyChanged(nameof(this.LastLoadedProject));
            base.OnPropertyChanged(nameof(this.LastDocument));
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