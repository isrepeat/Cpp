using System;
using System.Windows;
using System.Windows.Controls;


namespace TabsManagerExtension.TemlateSelectors {
    public class AdditionalContextWrapper<TBase, TAdditional>
    where TBase : class {
        public TBase BaseContext { get; }
        public TAdditional AdditionalContext { get; }

        public AdditionalContextWrapper(TBase baseContext, TAdditional additionalContext) {
            this.BaseContext = baseContext ?? throw new ArgumentNullException(nameof(baseContext));
            this.AdditionalContext = additionalContext;
        }
    }

    public class VirtualMenuItemTemplateSelector : Controls.MenuControl.MenuItemTemplateSelectorBase {
        public DataTemplate? LoadedProjectsCommandTemplate { get; set; }
        public DataTemplate? UnloadedProjectsCommandTemplate { get; set; }

        public override DataTemplate? SelectTemplate(object item, DependencyObject container) {
            if (item is Helpers.MenuItemCommand menuItemCommand) {
                if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferenceInfo projRefEntry) {

                    if (projRefEntry.SolutionProjectNode.IsLoaded) {
                        return this.LoadedProjectsCommandTemplate;
                    }
                    else {
                        return this.UnloadedProjectsCommandTemplate;
                    }

                        //if (!projRefEntry.SolutionProjectNode.IsSharedProject) {
                        //    if (projRefEntry.SolutionProjectNode.ProjectNodeObj is VsShell.Project.LoadedProjectNode loadedProjectNode) {
                        //        if (loadedProjectNode.IsIncludeSharedItems) {
                        //            ////if (container is FrameworkElement fe) {
                        //            ////    var compositeContext = new AdditionalContextWrapper<Helpers.MenuItemCommand, VsShell.Project.LoadedProjectNode>(
                        //            ////        menuItemCommand,
                        //            ////        loadedProjectNode);

                        //            ////    fe.DataContext = compositeContext;
                        //            ////}

                        //            //if (container is FrameworkElement fe) {
                        //            //    fe.Tag = loadedProjectNode;
                        //            //}

                        //            return this.ProjectThatInlcludeSharedItemsCommandTemplate;
                        //        }
                        //    }
                        //}
                    }
            }

            return base.SelectTemplate(item, container);
        }
    }
}