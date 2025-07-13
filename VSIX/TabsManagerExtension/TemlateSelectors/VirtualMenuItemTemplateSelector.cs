using System;
using System.Windows;
using System.Windows.Controls;


namespace TabsManagerExtension.TemlateSelectors {
    public class VirtualMenuItemTemplateSelector : Controls.MenuControl.MenuItemTemplateSelectorBase {
        public DataTemplate? DocumentProjectReferenceCommandTemplate { get; set; }
        public DataTemplate? ReloadDocumentReferencesProjectsCommandTemplate { get; set; }

        public override DataTemplate? SelectTemplate(object item, DependencyObject container) {
            if (item is Helpers.MenuItemCommand menuItemCommand) {
                if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferencesInfo.RefEntry refEntry) {
                    return this.DocumentProjectReferenceCommandTemplate;
                }
                else if (menuItemCommand.CommandParameterContext is State.Document.DocumentProjectReferencesInfo documentProjectReferencesInfo) {
                    return this.ReloadDocumentReferencesProjectsCommandTemplate;
                }
            }
            return base.SelectTemplate(item, container);
        }
    }
}