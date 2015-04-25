% New efficient implementation of NiCad clone detection tool
% J.R. Cordy, January 2010
% after the original Perl implementation by N. Synytskyy and C.K. Roy (thanks guys!)

% Copyright 2010-2012 J.R. Cordy, C.K. Roy, N. Synytskyy

% Version 5.3 (6 Sept 2012)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments
include "%system"

% Program parameters - from the command line
% clonepairs.x pcfile threshold minclonelines maxcloneslines showsource
var pcfile : string			% name of the system_functions.xml pcs file
var threshold := 0.0			% difference threshold, 0.0 .. 0.9 (optional, default 0.0 (exact))
var minclonelines := 5			% minimum clone size (optional, default 5 lines)
var maxclonelines := 2500		% maximum clone size (optional, default 2500 lines)
var showsource := false			% embed pc source in results (optional, default no)

% Usage message
procedure useerr
    put : 0, "Usage :  clonepairs.x pcfile.xml [ threshold ] [ minclonesize ] [ maxclonesize ] [ showsource ] > clone-pairs-file.xml"
    put : 0, ""
    put : 0, "  e.g.:  clonepairs.x linux_functions.xml 0.2 > linux_functions-clones.xml"
    put : 0, ""
    put : 0, "    threshold = difference threshold, 0.0 .. 0.9 (optional, default 0.0 (exact))"
    put : 0, "    minclonesize = minimum clone size (optional, default 5 lines)"
    put : 0, "    maxclonesize = maximum clone size (optional, default 2500 lines, max 20000 lines)"
    put : 0, "    showsource = embed pc source in results (optional, argument present = yes, default no)"
    quit : 1
end useerr


% Get program parameters
forward procedure getprogramarguments
getprogramarguments


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
    
% Clone pairs - fills as we find them 
type CP :
    record
	pc1, pc2 : int
	similarity : int1	% in percent
    end record

var clonepairs: array 1..maxclones of CP
var npairs := 0

% Statistics we collect
var ncompares := 0 
var nclones := 0 
var nfragments := 0 


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

  	% We filter out pcs that are smaller or bigger than our pc size limits
	% by setting their size to zero
	if pc.nlines <  minclonelines or pc.nlines > maxclonelines then
	    pc.nlines := 0
	    nlines := pc.firstline 	
	end if
    end for

    if not eof (pcf) then
	put : 0, "*** Error: too many potential clones ( > ", maxpcs, ")"
    end if

    close : pcf
end readpcs


% Use a standard quicksort to sort the pcs by decreasing size from largest to smallest

procedure swappcs (i, j : int)
    var t := pcs (i)
    pcs (i) := pcs (j)
    pcs (j) := t
end swappcs

var depth := 0

procedure sortpcs (first, last : int)
    depth := depth + 1

    % Indicate our progress 
    if depth mod 1000 = 1 then
        put : 0, "." ..
    end if

    
    if first < last then
	var low := first
	var high := last + 1
	var pivot := low
	loop
	    loop
		high -= 1
		exit when low = high 
		exit when pcs (high).nlines > pcs (pivot).nlines
	    end loop

	    exit when low = high

	    loop
		low += 1
		exit when low = high
		exit when pcs (pivot).nlines > pcs (low).nlines
	    end loop

	    exit when low = high

	    swappcs (low, high)
	end loop

	swappcs (high, pivot)
	sortpcs (first, high - 1)
	sortpcs (high + 1, last)
    end if

    depth := depth - 1
end sortpcs


% Standard dynamic programming version of the longest common subsequence (lcs) length algorithm 
% http://en.wikipedia.org/wiki/Longest_common_subsequence_problem#Computing_the_length_of_the_LCS
% Optimized to cut off early using a difference limit

var dpmatrix : array 0 .. maxclonelines, 0 .. maxclonelines of nat2
assert maxclonelines < 65536

function lcs (pc1, pc2 : PC, m, n: int, difflimit: int) : int
    for i : 0 .. m
        dpmatrix (i, 0) := 0
    end for
    for j : 0 .. n
        dpmatrix (0, j) := 0
    end for

    for i : 1 .. m
        for j : 1 .. n
            if lines (pc1.firstline + i) = lines (pc2.firstline + j) then
                dpmatrix (i, j) := dpmatrix (i - 1, j - 1) + 1
            elsif dpmatrix (i - 1, j) >= dpmatrix (i, j - 1) then
                dpmatrix (i, j) := dpmatrix(i - 1, j)
            else
                dpmatrix (i, j) := dpmatrix (i, j - 1)
            end if
        end for

	% Optimize by cutting off when it's hopeless
	if i - dpmatrix (i, n) > difflimit then
	    result 0
	end if
    end for

    result dpmatrix (m, n)
end lcs


% We have two different definitions of clone - exact and near miss
% Exact clones' pc lines must match exactly 

function exact (pc1, pc2 : PC) : boolean
    assert pc1.nlines = pc2.nlines 
    for i : 1..pc1.nlines
	if lines (pc1.firstline + i) not= lines (pc2.firstline + i) then
	    result false
	end if
    end for
    result true
end exact


% Near miss clones are relative to our difference threshold
% By the NiCad defintion, two pcs are near miss clones if
%    unique lines in pc1 / total lines in pc1 <= threshold, and
%    unique lines in pc2 / total lines in pc2 <= threshold
% where unique lines = total lines - common lines

function nearmiss (pc1, pc2 : PC) : int 	% returns percent same
    % Don't count self-clones
    if pc2.srcfile = pc1.srcfile and
	    pc2.srcstartline >= pc1.srcstartline and
	    pc2.srcendline <= pc1.srcendline then
	result 0
    end if

    % Compute the length of the longest common subsequence of lines
    const difflimit := round (pc1.nlines * threshold) 
    const pc1pc2lcs := lcs (pc1, pc2, pc1.nlines, pc2.nlines, difflimit)

    % This condition implements exactly the NiCad nearmiss clone definition
    % quoted above
    if ((pc1.nlines - pc1pc2lcs) / pc1.nlines) <= threshold 
	    and ((pc2.nlines - pc1pc2lcs) / pc2.nlines) <= threshold then 
        result round (pc1pc2lcs / pc1.nlines * 100)
    else
	result 0
    end if
end nearmiss


% Evaluate whether two pcs are clones, using the above

function clone (pc1, pc2 : PC, threshold : real) : int	% percent same
    ncompares += 1
    if threshold < 0.01 then
	if exact (pc1, pc2) then
	    nclones += 1
	    nfragments += 1
	    result 100
	end if
    else
	const sameness := nearmiss (pc1, pc2) 
	if 100 - sameness <= round (threshold * 100) then
	    nclones += 1
	    nfragments += 1
	    result sameness 
	end if
    end if
    result 0
end clone


% The NiCad algorithm - find  all clones by using each one as an exemplar,
% and comparing it to all others within the difference threshold of similar size

procedure findclones 

    for i : 1 .. npcs
	% Indicate our progress
	if i mod 1000 = 0 then
	    put : 0, "." ..
	end if

	% If the pc is within  our limits (already marked by nlines>0)
	if pcs (i).nlines not= 0 then

	    % We only need to look forward in the size-sorted array of pcs, 
	    % because pcs (i) has already been compared to all those before it
	    % of similar size, and our clone definition is commutative 

	    % No need to compare to those that are more than the threshold smaller
	    const difflimit := round (pcs (i).nlines * threshold)

	    for j : i+1 .. npcs
		exit when pcs (j).nlines <  pcs (i).nlines - difflimit

		% If the second pc is within  our limits (already marked by nlines>0)
    		if pcs (j).nlines not=0 then

		    % Check for a new clone pair
		    const similarity := clone (pcs (i), pcs (j), threshold) 
		    if similarity not= 0 then
			npairs += 1
			if npairs > maxclones then
			    put : 0, "*** Error: too many clone pairs"
			    quit : 1
			end if
			bind cp to clonepairs (npairs)
			cp.pc1 := i
			cp.pc2 := j
			cp.similarity := similarity
		    end if
		end if
	    end for
	end if
    end for
end findclones


% Use a standard quicksort to sort the clonepairs by decreasing similarity

procedure swapclonepairs (i, j : int)
    var t := clonepairs (i)
    clonepairs (i) := clonepairs (j)
    clonepairs (j) := t
end swapclonepairs

var cdepth := 0

procedure sortclonepairs (first, last : int)
    cdepth := cdepth + 1

    % Indicate our progress 
    if cdepth mod 1000 = 1 then
	put : 0, "." ..
    end if

    
    if first < last then
	var low := first
	var high := last + 1
	var pivot := low
	loop
	    loop
		high -= 1
		exit when low = high 
		exit when clonepairs (high).similarity > clonepairs (pivot).similarity
	    end loop

	    exit when low = high

	    loop
		low += 1
		exit when low = high
		exit when clonepairs (pivot).similarity > clonepairs (low).similarity
	    end loop

	    exit when low = high

	    swapclonepairs (low, high)
	end loop

	swapclonepairs (high, pivot)
	sortclonepairs (first, high - 1)
	sortclonepairs (high + 1, last)
    end if

    cdepth := cdepth - 1
end sortclonepairs


% Prune embedded clone pairs 

procedure pruneembeddedpairs 
    % Remove any embedded smaller clone pairs 
    % First mark all embedded pairs
    var embeddedpairs := 0

    for i : 1 .. npairs
	% Indicate our progress 
	if i mod 1000 = 1 then
	    put : 0, "." ..
	end if

	if clonepairs (i).pc1 not= 0 then
	    bind pc1 to pcs (clonepairs (i).pc1)
	    bind pc2 to pcs (clonepairs (i).pc2)
	    % Test all remaining pairs
	    for j : i + 1 .. npairs
		if clonepairs (j).pc1 not= 0 then
		    bind epc1 to pcs (clonepairs (j).pc1)
		    bind epc2 to pcs (clonepairs (j).pc2)
		    % Check for embedding
		    if (epc1.srcfile = pc1.srcfile and epc2.srcfile = pc2.srcfile
			    and (epc1.srcstartline >= pc1.srcstartline and
				epc1.srcendline <= pc1.srcendline) 
			    and (epc2.srcstartline >= pc2.srcstartline and
				epc2.srcendline <= pc2.srcendline)) 
			% Pairs maybe reversed
			or (epc1.srcfile = pc2.srcfile and epc2.srcfile = pc1.srcfile
			    and (epc1.srcstartline >= pc2.srcstartline and
				epc1.srcendline <= pc2.srcendline) 
			    and (epc2.srcstartline >= pc1.srcstartline and
				epc2.srcendline <= pc1.srcendline)) then
			embeddedpairs += 1
			% Mark for removal
			clonepairs (j).pc1 := 0
			clonepairs (j).pc2 := 0
		    end if
		end if
	    end for
	end if
    end for

    % Now remove the marked pairs
    npairs -= embeddedpairs
    var p := 1
    for i : 1 .. npairs 
	loop
	    exit when clonepairs (p).pc1 not= 0
	    p += 1
	end loop
	clonepairs (i) := clonepairs (p)
	p += 1
    end for
end pruneembeddedpairs


% Generate XML output of our result

procedure showpairs
    % System information
    var slashindex := length (pcfile)
    loop
	exit when slashindex = 0 or pcfile (slashindex) = "/"
	slashindex -= 1
    end loop

    var systeminfo := pcfile (slashindex + 1 .. *) 
    var usindex := length (systeminfo)
    loop
	exit when usindex = 2 or systeminfo (usindex) = "_"
	usindex -= 1
    end loop

    var systemname := systeminfo (1 .. usindex - 1)
    var granularity := systeminfo (usindex + 1 .. index (systeminfo, ".xml") - 1)

    put "<clones>"
    put "<systeminfo processor=\"nicad3\" system=\"", systemname, "\" granularity=\"", granularity, "\" threshold=\"", round (threshold * 100 + 0.1), 
	"%\" minlines=\"", minclonelines, "\" maxlines=\"", maxclonelines, "\"/>"
    put "<cloneinfo npcs=\"", npcs, "\" npairs=\"", npairs, "\"/>"
    external "clock" function cpuclock : int
    var cputime := cpuclock
    put "<runinfo ncompares=\"", ncompares, "\" cputime=\"", cputime, "\"/>"
    put ""

    for i : 1 .. npairs
	% Indicate progress
	if i mod 100 = 1 then
	    put : 0, "." ..
	end if

	bind cp to clonepairs (i)
	bind pc1 to pcs (cp.pc1)
	put "<clone nlines=\"", pc1.nlines, "\" similarity=\"", cp.similarity, "\">" 
	put linetable.gettext (pc1.info) (1 .. *-1), " pcid=\"", pc1.num, "\">" ..
	if showsource then
	    put ""
	    for k : 1 .. pc1.nlines
		put linetable.gettext (lines (pc1.firstline + k))
	    end for
	end if
	put "</source>"
	bind pc2 to pcs (cp.pc2)
	put linetable.gettext (pc2.info) (1 .. *-1), " pcid=\"", pc2.num, "\">" ..
	if showsource then
	    put ""
	    for k : 1 .. pc2.nlines
		put linetable.gettext (lines (pc2.firstline + k))
	    end for
	end if
	put "</source>"
	put "</clone>"
	put ""
    end for 
    put "</clones>"
end showpairs


% Proces program arguments

body procedure getprogramarguments
    % Check usage
    if nargs < 1 then
	useerr
    end if

    % Names of the extracted pc XML file
    pcfile := fetcharg(1)

    % Difference threshold
    if nargs > 1 then
	threshold := strreal (fetcharg (2))
	if threshold < 0.0 or threshold > 1.0 then
	    useerr
	end if
    end if

    % Minimum clone size
    if nargs > 2 then
	minclonelines := strint (fetcharg (3))
	if minclonelines < 1 or minclonelines > 100 then
	    useerr
	end if
    end if

    % Maximum clone size
    if nargs > 3 then
	maxclonelines := strint (fetcharg (4))
	if maxclonelines < minclonelines or maxclonelines > 20000 then
	    useerr
	end if
    end if

    % Embedded source in our result
    if nargs > 4 then
	showsource := true
    end if
end getprogramarguments


% Main program

% Steps of the process
put : 0, "Processing ", pcfile, " with difference threshold ", round (threshold * 100 + 0.1), 
    "%, clone size ", minclonelines, " .. ", maxclonelines, " lines" ..
if showsource then
    put : 0, ", with pc source"
else
    put : 0, ""
end if

put : 0, "Reading pcs " ..
readpcs
put : 0, " done."
put : 0, "Total ", npcs, "/", maxpcs, " potential clones of ", minclonelines, " lines or more"
put : 0, "Used ", nlines, "/", maxtotallines, " total pc lines"
linetable.printstats

put : 0, "Sorting pcs " ..
sortpcs (1,npcs)
put : 0, " done."

put : 0, "Finding clones " ..
findclones 
put : 0, " done."

put : 0, "Total ", ncompares, " pc comparisons"

put : 0, "Pruning embedded clone pairs " ..
const oldnpairs := npairs
pruneembeddedpairs
put : 0, " done."

put : 0, "Pruned ", oldnpairs - npairs, " embedded clone pairs"
put : 0, "Found ", npairs, " clone pairs"

%% put : 0, "Sorting results " ..
%% sortclonepairs (1,npairs)
%% put : 0, " done."

put : 0, "Generating XML output " ..
showpairs
put : 0, " done."
put : 0, ""

