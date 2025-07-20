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


namespace WpfTestApp {
    public class MultiStateBehaviourViewModel : Helpers.ObservableObject {

        public HelpersV3.Collections.MultiStateContainer<ProjectCommonState, LoadedProject, UnloadedProject> ProjectMultiState { get; }

        States.MultistateBehaviour.LoadedProject? _lastLoadedPrject;
        public States.MultistateBehaviour.LoadedProject? LastLoadedProject {
            get {
                return _lastLoadedPrject;
            }
        }

        public object? ProjectObj => this.ProjectMultiState.Current;


        public ICommand OnUnloadProjectCommand { get; }
        public ICommand OnReloadProjectCommand { get; }

        public MultiStateBehaviourViewModel() {
            this.OnUnloadProjectCommand = new Helpers.RelayCommand<object>(this.OnUnloadProject);
            this.OnReloadProjectCommand = new Helpers.RelayCommand<object>(this.OnReloadProject);

            this.ProjectMultiState = new();
            this.OnReloadProjectCommand.Execute(null);
        }


        private void OnUnloadProject(object parameter) {
            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.UnloadedProject>();
            base.OnPropertyChanged(nameof(this.ProjectObj));
        }

        private void OnReloadProject(object parameter) {
            this.ProjectMultiState.SwitchTo<States.MultistateBehaviour.LoadedProject>();
            base.OnPropertyChanged(nameof(this.ProjectObj));

            _lastLoadedPrject = this.ProjectMultiState.Current as States.MultistateBehaviour.LoadedProject;
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