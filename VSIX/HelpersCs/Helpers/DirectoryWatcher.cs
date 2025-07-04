using System;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.IO;

namespace Helpers {
    public enum DirectoryChangeType {
        Changed,
        Created,
        Deleted,
        Renamed
    }

    public class DirectoryChangedEventArgs : EventArgs {
        public DirectoryChangeType ChangeType { get; }
        public string FullPath { get; }
        public string? OldPath { get; }

        public DirectoryChangedEventArgs(DirectoryChangeType changeType, string fullPath, string? oldPath = null) {
            this.ChangeType = changeType;
            this.FullPath = fullPath;
            this.OldPath = oldPath;
        }

        public override bool Equals(object? obj) {
            return obj is DirectoryChangedEventArgs other &&
                   this.ChangeType == other.ChangeType &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FullPath, other.FullPath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.OldPath, other.OldPath);
        }

        public override int GetHashCode() {
            int h1 = this.ChangeType.GetHashCode();
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FullPath);
            int h3 = this.OldPath != null ? StringComparer.OrdinalIgnoreCase.GetHashCode(this.OldPath) : 0;
            return ((h1 * 397) ^ h2) * 397 ^ h3;
        }
    }

    public class DirectoryWatcher : IDisposable {
        private readonly FileSystemWatcher _fileSystemWatcher;

        public event Action<DirectoryChangedEventArgs>? DirectoryChanged;

        public DirectoryWatcher(string directoryPath, string filter = "*.*", bool includeSubdirectories = true) {
            if (!Directory.Exists(directoryPath)) {
                throw new DirectoryNotFoundException($"Directory not found: {directoryPath}");
            }

            _fileSystemWatcher = new FileSystemWatcher(directoryPath) {
                Filter = filter,
                IncludeSubdirectories = includeSubdirectories,
                NotifyFilter = NotifyFilters.Size | NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.DirectoryName
            };

            _fileSystemWatcher.Created += this.OnFileCreated;
            _fileSystemWatcher.Changed += this.OnFileChanged;
            _fileSystemWatcher.Renamed += this.OnFileRenamed;
            _fileSystemWatcher.Deleted += this.OnFileDeleted;
            _fileSystemWatcher.EnableRaisingEvents = true;
        }


        private void OnFileCreated(object sender, FileSystemEventArgs e) {
            this.DirectoryChanged?.Invoke(new DirectoryChangedEventArgs(DirectoryChangeType.Created, e.FullPath));
        }
        private void OnFileChanged(object sender, FileSystemEventArgs e) {
            this.DirectoryChanged?.Invoke(new DirectoryChangedEventArgs(DirectoryChangeType.Changed, e.FullPath));
        }
        private void OnFileRenamed(object sender, RenamedEventArgs e) {
            this.DirectoryChanged?.Invoke(new DirectoryChangedEventArgs(DirectoryChangeType.Renamed, e.FullPath, e.OldFullPath));
        }
        private void OnFileDeleted(object sender, FileSystemEventArgs e) {
            this.DirectoryChanged?.Invoke(new DirectoryChangedEventArgs(DirectoryChangeType.Deleted, e.FullPath));
        }

        public void Dispose() {
            _fileSystemWatcher.Deleted -= this.OnFileDeleted;
            _fileSystemWatcher.Renamed -= this.OnFileRenamed;
            _fileSystemWatcher.Changed -= this.OnFileChanged;
            _fileSystemWatcher.Created -= this.OnFileCreated;
            _fileSystemWatcher.Dispose();
        }
    }
}