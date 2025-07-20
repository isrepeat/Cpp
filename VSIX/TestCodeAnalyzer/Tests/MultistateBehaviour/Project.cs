using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using Helpers.Attributes;


namespace TestCodeAnalyzer.Tests.MultistateBehaviour {
    public class ProjectNode :
        IDisposable {

        private readonly Helpers.Collections.MultiStateContainer<LoadedProject, UnloadedProject, ProjectSharedState> _projectNodeState;
        public Helpers.Collections.MultiStateContainer<LoadedProject, UnloadedProject, ProjectSharedState> ProjectNodeState => _projectNodeState;

        public ProjectNode() {
            _projectNodeState = new Helpers.Collections.MultiStateContainer<LoadedProject, UnloadedProject, ProjectSharedState>(
                new LoadedProject(),
                new UnloadedProject(),
                new ProjectSharedState()
                );
        }

        public void Dispose() {
            _projectNodeState.Dispose();
        }
    }



    public class ProjectSharedState {
        public DocumentNode DocumentNode { get; } = new();
    }


    public class LoadedProject :
        IDisposable,
        Helpers.Collections.IMultiStateElement<ProjectSharedState> {

        private ProjectSharedState? _sharedState = null;
        public Document? Document { get; private set; } = null;

        public LoadedProject() {
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<ProjectSharedState> e) {
            if (_sharedState == null) {
                _sharedState = e.SharedState;
            }

            // Schedule switch delayed ...

            //if (previousState is Helpers.Collections.UnknownMultiStateElement<ProjectSharedState>) {
            //    sharedState.DocumentNode.DocumentNodeState.SwitchTo<Document>();
            //}
            //this.Document = sharedState.DocumentNode.DocumentNodeState.Get<Document>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<ProjectSharedState> e) {
            if (e.NextState is UnloadedProject) {
                e.SharedState.DocumentNode.DocumentNodeState.SwitchTo<InvalidatedDocument>();
            }
        }


        // Delayed switch
        private void OnDocumentReceived() {
            var newDocument = new Document(null);
            _sharedState?.DocumentNode.DocumentNodeState.SwitchToNew<Document>(newDocument);
        }
    }


    public class UnloadedProject :
        IDisposable,
        Helpers.Collections.IMultiStateElement<ProjectSharedState> {
        public InvalidatedDocument? LastDocument { get; private set; } = null;

        public UnloadedProject() {
        }
        public void Dispose() {
        }

        public void OnStateEnabled(Helpers._EventArgs.MultiStateElementEnabledEventArgs<ProjectSharedState> e) {
            if (e.PreviousState is Helpers.Collections.UnknownMultiStateElement<ProjectSharedState>) {
                e.SharedState.DocumentNode.DocumentNodeState.SwitchTo<InvalidatedDocument>();
            }
            this.LastDocument = e.SharedState.DocumentNode.DocumentNodeState.Get<InvalidatedDocument>();
        }

        public void OnStateDisabled(Helpers._EventArgs.MultiStateElementDisabledEventArgs<ProjectSharedState> e) {
            if (e.NextState is LoadedProject) {
                e.SharedState.DocumentNode.DocumentNodeState.SwitchTo<Document>();
            }
        }
    }
}