using System;
using System.Linq;
using System.Threading;
using System.Collections.Generic;


namespace TabsManagerExtension.Services {
    public interface IExtensionService {
        /// <summary>
        /// Список типов сервисов, от которых зависит данный.
        /// </summary>
        IReadOnlyList<Type> DependsOn();
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
        private static bool _isInitialized;
        private static readonly Dictionary<Type, IExtensionService> _services = new();

        private static int _activeUserCount = 0;
        private static bool _shutdownRequested = false;
        private static readonly object _sync = new();

        public static void Initialize() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"ExtensionServices.Initialize()");

            if (_isInitialized) {
                Helpers.Diagnostic.Logger.LogDebug($"ExtensionServices already initialized, ignore.");
                return;
            }

            ExtensionServices.Register(VsShell.Services.VsSelectionEventsServiceBase<
                VsShell.Services.VsIDEStateFlagsTrackerService
                >.Create());

            ExtensionServices.Register(VsShell.Services.VsSelectionEventsServiceBase<
                VsShell.Solution.Services.VsWindowFrameActivationTrackerService
                >.Create());

            ExtensionServices.Register(VsShell.Services.VsSelectionEventsServiceBase<
                VsShell.Solution.Services.VsSolutionExplorerSelectionTrackerService
                >.Create());

            ExtensionServices.Register(VsShell.Solution.Services.VsProjectItemsTrackerService.Create());
            ExtensionServices.Register(VsShell.Solution.Services.VsSolutionEventsTrackerService.Create());
            ExtensionServices.Register(VsShell.Solution.Services.IncludeDependencyAnalyzerService.Create());
            ExtensionServices.Register(VsShell.TextEditor.Services.TextEditorCommandFilterService.Create());
            ExtensionServices.Register(VsShell.TextEditor.Services.DocumentActivationTrackerService.Create());

            foreach (var service in _services.Values) {
                service.Initialize();
            }

            _isInitialized = true;
        }


        public static void RequestShutdown() {
            lock (_sync) {
                if (_activeUserCount == 0) {
                    Shutdown();
                }
                else {
                    _shutdownRequested = true;
                    Helpers.Diagnostic.Logger.LogDebug($"[ExtensionServices] Shutdown requested, but deferred until all consumers are released.");
                }
            }
        }


        public static void BeginUsage() {
            Interlocked.Increment(ref _activeUserCount);
        }

        public static void EndUsage() {
            var newCount = Interlocked.Decrement(ref _activeUserCount);

            // Если был запрос на Shutdown и сейчас никого нет — запускаем
            if (newCount == 0) {
                lock (_sync) {
                    if (_shutdownRequested) {
                        _shutdownRequested = false;
                        Shutdown();
                    }
                }
            }
        }



        private static void Shutdown() {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"ExtensionServices.Shutdown()");

            if (!_isInitialized) {
                Helpers.Diagnostic.Logger.LogDebug($"ExtensionServices already disposed, ignore.");
                return;
            }

            // Лог всех зарегистрированных сервисов и их зависимостей
            LogServices($"\n[ExtensionServices] Registered services and dependencies:", _services.Values.ToList());

            var visited = new HashSet<Type>();
            var result = new List<IExtensionService>();

            foreach (var type in _services.Keys) {
                VisitForToposort(type, visited, result);
            }

            // Выполняем завершение в обратном порядке (зависимые — последними)
            var reversedResult = Enumerable.Reverse(result);

            // Лог порядка выключения
            LogServices($"\n[ExtensionServices] Shutdown order:", reversedResult.ToList());

            foreach (var service in reversedResult) {
                service.Shutdown();
            }

            _isInitialized = false;
        }


        private static void Register<T>(T instance) where T : IExtensionService {
            _services[typeof(T)] = instance;
        }


        private static void VisitForToposort(Type type, HashSet<Type> visited, List<IExtensionService> result) {
            if (visited.Contains(type)) {
                return;
            }

            visited.Add(type);

            if (_services.TryGetValue(type, out var service)) {
                foreach (var dep in service.DependsOn()) {
                    VisitForToposort(dep, visited, result);
                }

                result.Add(service);
            }
        }

        private static void LogServices(string title, List<IExtensionService> services) {
            Helpers.Diagnostic.Logger.LogDebug($"{title}");
            int i = 1;

            foreach (var service in services) {
                Helpers.Diagnostic.Logger.LogDebug($"{i++}. {service.GetType().Name}");

                foreach (var dep in service.DependsOn()) {
                    Helpers.Diagnostic.Logger.LogDebug($"    └─ depends on: {dep.Name}");
                }
            }
        }
    }
}