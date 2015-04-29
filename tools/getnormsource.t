% NiCad Get Normalized Source Utility - inserts normalized source in clone files
% J.R. Cordy, September 2012

% Copyright 2012 J.R. Cordy

% Version 1.0 (14 Sept 2012)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments
include "%system"

% Program parameters - from the command line
% getnormsource.x pcfile clonesfile outputclonesfile
var pcfile : string			% name of the system_functions.xml pcs file
var ccfile : string			% name of the system_functions-clones.xml clones file
var rccfile : string			% name of the system_functions-clones-withnormsource.xml result file
var minclonelines := 5			% minimum clone size (optional, default 5 lines)
var maxclonelines := 2500		% maximum clone size (optional, default 2500 lines)

% Usage message
procedure useerr
    put : 0, "Usage :  getnormsource.x pcfile.xml inclonesfile.xml outclonesfile.xml"
    put : 0, ""
    put : 0, "  e.g.:  getnormsource.x linux_functions.xml linux_functions-clones.xml linux_functions-clones-withnormsource.xml"
    put : 0, ""
    quit : 1
end useerr

% Internal limits of this implementation
#if HUGE then
    const SIZE := 50000
#else
    const SIZE := 10000
#end if

const maxtotallines := 2500 * SIZE	% total lines in all potential clones together
const maxpcs := 150 * SIZE		% total number of potential clones
const maxclones := 250 * SIZE 		% maximum number of total clones in all classes

% Hash table for lines
const maxlinechars := 15000 * SIZE	% total characters in all unique lines
const maxlines := 300 * SIZE		% total unique lines

% In this case we don't want to trim spaces from the source
#define RAWLINES

% Line text hash table
include "linetable.i"

% Potential clones - we read all pcs into this array
type PC :
    record
	num : int			% pc id
	info : linetable.LN		% info line, e.g., <source ... >
	srcfile : linetable.LN		% original source file name
	srcstartline, srcendline : int	% original source line range
        firstline : int			% 0 origin index of first line in lines array
	nlines : int			% number of lines in the pc
	embedding : int			% embedding clone class, intially 0
    end record

var pcs : array 1 .. maxpcs of PC
var npcs := 0

var lines : array 1 .. maxtotallines of linetable.LN
var nlines := 0
    

% Read the potential clones from the system_functions.xml file
% All line text is stored in the lines array, referenced by pcs array

procedure readpcs
    % Try to open our system_functions.xml file
    var pcf: int
    open : pcf, pcfile, get

    if pcf = 0 then
	useerr
    end if

    % Read the potential clones from it
    for i : 1 .. maxpcs
	% Indicate our progress 
	if i mod 1000 = 1 then
 	    put : 0, "." ..
	end if

	exit when eof (pcf)

	% Process the next pc
	npcs += 1

	bind var pc to pcs (npcs)
	pc.num := i
	pc.embedding := 0

	% Get the info header <source ... >
	var sourceheader : string
	get : pcf, sourceheader : *
	pc.info := linetable.install (sourceheader)

	if index (sourceheader, "<source ") not= 1 then
	    put : 0, "*** Error: synchronization error on pc file"
	    quit : 1
	end if

	% Decode the info header
	const sfindex := index (sourceheader, "file=") + length ("file=") + 1
	const sfend := index (sourceheader, " startline=") - 2
	pc.srcfile := linetable.install (sourceheader (sfindex .. sfend))

	const slindex := index (sourceheader, "startline=") + length ("startline=") + 1
	const slend := index (sourceheader, " endline=") - 2
	pc.srcstartline := strint (sourceheader (slindex .. slend))

	const elindex := index (sourceheader, "endline=") + length ("endline=") + 1
	const elend := length (sourceheader) - 2
	pc.srcendline := strint (sourceheader (elindex .. elend))

	% Store the lines of the pc in the lines array
	% pc.firstline is zero origin, so it is actually one less than the index
	% of the first line of the pc in the lines array
	pc.firstline := nlines 
	loop
	    if eof (pcf) then
	        put : 0, "*** Error: synchronization error on pc file"
	        quit : 1
	    end if

	    var line : string
	    get : pcf, line : *

	    exit when line = "</source>"

	    if line not= "" then
	        nlines += 1
	        if nlines > maxtotallines then
		    put : 0, "*** Error: too many total lines ( > ", maxtotallines, ")"
		    quit : 1
	        end if
		lines (nlines) := linetable.install (line)
	    end if
	end loop

	pc.nlines := nlines - pc.firstline	% (sic) 

/*
  	% We filter out pcs that are smaller or bigger than our pc size limits
	% by setting their size to zero
	if pc.nlines <  minclonelines or pc.nlines > maxclonelines then
	    pc.nlines := 0
	    nlines := pc.firstline 	
	end if
*/
    end for

    if not eof (pcf) then
	put : 0, "*** Error: too many potential clones ( > ", maxpcs, ")"
    end if

    close : pcf
end readpcs


% Proces program arguments

procedure getprogramarguments
    % Check usage
    if nargs not= 3 then
	useerr
    end if

    % Name of the extracted pc XML file
    pcfile := fetcharg(1)

    % Name of the clones XML file, e.g., system_functions-clones.xml  
    ccfile := fetcharg (2)	

    % Name of the result normalized sourced clones XML file, e.g., system_functions-clones-sourced.xml
    rccfile := fetcharg (3)	
end getprogramarguments


% Read and resolve normalized pc sources in clones file

procedure resolvepcsources
    % Try to open our system_functions-clones.xml file
    var ccf: int
    open : ccf, ccfile, get

    if ccf = 0 then
	useerr
    end if

    % Create the resolved output file
    var rccf: int
    open : rccf, rccfile, put 

    if rccf = 0 then
	useerr
    end if

    % Read the input clones file, producing the sourced output clones file
    for i : 1 .. 999999999
	% Indicate progress
	if i mod 100 = 1 then
	    put : 0, "." ..
	end if

	exit when eof (ccf)

	% Get next input line
	var line : string
	get : ccf, line:*

	% If it is a pc source directive, output it with the pc's normalized source in it
	if index (line, "<source ") = 1 then
	    % <source file="examples/JHotDraw54b1/src/CH/ifa/draw/contrib/PolygonScaleHandle.java" startline="141" endline="145" pcid="691"></source>

	    var sourceheader := line (1 .. index (line, "></source>")) 
	    % <source file="examples/JHotDraw54b1/src/CH/ifa/draw/contrib/PolygonScaleHandle.java" startline="141" endline="145" pcid="691">
	    put : rccf, sourceheader

	    % Get the pcid of the normalized pc hose source we need
	    var textpcid := sourceheader (index (sourceheader, "pcid=") + length ("pcid=") + 1 .. length (sourceheader) - 1)
	    var pcid := strint (textpcid)

	    % Find that pc - free since they are read in order
	    bind pc to pcs (pcid)
	    if pc.num not= pcid then
		put : 0, "*** Error in pc number"
		quit : 99
	    end if

	    for k : 1 .. pc.nlines
		put : rccf, linetable.gettext (lines (pc.firstline + k))
	    end for

	    put : rccf, "</source>"

	else
	    put : rccf, line
	end if
    end for

    % Finished
    close : rccf
end resolvepcsources


% Main program

% Get program args
getprogramarguments

% Read normalized pcs
put : 0, "Reading pcs " ..
readpcs
put : 0, " done."

% Insert pc source in input clones file
put : 0, "Resolving normalized pc sources " ..
resolvepcsources
put : 0, " done."
put : 0, ""

