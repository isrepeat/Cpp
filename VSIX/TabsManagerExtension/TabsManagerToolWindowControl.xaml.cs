using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;

namespace TabsManagerExtension {
    public class DocumentInfo {
        public string DisplayName { get; set; }
        public string FullName { get; set; }
    }

    public partial class TabsManagerToolWindowControl : UserControl, INotifyPropertyChanged {
        private DTE2 _dte;
        private DTEEvents _dteEvents;
        private WindowEvents _windowEvents;
        private SolutionEvents _solutionEvents;
        private DocumentEvents _documentEvents;

        private const string SaveFilePath = "D:\\TabGroups.json";

        public ObservableCollection<DocumentInfo> OpenDocuments { get; set; } = new ObservableCollection<DocumentInfo>();
        
        private bool isVisualStudioClosing = false;

        public TabsManagerToolWindowControl() {
            InitializeComponent();
            InitializeDTE();
            DataContext = this;
            LoadOpenDocuments();
        }


        private void InitializeDTE() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (DTE2)Package.GetGlobalService(typeof(DTE));

            // Подключаемся к событиям
            _documentEvents = _dte.Events.DocumentEvents;
            _documentEvents.DocumentOpened += DocumentOpenedHandler;
            _documentEvents.DocumentClosing += DocumentClosingHandler;

            _solutionEvents = _dte.Events.SolutionEvents;
            _solutionEvents.BeforeClosing += OnSolutionClosing;
        }

        private void LoadOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();
            OpenDocuments.Clear();
            foreach (Document doc in _dte.Documents) {
                if (!OpenDocuments.Any(d => d.FullName == doc.FullName)) {
                    OpenDocuments.Add(new DocumentInfo {
                        DisplayName = doc.Name,
                        FullName = doc.FullName
                    });
                }
            }
        }

        private void DocumentOpenedHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, если документ уже в списке — игнорируем
            if (!OpenDocuments.Any(d => d.FullName == document.FullName)) {
                OpenDocuments.Add(new DocumentInfo {
                    DisplayName = document.Name,
                    FullName = document.FullName
                });
            }
        }

        private void DocumentClosingHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Удаляем все экземпляры документа (защита от дублирования)
            var docsToRemove = OpenDocuments.Where(d => d.FullName == document.FullName).ToList();
            foreach (var doc in docsToRemove) {
                OpenDocuments.Remove(doc);
            }
        }

        private void CloseDocument_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (sender is Button button && button.CommandParameter is string fullName) {
                var document = _dte.Documents.Cast<Document>().FirstOrDefault(doc => doc.FullName == fullName);
                document?.Close();
            }
        }

        private void DocumentList_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (DocumentList.SelectedItem is DocumentInfo docInfo) {
                var document = _dte.Documents.Cast<Document>().FirstOrDefault(d => d.FullName == docInfo.FullName);
                document?.Activate();
            }
        }


        private void OnSolutionClosing() {
            ThreadHelper.ThrowIfNotOnUIThread();
            isVisualStudioClosing = true;

            // Принудительно сохраняем группы до закрытия документов
            // ...
        }




        // Реализация INotifyPropertyChanged для автоматического обновления UI
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}