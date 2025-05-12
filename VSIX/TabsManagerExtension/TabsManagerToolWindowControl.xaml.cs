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
    public partial class TabsManagerToolWindowControl : UserControl, INotifyPropertyChanged {
        private DTE2 _dte;
        private DTEEvents _dteEvents;
        private WindowEvents _windowEvents;
        private SolutionEvents _solutionEvents;

        private const string SaveFilePath = "D:\\TabGroups.json";
        public ObservableCollection<string> GroupNames { get; set; } = new ObservableCollection<string>();
        public Dictionary<string, List<string>> TabGroups { get; set; } = new Dictionary<string, List<string>>();
        

        private string _selectedGroup;
        public string SelectedGroup {
            get => _selectedGroup;
            set {
                _selectedGroup = value;
                OnPropertyChanged();
                UpdateTabList();
            }
        }

        public ObservableCollection<string> SelectedGroupTabs { get; set; } = new ObservableCollection<string>();
        public string NewGroupName { get; set; }

        private DocumentEvents _documentEvents;
        private bool isVisualStudioClosing = false;
        private IntPtr mainWindowHandle;

        public TabsManagerToolWindowControl() {
            InitializeComponent();
            InitializeDTE();
            InitializeOpenDocuments();
            DataContext = this;
            LoadGroups();
        }


        private void InitializeDTE() {
            ThreadHelper.ThrowIfNotOnUIThread();
            _dte = (DTE2)Package.GetGlobalService(typeof(DTE));

            // Подключаемся к событиям
            _documentEvents = _dte.Events.DocumentEvents;
            _documentEvents.DocumentClosing += DocumentClosingHandler;

            _solutionEvents = _dte.Events.SolutionEvents;
            _solutionEvents.BeforeClosing += OnSolutionClosing;
        }
        private void InitializeOpenDocuments() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var openDocuments = _dte.Documents.Cast<Document>().ToList();

            foreach (var doc in openDocuments) {
                try {
                    // Принудительно активируем каждый документ (инициализация)
                    doc.Activate();
                }
                catch {
                    // Игнорируем ошибки (возможно, документ недоступен)
                }
            }
        }


        private void CreateGroup_Click(object sender, RoutedEventArgs e) {
            if (!string.IsNullOrWhiteSpace(NewGroupName) && !TabGroups.ContainsKey(NewGroupName)) {
                TabGroups[NewGroupName] = new List<string>();
                GroupNames.Add(NewGroupName); // Обновляем ObservableCollection
                SaveGroups();
            }
        }

        private void AddActiveTabToGroup_Click(object sender, RoutedEventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();
            if (!string.IsNullOrWhiteSpace(SelectedGroup) && TabGroups.ContainsKey(SelectedGroup)) {
                var activeDocument = _dte.ActiveDocument;
                if (activeDocument != null) {
                    string docName = activeDocument.Name;
                    if (!TabGroups[SelectedGroup].Contains(docName)) {
                        TabGroups[SelectedGroup].Add(docName);
                        UpdateTabList();
                        SaveGroups();
                    }
                }
            }
        }



        private void RemoveTabFromGroup_Click(object sender, RoutedEventArgs e) {
            if (!string.IsNullOrWhiteSpace(SelectedGroup) && TabGroups.ContainsKey(SelectedGroup)) {
                if (TabList.SelectedItem is string selectedTab) {
                    TabGroups[SelectedGroup].Remove(selectedTab);
                    UpdateTabList();
                    SaveGroups();
                }
            }
        }

        private void UpdateTabList() {
            ThreadHelper.ThrowIfNotOnUIThread();
            SelectedGroupTabs.Clear();

            if (SelectedGroup != null && TabGroups.ContainsKey(SelectedGroup)) {
                // Получаем все открытые документы
                var openDocuments = _dte.Documents.Cast<Document>().ToList();
                var openDocumentNames = openDocuments.Select(doc => doc.Name).ToList();

                foreach (var tabName in TabGroups[SelectedGroup]) {
                    if (openDocumentNames.Contains(tabName)) {
                        // Вкладка открыта, отображаем её имя
                        SelectedGroupTabs.Add(tabName);
                    }
                    else {
                        // Вкладка закрыта, отображаем как [Закрыто] Имя
                        SelectedGroupTabs.Add($"[Закрыто] {tabName}");
                    }
                }
            }
        }

        private void SaveGroups() {
            try {
                // Создаем директорию, если она не существует
                var directory = Path.GetDirectoryName(SaveFilePath);
                if (!Directory.Exists(directory)) {
                    Directory.CreateDirectory(directory);
                }

                // Записываем данные в файл JSON
                string json = JsonConvert.SerializeObject(TabGroups, Formatting.Indented);
                File.WriteAllText(SaveFilePath, json);
            }
            catch (Exception ex) {
                MessageBox.Show($"Ошибка при сохранении групп вкладок: {ex.Message}");
            }
        }

        private void LoadGroups() {
            try {
                // Проверяем, существует ли файл
                if (File.Exists(SaveFilePath)) {
                    string json = File.ReadAllText(SaveFilePath);
                    if (!string.IsNullOrWhiteSpace(json)) {
                        TabGroups = JsonConvert.DeserializeObject<Dictionary<string, List<string>>>(json)
                                    ?? new Dictionary<string, List<string>>();
                    }
                }
                else {
                    TabGroups = new Dictionary<string, List<string>>();
                }

                // Обновляем список групп
                GroupNames.Clear();
                foreach (var group in TabGroups.Keys) {
                    GroupNames.Add(group);
                }

                // Обновляем список вкладок для выбранной группы
                UpdateTabList();
            }
            catch (Exception ex) {
                MessageBox.Show($"Ошибка при загрузке групп вкладок: {ex.Message}");
            }
        }


        private void OnSolutionClosing() {
            ThreadHelper.ThrowIfNotOnUIThread();
            isVisualStudioClosing = true;

            // Принудительно сохраняем группы до закрытия документов
            SaveGroups();
        }


        private void DocumentClosingHandler(Document document) {
            ThreadHelper.ThrowIfNotOnUIThread();

            // Проверяем, закрывается ли Visual Studio
            if (isVisualStudioClosing) 
                return;

            // Получаем имя закрываемого документа
            string docName = document.Name;

            // Проверяем все группы и удаляем документ
            bool updated = false;
            foreach (var group in TabGroups.Keys.ToList()) {
                if (TabGroups[group].Contains(docName)) {
                    TabGroups[group].Remove(docName);
                    updated = true;
                }
            }

            // Если были изменения, сохраняем и обновляем UI
            if (updated) {
                SaveGroups();
                if (SelectedGroup != null) {
                    UpdateTabList();
                }
            }
        }


        // Реализация INotifyPropertyChanged для автоматического обновления UI
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}