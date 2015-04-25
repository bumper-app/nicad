
NiCad3 clone detection system, Version 3.5 (19 March 2013)
----------------------------------------------------------
Software Technology Laboratory, Queen's University
April 2010 (Revised March 2013)

Copyright 2011-2013, J.R. Cordy & C.K. Roy

This directory contains all of the parsers and tools necessary to
install and run the NiCad near-miss clone detection system.
NiCad3 should compile and run correctly on all Linux, Mac OS X and 
Cygwin systems, but does not at present run directly under Windows. 

Installing NiCad3
-----------------
NiCad3 can be installed on Ubuntu, Mandriva, Mac OS X and other Unix-like
systems with a GCC compiler and a FreeTXL distribution.

0. NiCad3 requires that FreeTXL 10.5i or later be installed on your system.  
   FreeTXL can be downloaded from: 

      http://www.txl.ca/

   Install TXL 10.5i or later before proceeding.

1. NiCad3 optimizes by using precompiled TXL programs.  Use the command:

      make

   in this directory to precompile all NiCad3 tools and TXL programs before 
   installing NiCad3.  

   If you are not planning to install NiCad3 for everyone on your system, 
   and just want to use it yourself in this directory, you can stop here
   and move on to "Testing NiCad3" below.

2. To install NiCad3 for use by everyone on your system, copy the entire
   contents of this directory to any place where it can reside permanently.  
   On most systems the appropriate place would be /usr/local/lib/nicad.

      sudo mkdir /usr/local/lib/nicad
      sudo cp -r . /usr/local/lib/nicad

3. Edit the command scripts "nicad" and "nicadincr" in this directory
   to specify the directory where you installed NiCad3, and the kind of
   your system.  For example, on an Ubuntu system where you put NiCad3 in
   /usr/local/lib/nicad, you would modify the scripts to say:

      LIB=/usr/local/lib/nicad
      SYS=Ubuntu

   At present possible values for "SYS" are Ubuntu, MacOSX, and Mandriva.
   If you are in doubt or your Linux system is not listed, use "Ubuntu".

4. Copy the edited "nicad" and "nicadincr" scripts to any "bin" 
   directory which is on the command search path of the intended users
   of NiCad3.  On most systems the appropriate place  would be /usr/local/bin.

      cp nicad nicadincr /usr/local/bin

Testing NiCad3
-------------
Once NiCad3 is installed, for a quick demonstration, run this command
in this directory:

      nicad functions java examples/JHotDraw54b1 

Results can then be found in examples/JHotDraw54b1_functions-clones.
Open any of the html files with a standard browser to see the results.

Using NiCad3
-----------
To run NiCad3 on a new system,

1. Make a new working directory to hold the results (e.g. ~/nicadresults) 
   and change to that directory.  Copy the entire source directory of the 
   system you want to analyze to the working directory you are in 
   (e.g., cp -r /sources/JHotDraw54b1 .)  You can use a symbolic link 
   (e.g., ln -s /sources/JHotDraw54b1), but be warned that NiCad3 will create 
   temporary files in the source directories, so it must be writable. 

   At present NiCad3 can handle systems with source files written in the 
   languages: C (.c), C# (.cs), Java (.java) and Python (.py).

2. Run the NiCad3 command on it, specifying the granularity and language
   of the system, e.g.,

      nicad functions java JHotDraw54b1

   At present NiCad3 can handle the granularities: functions and blocks.

3. Examine the results in the system's clone results directory,
   e.g., JHotDraw54b1_functions-clones for the command above.
   NiCad3 results are reported in two ways: as clone classes in XML format, 
   both with and without original sources, and as browsable HTML pages
   with clone classes and original source for each clone.

   For the command above, the following results files will be created
   in that directory:

      Clone classes in XML-like format:

          JHotDraw54b1_functions-clones-0.0.xml              
          JHotDraw54b1_functions-clones-0.1.xml              
          JHotDraw54b1_functions-clones-0.2.xml
          JHotDraw54b1_functions-clones-0.3.xml

      Clone classes with original sources for clones in XML-like format:

          JHotDraw54b1_functions-clones-0.0-withsource.xml   
          JHotDraw54b1_functions-clones-0.1-withsource.xml   
          JHotDraw54b1_functions-clones-0.2-withsource.xml
          JHotDraw54b1_functions-clones-0.3-withsource.xml

      Clone classes with original sources for clones as HTML pages.
      These can be opened with any standard web browser:

          JHotDraw54b1_functions-clones-0.0-withsource.html  
          JHotDraw54b1_functions-clones-0.1-withsource.html  
          JHotDraw54b1_functions-clones-0.2-withsource.html
          JHotDraw54b1_functions-clones-0.3-withsource.html

   The 0.0, 0.1, 0.2 and 0.3 indicate the four default near-miss
   difference thresholds run by NiCad3, where 0.0 means exact clones,
   0.1 means at most 10% different pretty-printed lines, and so on.
   
4. NiCad3 supports renaming, filtering, abstraction and custom normalization 
   before comparison using configuration files stored in the $LIB/config 
   subdirectory of the NiCad3 installation directory.  To use a configuration, 
   run NiCad3 giving the name of the configuration as an extra parameter
   on the command line.  E.g., to use the blind renaming configuration,

      nicad functions java JHotDraw54b1 blindrename

   When using configurations, the requested transformations will be applied
   and the results reported in different directory, e.g., 

    JHotDraw54b1_functions-blind-clones

NiCad3Cross
-----------
NiCad3Cross is the NiCad3 cross-clone detector.  It does an cross-system test 
- that is, given two systems s1 and s2, it reports only clones of fragments 
of s1 in s2.  This is useful in incremental clone detection for new versions, 
or for detecting clones between two systems to be checked for cross-cloning.

NiCad3Cross is run in much the same way as NiCad3, but giving a second system
source directory on the command line, for example:

      nicad3cross functions java JHotDraw54b1 JHotDraw76 

Results are stored in a system's cross-clone results directory, 
e.g., JHotDraw54b1_functions-crossclones for the command above.

Maintenance and Extension of NiCad3
-----------------------------------
Maintaining or adding NiCad3 TXL plugins is easy - you just create the new
programs with appropriate names (see the $LIB/txl subdirectory of the NiCad 
installation directory for examples), and NiCad3 will automatically allow
your new plugins to be used as normalizations or languages.

If you plan to change, maintain or recompile the NiCad3 clone comparison
tools themselves, you will require Turing Plus 4.0 (2009) or later to be 
installed on your system.  See the $LIB/tools subdirectory of the NiCad3 
installation for details.  

Turing Plus 4.0 (2009) can be downloaded from: 
 
      http://research.cs.queensu.ca/~stl/download/pub/tplus/

JRC 19.3.13
