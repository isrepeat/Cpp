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
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.ToolWindows {
    internal sealed class TabsManagerToolWindowCommand {
        public const int CommandId = 0x0100;

        public static readonly Guid CommandSet = new Guid("8a30806a-edfc-4c91-8182-025665145a07");

        private readonly AsyncPackage package;

        /// <summary>
        /// Initializes a new instance of the <see cref="TabsManagerToolWindowCommand"/> class.
        /// Adds our command handlers for menu (commands must exist in the command table file)
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        /// <param name="commandService">Command service to add command to, not null.</param>
        private TabsManagerToolWindowCommand(AsyncPackage package, OleMenuCommandService commandService) {
            this.package = package ?? throw new ArgumentNullException(nameof(package));
            commandService = commandService ?? throw new ArgumentNullException(nameof(commandService));

            var menuCommandID = new CommandID(CommandSet, CommandId);
            var menuItem = new MenuCommand(this.Execute, menuCommandID);
            commandService.AddCommand(menuItem);
        }

        public static TabsManagerToolWindowCommand Instance {
            get;
            private set;
        }

        private Microsoft.VisualStudio.Shell.IAsyncServiceProvider ServiceProvider {
            get {
                return this.package;
            }
        }

        /// <summary>
        /// Initializes the singleton instance of the command.
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        public static async Task InitializeAsync(AsyncPackage package) {
            // Switch to the main thread - the call to AddCommand in TabsManagerToolWindowCommand's constructor requires
            // the UI thread.
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(package.DisposalToken);

            OleMenuCommandService commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
            Instance = new TabsManagerToolWindowCommand(package, commandService);
        }


        private async void Execute(object sender, EventArgs e) {
            //VsixVisualTreeHelper.Instance.ToggleCustomTabs();
            //this.TestIncludeDependencyAnalyzer();

            //var externalDependenciesAnalyzer = VsShell.Solution.Services.ExternalDependenciesAnalyzerService.Instance;
            //externalDependenciesAnalyzer.LogAllItemIdsViaHierarchy();
            //externalDependenciesAnalyzer.Analyze();


            //string engineProjectName = "Engine\\Engine.vcxproj";
            //string engineCppFilepath = "d:\\WORK\\TEST\\Extensions\\TestIncludeSolution\\Engine\\Engine.cpp";

            //string gameProjectName = "Game\\Game.vcxproj";
            //string gameCppFilepath = "d:\\WORK\\TEST\\Extensions\\TestIncludeSolution\\Game\\Game.cpp";

            //string includeFilepath = "d:\\WORK\\TEST\\Extensions\\TestIncludeSolution\\Helpers.Shared\\Logger.h";

            ////var nodes = graphService.Includes[includeFilepath];
            ////nodes.FirstOrDefault()?.Open();
            
            //{
            //    var project = graphService.Projects["Game\\Game.vcxproj"];
            //    var externalIncludeNode = project.Includes.FirstOrDefault(i => String.Equals(i.FilePath, includeFilepath, StringComparison.OrdinalIgnoreCase));
            //    externalIncludeNode?.Open();
            //}
            ////await Task.Delay(3000);
            ////{
            ////    var project = graphService.Projects["Engine\\Engine.vcxproj"];
            ////    var externalIncludeNode = project.Includes.FirstOrDefault(i => String.Equals(i.FilePath, includeFilepath, StringComparison.OrdinalIgnoreCase));
            ////    externalIncludeNode?.Open();
            ////}

            ////graphService.OpenIncludeInProjectContext(gameProjectName, includeFilepath);
            ////graphService.OpenIncludeInProjectContext(engineProjectName, includeFilepath);

            int xx = 9;
        }


        private void TestIncludeDependencyAnalyzer() {
            var analyzer = VsShell.Solution.Services.IncludeDependencyAnalyzerService.Instance;
            if (!analyzer.IsReady()) {
                return;
            }

            //analyzer.Build();

            string includeTaget = "Logger.h";
            //string includeTaget = "RenderPipeline.h";

            var transitiveIncludingFiles = analyzer.GetTransitiveFilesIncludersByIncludeString(includeTaget);
            var transitiveIncludingProjects = analyzer.GetTransitiveProjectsIncludersByIncludeString(includeTaget);

            string includeTagetFullName = "d:\\WORK\\TEST\\Extensions\\TestIncludeSolution\\Helpers.Shared\\Logger.h";
            var transitiveIncludingFiles2 = analyzer.GetTransitiveFilesIncludersByIncludePath(includeTagetFullName);
            var transitiveIncludingProjects2 = analyzer.GetTransitiveProjectsIncludersByIncludePath(includeTagetFullName);

            int xx = 9;
        }
    }        
}