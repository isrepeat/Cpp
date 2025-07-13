using System;
using System.Windows;
using System.Windows.Controls;


namespace TabsManagerExtension.TemlateSelectors {
    public class VirtualMenuItemTemplateSelector : Controls.MenuControl.MenuItemTemplateSelectorBase {
        public DataTemplate? ProjectReferenceCommandTemplate { get; set; }

        public override DataTemplate? SelectTemplate(object item, DependencyObject container) {
            if (item is Helpers.MenuItemCommand menuItemCommand) {
                if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferenceInfo projRefEntry) {
                    return this.ProjectReferenceCommandTemplate;
                }
            }

            return base.SelectTemplate(item, container);
        }
    }
}