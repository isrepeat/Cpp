using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VideoPlayerPrototype
{
    internal class Assets
    {
        public static readonly IList<string> mediaTypes = new List<string> {
            ".mp4",
            ".avi",
            ".wmv",
            ".mov",
            ".mkv",
            ".3gp",
            ".flv",
            ".webm",
            ".ts",
        };

        public static readonly string volumeFormat = "{0,4}%";
    }
}
