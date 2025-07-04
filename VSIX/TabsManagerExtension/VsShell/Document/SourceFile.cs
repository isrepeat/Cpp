using System;
using System.IO;
using System.Collections.Generic;


namespace TabsManagerExtension.VsShell.Document {
    public class SourceFile {
        public string FilePath { get; }
        public string ProjectId { get; }
        public State.Document.TabItemProject Project { get; }

        public SourceFile(string filePath, State.Document.TabItemProject project) {
            this.FilePath = filePath;
            this.Project = project;
            this.ProjectId = project.ShellProject.Project.UniqueName;
        }

        public override bool Equals(object? obj) {
            return obj is SourceFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.ProjectId, other.ProjectId);
        }

        public override int GetHashCode() {
            int h1 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath);
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.ProjectId);
            return (h1 * 397) ^ h2; // HashCode.Combine(h1, h2);
        }

        public override string ToString() => $"{this.FilePath} [{this.ProjectId}]";
    }
}