using System;
using System.Collections.Generic;


namespace TabsManagerExtension.Services {
    public interface IExtensionService {
        void Initialize();
        void Shutdown();
    }


    /// <summary>
    /// Потокобезопасная реализация singleton-сервиса с управляемым созданием экземпляра.
    /// Используется как базовый класс для IExtensionService-сервисов.
    /// </summary>
    public abstract class SingletonServiceBase<T> where T : SingletonServiceBase<T>, IExtensionService, new() {
        private static readonly object _syncRoot = new();
        private static T? _instance;

        /// <summary>
        /// Единственный экземпляр сервиса. Должен быть создан через Create().
        /// </summary>
        public static T Instance {
            get {
                lock (_syncRoot) {
                    return _instance ?? throw new InvalidOperationException($"{typeof(T).Name} must be initialized via Create().");
                }
            }
        }

        /// <summary>
        /// Создаёт экземпляр singleton-сервиса. Должно вызываться только один раз.
        /// </summary>
        public static T Create() {
            lock (_syncRoot) {
                if (_instance != null) {
                    throw new InvalidOperationException($"{typeof(T).Name} already created.");
                }

                _instance = new T();
                return _instance;
            }
        }

        /// <summary>
        /// Сбрасывает ссылку на singleton после Shutdown (опционально).
        /// </summary>
        protected static void ClearInstance() {
            lock (_syncRoot) {
                _instance = null;
            }
        }
    }


    /// <summary>
    /// Централизованный менеджер глобальных сервисов расширения.
    /// Инициализирует и завершает работу всех IExtensionService-синглтонов.
    /// </summary>
    public static class ExtensionServices {
        private static readonly List<IExtensionService> _services = new() {
            VsShell.Services.VsSelectionTrackerService.Create(),
            VsShell.TextEditor.Services.TextEditorCommandFilterService.Create(),
            VsShell.TextEditor.Services.DocumentActivationTrackerService.Create(),
        };

        private static bool _isInitialized;

        /// <summary>
        /// Инициализирует все сервисы один раз.
        /// </summary>
        public static void Initialize() {
            if (_isInitialized) {
                return;
            }

            foreach (var service in _services) {
                service.Initialize();
            }

            _isInitialized = true;
        }

        /// <summary>
        /// Завершает работу всех сервисов.
        /// </summary>
        public static void Shutdown() {
            if (!_isInitialized) {
                return;
            }

            foreach (var service in _services) {
                service.Shutdown();
            }

            _isInitialized = false;
        }
    }
}