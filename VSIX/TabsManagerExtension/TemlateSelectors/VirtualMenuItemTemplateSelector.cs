using System.Windows;
using System.Windows.Controls;


namespace TabsManagerExtension.TemlateSelectors {
    public class VirtualMenuItemTemplateSelector : Controls.MenuControl.MenuItemTemplateSelectorBase {
        public DataTemplate? SharedProjectCommandTemplate { get; set; }

        public override DataTemplate? SelectTemplate(object item, DependencyObject container) {
            if (item is Helpers.MenuItemCommand menuItemCommand) {
                if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferenceInfo projRefEntry) {
                    if (projRefEntry.ProjectNode.IsIncludeSharedItems && !projRefEntry.ProjectNode.IsSharedProject) {
                        return this.SharedProjectCommandTemplate;
                    }
                }
            }

            return base.SelectTemplate(item, container);
        }
    }
}