% Example using TXL 10.5a source coordinate extensions to extract
% a table of all method definitions with source coordinates

% Jim Cordy, January 2008

% Revised Aug 2012 - disallow ouput forms in input parse - JRC
% Revised July 2011 - ignore BOM headers in source
% Revised 30.04.08 - unmark embedded functions - JRC

% Using C# grammar
include "csharp.grm"

% Ignore BOM headers from Windows
include "bom.grm"

% Temporary handling of designated Linq extensions
redefine invocation_operator
	...
    |	'(( [repeat argument_list_or_key] '))
end redefine

define argument_list_or_key
	[argument_list]
    |	'in
end define

% Redefinitions to collect source coordinates for function definitions as parsed input,
% and to allow for XML markup of function definitions as output

redefine method_declaration
	[method_definition]
    |
	% Uninteresting interface form
	[method_header] ';			[NL]
end redefine

define method_definition
	% Input form 
	[srcfilename] [srclinenumber] 		% Keep track of starting file and line number
	[method_header]				
	'{                                      [NL][IN] 
	    [opt statement_list]     		[EX]
	    [srcfilename] [srclinenumber] 	% Keep track of ending file and line number
	'}  [opt ';]				
    |
	% Output form 
	[not token]				% disallow output form in input parse
	[opt xml_source_coordinate]
	[method_header]				
	'{                                      [NL][IN] 
	    [opt statement_list]     		[EX]
	'}  [opt ';]				
	[opt end_xml_source_coordinate]
end define

define xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [NL]
end define

define end_xml_source_coordinate
    [NL] '< [SPOFF] '/ 'source '> [SPON] [NL]
end define

redefine program
	...
    | 	[repeat method_definition]
end redefine


% Main function - extract and mark up function definitions from parsed input program
function main
    replace [program]
	P [program]
    construct Functions [repeat method_definition]
    	_ [^ P] 			% Extract all functions from program
	  [convertFunctionDefinitions] 	% Mark up with XML
    by 
    	Functions [removeOptSemis]
	          [removeEmptyStatements]
end function

rule convertFunctionDefinitions
    % Find each function definition and match its input source coordinates
    replace [method_definition]
	FileName [srcfilename] LineNumber [srclinenumber]
	FunctionHeader [method_header]
	'{
	    FunctionBody [opt statement_list]
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'}  Semi [opt ';]

    % Convert file name and line numbers to strings for XML
    construct FileNameString [stringlit]
	_ [quote FileName] 
    construct LineNumberString [stringlit]
	_ [quote LineNumber] 
    construct EndLineNumberString [stringlit]
	_ [quote EndLineNumber] 

    % Output is XML form with attributes indicating input source coordinates
    construct XmlHeader [xml_source_coordinate]
	<source file=FileNameString startline=LineNumberString endline=EndLineNumberString>
    by
	XmlHeader
	FunctionHeader 
	'{
	    FunctionBody % [unmarkEmbeddedFunctionDefinitions] 
	'}
	</source>
end rule

rule unmarkEmbeddedFunctionDefinitions
    replace [method_definition]
	FileName [srcfilename] LineNumber [srclinenumber]
	FunctionHeader [method_header]
	'{
	    FunctionBody [opt statement_list]
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'}
    by
	FunctionHeader 
	'{
	    FunctionBody 
	'}
end rule

rule removeOptSemis
    replace [opt ';]
	';
    by
	% none
end rule

rule removeEmptyStatements
    replace [repeat declaration_or_statement+]
	';
	More [repeat declaration_or_statement+]
    by
	More
end rule
