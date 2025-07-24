using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Helpers.Attributes;
using System.Windows.Media.Animation;


namespace WpfTestApp.States.MultistateBehaviour2 {
    public class DocumentMultiStateContainer :
        HelpersV4.Collections.MultiStateContainer<
            DocumentCommonState,
            Document,
            InvalidatedDocument> {
        public DocumentMultiStateContainer(string vsHierarchyData)
            : base(new DocumentCommonState(vsHierarchyData)) {
        }
    }



    public partial class DocumentCommonState :
        HelpersV4.Collections.CommonStateBase,
        IDisposable {

        public DocumentCommonState(string vsHierarchyData) {
        }

        public void Dispose() {
        }
    }



    public abstract class DocumentCommonStateViewModel :
        HelpersV4.Collections.CommonStateViewModelBase<DocumentCommonState> {

        protected DocumentCommonStateViewModel(DocumentCommonState commonState)
            : base(commonState) {
        }

        protected override void OnCommonStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
        }
    }


    public class App {
        private LoadedProject _loadedProject;
        public App() {
            _loadedProject = new LoadedProject();
            // ... switch to UnloadedProject

            //_loadedProject.Documents[0].Instance
        }
    }


    public class LoadedProject {
        public List<Multistate<Document>> Documents => _documents;

        private List<Multistate<Document>> _documents = new();
        private List<ExternalInclude> _externalIncludes = new();
        public LoadedProject() {
            _documents.Add(new Multistate<Document>(new Document()));
        }
    }


    public class Multistate<T>
        where T : HelpersV4.Collections.IMultiStateElement {

        public T Instance { get; }

        public Multistate(T instance) {
            this.Instance = instance;
        }
    }


    public class Document :
        HelpersV4.Collections.IMultiStateElement {

        public Document() {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
            //base.CommonState.HierarchyMultiState.SwitchTo<HierarchyItem>();
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {
                //base.CommonState.HierarchyMultiState.SwitchTo<InvalidatedHierarchyItem>();
            }
        }

        public override string ToString() {
            return $"<Document>";
        }
    }


    public class ExternalInclude : Document {
        public ExternalInclude() : base() {
        }
        public override string ToString() {
            return $"<ExternalInclude>";
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV4.Collections.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV4._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV4._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument>";
        }
    }
}