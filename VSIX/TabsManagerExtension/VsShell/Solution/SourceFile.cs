using System;
using System.IO;
using System.Collections.Generic;


namespace TabsManagerExtension.VsShell.Solution {
    public class IncludeEntry {
        public string RawInclude { get; }
        public string NormalizedName { get; }

        public IncludeEntry(string rawInclude) {
            this.RawInclude = rawInclude;
            this.NormalizedName = Path.GetFileName(rawInclude);
        }

        public override bool Equals(object? obj) {
            return obj is IncludeEntry other &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.RawInclude, other.RawInclude);
        }

        public override int GetHashCode() {
            return StringComparer.OrdinalIgnoreCase.GetHashCode(this.RawInclude);
        }

        public override string ToString() => this.RawInclude;
    }


    public class ResolvedIncludeEntry {
        public IncludeEntry IncludeEntry { get; }
        public string? ResolvedPath { get; }

        public ResolvedIncludeEntry(IncludeEntry includeEntry, string? resolvedPath) {
            this.IncludeEntry = includeEntry;
            this.ResolvedPath = resolvedPath;
        }

        public override bool Equals(object? obj) {
            return obj is ResolvedIncludeEntry other &&
                   this.IncludeEntry.Equals(other.IncludeEntry) &&
                   StringComparer.OrdinalIgnoreCase.Equals(this.ResolvedPath, other.ResolvedPath);
        }

        public override int GetHashCode() {
            int h1 = this.IncludeEntry.GetHashCode();
            int h2 = this.ResolvedPath is null ? 0 : StringComparer.OrdinalIgnoreCase.GetHashCode(this.ResolvedPath);
            return (h1 * 397) ^ h2;
        }

        public override string ToString() => $"{this.IncludeEntry.RawInclude} → {this.ResolvedPath ?? "unresolved"}";
    }



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