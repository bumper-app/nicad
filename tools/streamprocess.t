% Stream processor for single file extracted potential clones
% J.R. Cordy, May 2010

% Copyright 2010, J.R. Cordy

% Version 1.0 (20 May 2010)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments
include "%system"

% Main program
if fetcharg (1) = "" or fetcharg (2) not= "" then
    put : 0, "Usage:  streamprocess.x normalizing_command < system_functions.xml > system_functions-normalized.xml"
    quit : 1
end if

% Get program arguments
const command := fetcharg (1)	% command to run on potential clones

% Make a new temp name
var unique : int := getpid 
var tempinfile := "/tmp/nicad" + intstr (unique) + ".in"
var tempoutfile := "/tmp/nicad" + intstr (unique) + ".out"

% Read and process the potential clones 
var line : string
get line:*

loop
    exit when eof 

    if index (line, "<source") not= 1 then
        put : 0, "*** Error: synchronization error on potential clones file"
        quit : 1
    end if
    
    % Get the next N potenial clones and put them in a file for processing
    const batchsize := 100

    var tf: int
    open : tf, tempinfile, put

    if tf = 0 then
        put : 0, "*** Error: can't create temp file "
        quit : 1
    end if

    for i : 1 .. batchsize 
        % Next PC
	put : tf, line
	loop
	    % Turing line input gets only 255 chars - lines can be longer
	    loop
		get line:*
		exit when eof or length (line) < 255
		put : tf, line ..
	    end loop
	    exit when eof or index (line, "</source>") = 1
            put : tf, line
	end loop
        put : tf, line

        % Might be at end of PCs
        exit when eof

	if index (line, "</source>") not= 1 then
	    put : 0, "*** Error: synchronization error on potential clones file"
	    quit : 1
	end if

        get line:*
    end for

    close : tf

    % Process them using the command - automatically makes the output part of our output
    external "system" function csystem (command : string) : int
    var rc := 0
    const commandline := command + " < " + tempinfile % + " > " + tempoutfile
    rc := csystem (commandline)

    if rc not= 0 then
	put : 0, "*** Error: command failed: " + commandline
	quit : 1
    end if
end loop

% put : 0, "Done"
