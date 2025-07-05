using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio;


namespace TabsManagerExtension.VsShell.Document {
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
}