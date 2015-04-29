% NiCad 3.0 unpack clone classes
% J.R. Cordy, September 2012

% Copyright 2012 J.R. Cordy

% Version 1.1 (19 March 2013)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments
include "%system"

% Program parameters - from the command line
% splitclasses.x cloneclassesfile.xml
var ccfile : string			% name of the system_functions-classes-withsource.xml file

% Usage message
procedure useerr
    put : 0, "Usage :  splitclasses.x cloneclassfile.xml"
    put : 0, ""
    put : 0, "  e.g.:  splitclasses.x linux_functions-clones-0.3-classes-withsource.xml"
    put : 0, "  (Output in folders linux_functions-clones-0.3-classes-withsource/*)"
    put : 0, ""
    quit : 1
end useerr


% Main program
ccfile := fetcharg (1)
var ccf: int
open : ccf, ccfile, get

if ccf = 0 or ccfile (length (ccfile) - 3 .. *) not= ".xml" then
    useerr
end if

% Make class folder
external "system" function csystem (command : string) : int
var ccdir := ccfile (1 .. *-4)	% strip .xml
put : 0, "Making output folder ", ccdir
var rc := 0
var commandline := "/bin/rm -rf " + ccdir + " > /dev/null 2>&1"
rc := csystem (commandline)
commandline := "/bin/mkdir " + ccdir
rc := csystem (commandline)

if rc not= 0 then
    put : 0, "*** Error, cannot create output directory"
    quit : 99
end if

% Read in clone classes and split them into subfolders

loop
    exit when eof (ccf)

    var line : string
    get : ccf, line:*

    if index (line, "<class ") = 1 then
        % <class classid="1" nclones="2" nlines="5280" similarity="82">
        var classid := line (index (line, "classid=\"") + length ("classid=\"") .. index (line, " nclones=\"") - 2)
	var classdir := ccdir + "/" + classid
	put : 0, "Making clone class folder ", classdir
 	commandline := "/bin/mkdir " + classdir
	rc := csystem (commandline)

	if rc not= 0 then
    	    put : 0, "*** Error, cannot create class directory"
    	    quit : 99
	end if

	loop
	    if eof (ccf) then
	        put : 0, "*** Error, synchronization error on class file"
    		quit : 99
	    end if

	    get : ccf, line:*
	    exit when index (line, "</class>") = 1

	    if index (line, "<source ") = 1 then
		% <source file="models/Simulink/models/automotive/powerwindow03.mdl" startline="1134" endline="8688" pcid="51">
		var pcid := line (index (line, "pcid=\"") + length ("pcid=\"") .. index (line, "\">") - 1)
		var pcfile := classdir + "/" + pcid
		var pcf : int
		open : pcf, pcfile, put

		if pcf = 0 then
		    put : 0, "*** Error, cannot create pc file in class directory"
		    quit : 99
		end if

		put : pcf, line
		loop
		    get : ccf, line:*
		    exit when index (line, "</source>") = 1
		    put : pcf, line
		end loop

		close : pcf
	    end if
	end loop
    end if
end loop

put : 0, "Done"
put : 0, ""
