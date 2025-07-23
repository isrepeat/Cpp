using System;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;


namespace Helpers.Reflaction {
    public static class FieldsUtils {
        public static HashSet<T> CollectAllStaticFieldsOfType<T>(params Type[] targetTypes) {
            var flags = BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy;
            var result = new HashSet<T>();

            foreach (var targetType in targetTypes) {
                foreach (var type in FieldsUtils.GetAllNestedTypesIncludingSelf(targetType)) {
                    var fields = type.GetFields(flags)
                        .Where(f => f.FieldType == typeof(T))
                        .Select(f => (T)f.GetValue(null)!);

                    foreach (var item in fields) {
                        result.Add(item);
                    }
                }
            }

            return result;
        }

        //public static IReadOnlyDictionary<string, T> BuildIdMap<T>(IEnumerable<T> items, Func<T, string> idSelector) {
        //    return items.ToDictionary(idSelector);
        //}


        private static List<Type> GetAllNestedTypesIncludingSelf(Type root) {
            var result = new List<Type>();
            FieldsUtils.CollectRecursive(root, result);
            return result;
        }

        private static void CollectRecursive(Type type, List<Type> result) {
            result.Add(type);

            foreach (var nested in type.GetNestedTypes(BindingFlags.Public | BindingFlags.Static)) {
                FieldsUtils.CollectRecursive(nested, result);
            }
        }
    }
}