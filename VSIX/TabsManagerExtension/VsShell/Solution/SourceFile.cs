using System;
using System.IO;
using System.Collections.Generic;


namespace TabsManagerExtension.VsShell.Solution {
    public class IncludeFile {
        public string RawInclude { get; }
        public string NormalizedName { get; }
        public IncludeFile(string rawInclude) {
            this.RawInclude = rawInclude;
            this.NormalizedName = Path.GetFileName(rawInclude);
        }

        public override string ToString() => this.NormalizedName;

        public override int GetHashCode() => StringComparer.OrdinalIgnoreCase.GetHashCode(this.NormalizedName);

        public override bool Equals(object? obj) {
            return obj is IncludeFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.NormalizedName, other.NormalizedName);
        }
    }


    public class SourceFile {
        public string FilePath { get; }
        public State.Document.TabItemProject Project { get; }
        public List<IncludeFile> Includes { get; } = new();

        public SourceFile(string filePath, State.Document.TabItemProject project) {
            this.FilePath = filePath;
            this.Project = project;
        }

        public override int GetHashCode() {
            int h1 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.FilePath);
            int h2 = StringComparer.OrdinalIgnoreCase.GetHashCode(this.Project.ShellProject.Project.UniqueName);
            return (h1 * 397) ^ h2; // HashCode.Combine(h1, h2);
        }

        public override bool Equals(object? obj) {
            return obj is SourceFile other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.FilePath, other.FilePath) &&
                   StringComparer.OrdinalIgnoreCase.Equals(
                       this.Project.ShellProject.Project.UniqueName,
                       other.Project.ShellProject.Project.UniqueName
                       );
        }

        public override string ToString() => $"{this.FilePath} [{this.Project.ShellProject.Project.UniqueName}]";
    }
}