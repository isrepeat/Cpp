using System;
using System.IO;
using System.Collections.Generic;


namespace TabsManagerExtension.VsShell.Document {
    public class SourceFile {
        public string FilePath { get; }
        public VsShell.Project.LoadedProject LoadedProject { get; }

        public SourceFile(string filePath, VsShell.Project.LoadedProject loadedProject) {
            this.FilePath = filePath;
            this.LoadedProject = loadedProject;
        }

        public override bool Equals(object? obj) {
            return obj is SourceFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.LoadedProject.UniqueName, other.LoadedProject.UniqueName);
        }

        public override int GetHashCode() {
            int h1 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath);
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.LoadedProject.UniqueName);
            return (h1 * 397) ^ h2; // HashCode.Combine(h1, h2);
        }

        public override string ToString() {
            return $"{this.FilePath} [{this.LoadedProject.UniqueName}]";
        }
    }
}