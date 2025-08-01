using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using TabsManagerExtension.VsShell.Project;


namespace TabsManagerExtension.State.Document {
    public interface IActivatableTab {
        void Activate();
    }

    public class TabItemDocument : TabItemBase, IActivatableTab {
        public VsShell.Document.ShellDocument ShellDocument { get; private set; }
        public DocumentProjectReferencesInfo DocumentProjectReferencesInfo { get; }
        //public VsShell.Project.ProjectNode ProjectNodeContext { get; set; }

        public TabItemDocument(VsShell.Document.ShellDocument shellDocument) {
            ThreadHelper.ThrowIfNotOnUIThread();

            base.Caption = shellDocument.Document.Name;
            base.FullName = shellDocument.Document.FullName;
            this.ShellDocument = shellDocument;
            this.DocumentProjectReferencesInfo = new DocumentProjectReferencesInfo(base.FullName);
        }

        public TabItemDocument(EnvDTE.Document document)
            : this(new VsShell.Document.ShellDocument(document)) {
        }

        public void Activate() {
            ThreadHelper.ThrowIfNotOnUIThread();

            try {
                this.ShellDocument.Document?.Activate();
            }
            catch (COMException ex) {
                Helpers.Diagnostic.Logger.LogWarning($"Failed to activate document '{this.Caption}': {ex.Message}");
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"Unexpected error activating document '{this.Caption}': {ex.Message}");
            }
        }


        public override string ToString() {
            //return $"TabItemDocument(FullName='{this.FullName}', ProjectCtx='{this.ProjectNodeContext}')";
            return $"TabItemDocument(FullName='{this.FullName}'')";
        }
    }
}