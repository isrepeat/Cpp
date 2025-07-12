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
        //public DataTemplate? LoadedProjectsCommandTemplate { get; set; }
        //public DataTemplate? UnloadedProjectsCommandTemplate { get; set; }
        public DataTemplate? ProjectReferenceCommandTemplate { get; set; }

        public override DataTemplate? SelectTemplate(object item, DependencyObject container) {
            if (item is Helpers.MenuItemCommand menuItemCommand) {
                if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferenceInfo projRefEntry) {
                    return this.ProjectReferenceCommandTemplate;

                    //if (projRefEntry.ProjectNode.IsLoaded) {
                    //    return this.LoadedProjectsCommandTemplate;
                    //}
                    //else {
                    //    return this.UnloadedProjectsCommandTemplate;
                    //}
                }
            }

            return base.SelectTemplate(item, container);
        }
    }
}