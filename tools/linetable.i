% Fast hash table to store NiCad potential clone line text
% J.R. Cordy, January 2010

% Adapted from TXL Version 10.3 (c)1988-2003 Queen's University at Kingston
% J.R. Cordy, C.D. Halpern, E.M. Promislow & I.H. Carmichael
% March 2003

% Version 1.1 (11 April 2010)

module linetable
    import (maxlinechars, maxlines)
    export (LN, install, gettext, printstats)

    % Text of all unique lines
    var lineText : array 1 .. maxlinechars + 1 of char
    var lineTextSize := 1

    % Hash table of all unique lines
    var lineTable : array 0 .. maxlines - 1 of int
    var lineTableSize := 0
    const nilLine := 0

    % The representation of lines is their index in the lineText array
    type LN : nat

    % Initialize the table
    
    for i : 0 .. maxlines - 1
    	lineTable (i) := nilLine
    end for
    
    type (string,lineText (1)) := " "	% need one character here as placeholder
    lineTextSize += 2

    lineTable (0) := addr (lineText (2))	% line 0 is the null line, with no characters in it
    lineTableSize := 1


    % New superfast hash function
    
    function hash (s : string) : nat
	type nat256 : array 0 .. 255 of nat1
	var register h : nat := length (s)
	var register j := h - 1
	if h > 0 then
	    for i : 0 .. h shr 1
	        h += h shl 1 + type (nat256, s) (i)
	        h += h shl 1 + type (nat256, s) (j)
	        j -= 1
	    end for
	end if
	result h
    end hash
    
    % Choose an appropriate secondary hash - critical to performance!
    
    var secondaryHash := 11
    
    const nprimes := 10
    const primes : array 1 .. nprimes of int := 
        init (1021, 2027, 4091, 8191, 16381, 32003, 65003, 131009, 262007, 524047)

    for i : 1 .. nprimes
        exit when i = 10 or primes (i) > maxlines
	secondaryHash := primes (i)
    end for


    % Ignore leading spaces
    var spaceP : array 0 .. 255 of boolean
    for c : 0 .. 255
        spaceP (c) := false
    end for
    spaceP (ord (' ')) := true
    spaceP (ord ('\t')) := true
    spaceP (ord ('\f')) := true
    

    function install (rawline : string) : LN

        % Trim spaces from the line
	var line := rawline
 	var rawlinelength := length (rawline)
	var first := 1
	loop
	    exit when first > rawlinelength or not spaceP (ord (line (first)))
	    first += 1
	end loop
	var last := rawlinelength
	loop
	    exit when last < 1 or not spaceP (ord (line (last)))
	    last -= 1
	end loop
	line := line (first .. last)

	% Now hash it
	var lineIndex : nat := hash (line)
	if lineIndex >= maxlines then
	    lineIndex := lineIndex mod maxlines
	end if
	const startIndex : nat := lineIndex
	const linelength := length (line)

	loop
	    if lineTable (lineIndex) = nilLine then
		if lineTextSize + linelength + 1 > maxlinechars then
		    put : 0, "*** Error: too much total unique line text ( > ", maxlinechars, " chars)"
		    quit : 1
		end if
	        type (string, lineText (lineTextSize)) := line
		lineTable (lineIndex) := lineTextSize
		lineTextSize += linelength + 1
		lineTableSize += 1
		exit
	    end if

	    exit when type (string, lineText (lineTable (lineIndex))) = line
	    
	    lineIndex += secondaryHash
	    if lineIndex >= maxlines then
	        lineIndex := lineIndex mod maxlines
	    end if
	    
	    if lineIndex = startIndex then
	        put : 0, "*** Error: too many unique lines ( > ", maxlines, ")"
	        quit : 1
	    end if
	end loop

	result lineIndex
	
    end install


    function gettext (lineIndex : LN) : string
	result type (string, lineText (lineTable (lineIndex)))
    end gettext


    procedure printstats
	put : 0, "Used ", lineTextSize, "/", maxlinechars, " unique line text chars"
	put : 0, " and ", lineTableSize, "/", maxlines, " unique lines"
    end printstats
    
end linetable
