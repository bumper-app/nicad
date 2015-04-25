# nicad

NiCad is a flexible TXL-based hybrid language-sensitive / text comparison software clone detection system developed by James R. Cordy and Chanchal K. Roy based on Chanchal's PhD thesis work. NiCad3 is a significantly new implementation with many fundamental improvements.

The NiCad Clone Detector is a scalable, flexible clone detection tool designed to implement the NiCad (Automated Detection of Near-Miss Intentional Clones) hybrid clone detection method in a convenient, easy-to-use command-line tool that can easily be embedded in IDEs and other environments. It takes as input a source directory or directories to be checked for clones and a configuration file specifying the normalization and filtering to be done, and provides output results in both XML form for easy analysis and HTML form for convenient browsing.

NiCad handles a range of languages, including C, Java, Python, and C#, and provides a range of normalizations, filters and abstractions. It is designed to be easily extensible using a component-based plugin architecture. It is scalable to very large systems and has been used to analyze, for example, all 47 releases of FreeBSD (60 million lines) as a single system.

