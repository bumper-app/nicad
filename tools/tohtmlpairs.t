% Convert NiCad original source clonepairs XML report to HTML
% J.R. Cordy, April 2010

% Version 1.0 (11 April 2010)

% This program requires the Turing Plus 2009 compiler,
% http://research.cs.queensu.ca/~stl/download/pub/tplus/

% Unix system interface, so we can get command line arguments 
include "%system"


% HTML special character encoding
function encode (line : string) : string
    var eline := ""
    for i : 1 .. length (line)
        case ord (line (i)) of
	    label ord ("<") :
		eline += "&lt;"
	    label ord (">") :
		eline += "&gt;"
	    label ord ("&") :
		eline += "&amp;"
	    label :
		eline += line (i)
	end case
    end for
    result eline
end encode


% Main program
if fetcharg (1) = "" or fetcharg (2) = "" then
    put : 0, "Usage:  tothml.x system_functions-clonepairs-withsource.xml system_functions-clonepairs-withsource.html"
    quit : 1
end if

% Get program argument
const ccfile := fetcharg (1)	% name of the sourced clonepairs XML file, e.g., system_functions-clonepairs-withsource.xml

% Try to open our system_functions-clonepairs-withsource.xml file
var ccf: int
open : ccf, ccfile, get 

if ccf = 0 then
    put : 0, "*** Error: can't open sourced clone classes input file"
    quit : 1
end if

% Create the resolved output file
var rccfile := fetcharg (2)	% name of the sourced clonepairs HTML file, e.g., system_functions-clonepairs-withsource.html
var rccf: int
open : rccf, rccfile, put 

% HTML style header
put : rccf, "<html>"
put : rccf, "<head>"
put : rccf, "<style type=\"text/css\">"
put : rccf, "body {font-family:Arial}"
put : rccf, "table {background-color:white; border:0px solid white; width:95%; margin-left:auto; margin-right: auto}"
put: rccf, "td {background-color:#b0c4de; padding:16px; border:4px solid white}"
put : rccf, "pre {background-color:white; padding:4px}"
put : rccf, "</style>"
put : rccf, "<title>"
put : rccf, "NiCad Clone Report"
put : rccf, "</title>"
put : rccf, "</head>"
put : rccf, "<body>"
put : rccf, "<h2>"
put : rccf, "NiCad Clone Report"
put : rccf, "</h2>"

% Check file type
var line: string
get : ccf, line:* 

if line not= "<clones>" then
    put : 0, "*** Error: malformed sourced clone classes input file"
    quit : 1
end if

% System information
% <systeminfo  system="httpd-2.2.8" granularity="functions" threshold="30%" minlines="3", maxlines="2000"/>
get : ccf, line:* 
const systemname := line (index (line, "system=\"") + length ("system=\"") .. index (line, "\" granularity=") - 1)
const granularity := line (index (line, "granularity=\"") + length ("granularity=\"") .. index (line, "\" threshold=") - 1)
const threshold := line (index (line, "threshold=\"") + length ("threshold=\"") .. index (line, "\" minlines=") - 1)
const minlines := line (index (line, "minlines=\"") + length ("minlines=\"") .. index (line, "\" maxlines=") - 1)
const maxlines := line (index (line, "maxlines=\"") + length ("maxlines=\"") .. index (line, "\"/>") - 1)

put : rccf, "System: ", systemname
put : rccf, "<br>"
put : rccf, "<br>Granularity: ", granularity
put : rccf, "<br>Max difference threshold: ", threshold
put : rccf, "<br>Clone size: ", minlines, " - ", maxlines, " lines"
put : rccf, "<br>"

% <cloneinfo npcs="2886" npairs="294"/>
get : ccf, line:* 
const npcs := line (index (line, "npcs=\"") + length ("npcs=\"") .. index (line, "\" npairs=") - 1)
const npairs := line (index (line, "npairs=\"") + length ("npairs=\"") .. index (line, "\"/>") - 1)

put : rccf, "<br>Total ", granularity, ": ", npcs 
put : rccf, "<br>Clone pairs found: ", npairs
put : rccf, "<br>"

% <runinfo ncompares="2915575" cputime="480000"/>
get : ccf, line:* 
const ncompares := line (index (line, "ncompares=\"") + length ("ncompares=\"") .. index (line, "\" cputime=") - 1)
const cputime := line (index (line, "cputime=\"") + length ("cputime=\"") .. index (line, "\"/>") - 1)
const cputotalms := strint (cputime) div 1000
const cpums := cputotalms mod 1000
const cpusec := (cpums div 1000) mod 60
const cpumin := (cpums div 1000) div 60 

put : rccf, "<br>LCS compares: ", ncompares, " &nbsp;&nbsp;&nbsp; CPU time: ", cpumin, " min ", cpusec, ".", cpums, " sec"
put : rccf, "<br>"

% Read the clone pairs from it
loop
    exit when eof (ccf)

    get : ccf, line:* 

    % Optional classes header
    if index (line, "<classinfo ") = 1 then

	% <classinfo nclasses="69"/>
	const nclasses := line (index (line, "nclasses=\"") + length ("nclasses=\"") .. index (line, "\"/>") - 1)
	put : rccf, "<br>Number of  classes: ", nclasses
	put : rccf, "<br>"

    % For each clone pair or class
    elsif index (line, "<clone ") = 1 or index (line, "<class ") = 1 then

	if index (line, "<clone ") = 1 then
	    % <clone  nlines="12" similarity="71">
	    const pairlines := line (index (line, "nlines=\"") + length ("nlines=\"") .. index (line, "\" similarity=") - 1)
	    const similarity := line (index (line, "similarity=\"") + length ("similarity=\"") .. index (line, "\">") - 1)

	    put : rccf, "<h3>"
	    put : rccf, "Clone pair, nominal size ", pairlines, " lines, similarity ", similarity, "%"
	    put : rccf, "</h3>"
	    put : rccf, "<table cellpadding=12 border=2 frame=\"box\" width=\"90%\">"

        else % index (line, "<class ") = 1 
	    % <class classid="2"  nclones="1" nlines="12" similarity="70">
	    const classid := line (index (line, "classid=\"") + length ("classid=\"") .. index (line, "\" nclones=") - 1)
	    const nclassclones := line (index (line, "nclones=\"") + length ("nclones=\"") .. index (line, "\" nlines=") - 1)
	    const classlines := line (index (line, "nlines=\"") + length ("nlines=\"") .. index (line, "\" similarity=") - 1)
	    const similarity := line (index (line, "similarity=\"") + length ("similarity=\"") .. index (line, "\">") - 1)
	    put : rccf, "<h3>Clone class ", classid, ", ", nclassclones, " fragments, nominal size ", classlines, " lines, similarity ", similarity, "%"
	    put : rccf, "</h3>"
	    put : rccf, "<table cellpadding=12 border=2 frame=\"box\" width=\"90%\">"
	end if

	% For each clone in the class
	loop
	    exit when eof (ccf)
	    get : ccf, line:* 
	    exit when line = "</clone>" or line = "</class>"

	    if index (line, "<source") = 1 then

		% <source file="systems/c/httpd-2.2.8/server/mpm/worker/worker.c.ifdefed" startline="1650" endline="1875" pcid="2553">
		const srcfile := line (index (line, "file=\"") + length ("file=\"") .. index (line, "\" startline=") - 1)
		const startline := line (index (line, "startline=\"") + length ("startline=\"") .. index (line, "\" endline=") - 1)
		const endline := line (index (line, "endline=\"") + length ("endline=\"") .. index (line, "\" pcid=") - 1)
		const pcid := line (index (line, "pcid=\"") + length ("pcid=\"") .. index (line, "\">") - 1)

		put : rccf, "<tr><td>"
		put : rccf, "Lines ", startline, " - ", endline, " of ", srcfile
		put : rccf, "<pre>"

		loop
		    exit when eof (ccf)

		    % Turing line input gets only 255 chars - lines can be longer
		    loop
		        get : ccf, line:* 
			exit when eof (ccf) or length (line) < 255
			% Encoding may make a line longer
		        put : ccf, encode (line (1..100)) ..
		        put : ccf, encode (line (100..*)) ..
		    end loop

		    exit when line = "</source>"

		    % Encoding may make a line longer
		    if length (line) > 100 then
			put : rccf, encode (line (1..100)) ..
			put : rccf, encode (line (100..*))
		    else
			put : rccf, encode (line)
		    end if
		end loop

		put : rccf, "</pre>"
		put : rccf, "</td></tr>"
	    end if

	    if line not= "</source>" then
		put : 0, "*** Error: clone source file synchronization error"
		quit : 1 
	    end if
	end loop

	if line not= "</clone>" and line not= "</class>" then
	    put : 0, "*** Error: clone pair / class file synchronization error"
	    quit : 1 
	end if

	put : rccf, "</table>"
    end if
end loop

% HTML trailer
put : rccf, "</body>"
put : rccf, "</html>"

close : ccf
close : rccf
