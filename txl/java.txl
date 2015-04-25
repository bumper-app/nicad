% Example using TXL 10.5a source coordinate extensions to extract
% a table of all method definitions with source coordinates

% Jim Cordy, January 2008

% Requires TXL 10.5a or later

% Using Java 5 grammar
include "java.grm"

% Main function - extract and mark up function definitions from parsed input program
function main
    match [program]
	P [program]
end function
