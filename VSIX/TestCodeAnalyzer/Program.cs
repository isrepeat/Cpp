using System;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;


namespace TestCodeAnalyzer {
    public class Programm {
        public static void Main() {
            Programm.TestMultistateBehaviour();

            Console.WriteLine("Press any key to continue . . .");
            Console.ReadKey();
        }

        private static void TestMultistateBehaviour() {
            var projectNode = new Tests.MultistateBehaviour.ProjectNode();
        }
    }
}