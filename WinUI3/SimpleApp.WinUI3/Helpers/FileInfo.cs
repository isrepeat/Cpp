using Microsoft.UI.Xaml.Media.Imaging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace SimpleApp.WinUI3.Helpers {
    public class FileInfo {
        public string Path { get; set; }
        public string Name { get; set; }
        public string Extension { get; set; }
        public string Directory { get; set; }
        public string NameWithoutExtension { get; set; }
        public string PathWithoutExtension { get; set; }
        public List<string> NameSuffixes { get; set; } = null;
        public string NameWithoutSuffixes { get; set; }
        public Enums.MediaType MediaType { get; set; } = Enums.MediaType.Unknown;

        public FileInfo() {
        }

        public FileInfo(string filePath) {
            this.Path = filePath;
            this.Name = System.IO.Path.GetFileName(filePath);
            this.Extension = System.IO.Path.GetExtension(filePath);
            this.Directory = System.IO.Path.GetDirectoryName(filePath);
            this.NameWithoutExtension = System.IO.Path.GetFileNameWithoutExtension(filePath);
            this.PathWithoutExtension = System.IO.Path.Combine(this.Directory, this.NameWithoutExtension);

            this.NameSuffixes = new List<string>();
            try {
                var matches = Regex.Matches(this.NameWithoutExtension, @"({.+?})", RegexOptions.None, TimeSpan.FromSeconds(1));
                foreach (Match match in matches) {
                    if (match.Groups[1].Success) {
                        this.NameSuffixes.Add(match.Groups[1].Value);
                    }
                }
            }
            catch (RegexMatchTimeoutException ex) {
                Diagnostic.Logger.LogDebug($"RegexMatchTimeoutException = {ex.Message}");
            }

            var concatenatedSuffixes = String.Join(string.Empty, this.NameSuffixes);
            if (String.IsNullOrEmpty(concatenatedSuffixes)) {
                this.NameWithoutSuffixes = this.NameWithoutExtension;
            }
            else {
                this.NameWithoutSuffixes = this.NameWithoutExtension.Replace(concatenatedSuffixes, string.Empty);
            }


            this.MediaType = this.Extension switch {
                ".mp4" => Enums.MediaType.Video,
                ".jpg" => Enums.MediaType.Image,
                ".jpeg" => Enums.MediaType.Image,
                _ => Enums.MediaType.Unknown
            };
        }
    }

    public class FileInfoComparer : IEqualityComparer<Helpers.FileInfo> {
        public enum ComparedBy {
            Path,
            Name,
            NameWithoutExtension,
        }
        public enum Suffix {
            WithSuffix,
            WithoutSuffix,
        }
        public class Options {
            public StringComparison StringComparison { get; set; } = StringComparison.OrdinalIgnoreCase;
            public ComparedBy ComparedBy { get; set; } = ComparedBy.Path;
            public Suffix Suffix { get; set; } = Suffix.WithSuffix;
        }


        private Options options = null;
        public FileInfoComparer(Options options) {
            this.options = options;
        }
        public bool Equals(Helpers.FileInfo a, Helpers.FileInfo b) {
            if (a == null || b == null || this.options == null) {
                return false;
            }

            switch (this.options.ComparedBy) {
                case ComparedBy.Path:
                    return string.Equals(a.Path, b.Path, this.options.StringComparison);

                case ComparedBy.Name:
                    return string.Equals(a.Name, b.Name, this.options.StringComparison);

                case ComparedBy.NameWithoutExtension:
                    if (this.options.Suffix == Suffix.WithoutSuffix) {
                        return string.Equals(a.NameWithoutSuffixes, b.NameWithoutSuffixes, this.options.StringComparison);
                    }
                    else {
                        return string.Equals(a.NameWithoutExtension, b.NameWithoutExtension, this.options.StringComparison);
                    }

                default:
                    System.Diagnostics.Debugger.Break();
                    throw new ArgumentException();
            }
        }

        public int GetHashCode(Helpers.FileInfo obj) {
            return obj.Name?.GetHashCode(StringComparison.OrdinalIgnoreCase) ?? 0;
        }
    }
}