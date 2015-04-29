% Get original source for NiCad detected clones
% J.R. Cordy, April 2010

% Coppyright 2010, J.R. Cordy

% Version 1.0 (11 April 2010)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments
include "%system"

% Clean out Python INDENT / DEDENT tokens
function clean (line : string) : string
    var eline := line
    loop
	var idindex := index (eline, " INDENT")
	if idindex = 0 then
	    idindex := index (eline, "DEDENT ")
	end if

	exit when idindex = 0

	eline := eline (1 .. idindex - 1) + eline (idindex + length (" INDENT") .. *)
    end loop
    result eline
end clean

% Main program
if fetcharg (1) = "" or fetcharg (2) = "" then
    put : 0, "Usage:  getsource.x system_functions-clones.xml system_functions-clones-sourced.xml"
    quit : 1
end if

% Get program argument
const ccfile := fetcharg (1)	% name of the clones XML file, e.g., system_functions-clones.xml

% Try to open our system_functions-clones.xml file
var ccf: int
open : ccf, ccfile, get

if ccf = 0 then
    put : 0, "*** Error: can't open clone classes file"
    quit : 1
end if

% Create the resolved output file
var rccfile := fetcharg (2)	% name of the sourced clones XML file, e.g., system_functions-clones-sourced.xml
var rccf: int
open : rccf, rccfile, put 

% Read the potential clones from it
loop
    exit when eof (ccf)

    var line : string
    get : ccf, line:*

    if index (line, "<source") = 1 then
	var sourceheader := line (1 .. index (line, "</source>") - 1)
	put : rccf, sourceheader

	var sfindex := index (sourceheader, "file=") + length ("file=") + 1
	var sfend := index (sourceheader, " startline=") - 2
	var srcfile := sourceheader (sfindex .. sfend)

	var sf: int
	open : sf, srcfile, get

	if sf = 0 then
	    put : 0, "*** Error: can't open source file ", srcfile
	    quit : 1
	end if

	var slindex := index (sourceheader, "startline=") + length ("startline=") + 1
	var slend := index (sourceheader, " endline=") - 2
	var startline := strint (sourceheader (slindex .. slend))
	
	for i : 1 .. startline - 1
	    % Turing line input gets only 255 chars - lines can be longer
	    loop
	        get : sf, line:*
	        exit when eof (sf) or length (line) < 255
	    end loop
	end for

	var elindex := index (sourceheader, "endline=") + length ("endline=") + 1
	var elend := index (sourceheader, " pcid=") - 2
	var endline := strint (sourceheader (elindex .. elend))

	for i : startline .. endline
	    % Turing line input gets only 255 chars - lines can be longer
	    loop
	        get : sf, line:*
	        exit when eof (sf) or length (line) < 255
		put : rccf, clean (line) ..
	    end loop
	    put : rccf, clean (line)
	end for

	close : sf

	put : rccf, "</source>"

    else
	put : rccf, line
    end if
end loop

% put : 0, "Done"
close : rccf
