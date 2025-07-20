using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using Helpers.Attributes;
using System.ComponentModel;


namespace WpfTestApp.States.MultistateBehaviour {
    public class DocumentCommonState :
        HelpersV3.Collections.CommonStateBase,
        IDisposable {

        public void Dispose() {
        }
    }



    public abstract class DocumentCommonStateViewModel :
        Helpers.ObservableObject,
        IDisposable {

        protected DocumentCommonState CommonState { get; }
        protected DocumentCommonStateViewModel(DocumentCommonState commonState) {
            this.CommonState = commonState;
            this.CommonState.SharedStatePropertyChanged += this.OnSharedStatePropertyChanged;
        }
        public virtual void Dispose() {
            this.CommonState.SharedStatePropertyChanged -= this.OnSharedStatePropertyChanged;
        }

        private void OnSharedStatePropertyChanged(object? sender, PropertyChangedEventArgs e) {
        }
    }



    public class Document :
        DocumentCommonStateViewModel,
        HelpersV3.Collections.IMultiStateElement {

        public Document(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e) {
            if (e.NextState is InvalidatedDocument) {   
            }
        }

        public override string ToString() {
            return $"<Document>";
        }
    }



    public class InvalidatedDocument :
        DocumentCommonStateViewModel,
        HelpersV3.Collections.IMultiStateElement {

        public InvalidatedDocument(DocumentCommonState commonState) : base(commonState) {
        }
        public override void Dispose() {
        }

        public void OnStateEnabled(HelpersV3._EventArgs.MultiStateElementEnabledEventArgs e) {
        }

        public void OnStateDisabled(HelpersV3._EventArgs.MultiStateElementDisabledEventArgs e) {
        }

        public override string ToString() {
            return $"<InvalidatedDocument>";
        }
    }
}