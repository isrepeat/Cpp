using System;


namespace Helpers {
    public enum AssertStatus {
        Successed,
        Failed,
    }

    public static class Assert {
        //[Conditional("DEBUG")]
        public static Result Require(bool condition, string message = "Assertion failed") {
            if (!condition) {
                Assert.HandleFailure(message);
                return new Result(AssertStatus.Failed);
            }
            return new Result(AssertStatus.Successed);
        }

        public static void Unexpected(string message = "Assertion failed") {
            Assert.HandleFailure(message);
        }

        private static void HandleFailure(string message) {
            Helpers.Diagnostic.Logger.LogError($"[ASSERT] {message}");
            System.Diagnostics.Debugger.Break();
        }

        public class Result {
            public bool Successed => _assertStatus == AssertStatus.Successed;
            public bool Failed => _assertStatus == AssertStatus.Failed;

            private AssertStatus _assertStatus;
            public Result(AssertStatus assertStatus) {
                _assertStatus = assertStatus;
            }

            public static bool operator ==(Result result, AssertStatus status) {
                return result?._assertStatus == status;
            }

            public static bool operator !=(Result result, AssertStatus status) {
                return !(result == status);
            }

            public static bool operator ==(AssertStatus status, Result result) {
                return result == status;
            }

            public static bool operator !=(AssertStatus status, Result result) {
                return !(result == status);
            }

            public override bool Equals(object? obj) {
                if (obj is Result other) {
                    return _assertStatus == other._assertStatus;
                }
                return false;
            }

            public override int GetHashCode() {
                return _assertStatus.GetHashCode();
            }
        }
    }


    public static class ThrowableAssert {
        public static void Require(bool condition, string message = "Assertion failed") {
            if (!condition) {
                ThrowableAssert.HandleFailure(message);
            }
        }

        public static void Unexpected(string message = "Assertion failed") {
            ThrowableAssert.HandleFailure(message);
        }

        private static void HandleFailure(string message) {
            Helpers.Diagnostic.Logger.LogError($"[ASSERT] {message}");
            System.Diagnostics.Debugger.Break();
            throw new AssertionException(message);
        }
    }

    public class AssertionException : Exception {
        public AssertionException(string message)
            : base(message) {
        }
    }
}