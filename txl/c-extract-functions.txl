% Example using TXL 10.5a source coordinate extensions to extract
% a table of all function definitions with source coordinates

% Jim Cordy, January 2008

% Revised Aug 2012 - disallow ouput forms in input parse - JRC
% Revised July 2011 - ignore BOM headers in source
% Revised 30.04.08 - unmark embedded functions - JRC

% Using Gnu C grammar
include "c.grm"

% Ignore BOM headers from Windows
include "bom.grm"

% Redefinitions to collect source coordinates for function definitions as parsed input,
% and to allow for XML markup of function definitions as output

redefine function_definition
	% Input form 
	[srcfilename] [srclinenumber] 		% Keep track of starting file and line number
	[function_header]
	[opt KR_parameter_decls]
	'{ 				[IN][NL]
	    [compound_statement_body]	[EX]
	    [srcfilename] [srclinenumber] 	% Keep track of ending file and line number
	'}
    |
	% Output form 
        [not token]                             % disallow output form in input parse
	[opt xml_source_coordinate]
	[function_header]
	[opt KR_parameter_decls]
	'{ 				[IN][NL]
	    [compound_statement_body] 	[EX]
	'}
	[opt end_xml_source_coordinate]
end redefine

define xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [NL]
end define

define end_xml_source_coordinate
    [NL] '< [SPOFF] '/ 'source '> [SPON] [NL]
end define

redefine program
	...
    | 	[repeat function_definition]
end redefine


% Main function - extract and mark up function definitions from parsed input program
function main
    replace [program]
	P [program]
    construct Functions [repeat function_definition]
    	_ [^ P] 			% Extract all functions from program
	  [convertFunctionDefinitions] 	% Mark up with XML
    by 
    	Functions [removeOptSemis]
	          [removeEmptyStatements]
end function

rule convertFunctionDefinitions
    % Find each function definition and match its input source coordinates
    replace [function_definition]
	FileName [srcfilename] LineNumber [srclinenumber]
	FunctionHeader [function_header]
	KR_Parms [opt KR_parameter_decls]
	'{
	    FunctionBody [compound_statement_body]
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'}

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
	KR_Parms
	'{
	    FunctionBody [unmarkEmbeddedFunctionDefinitions]
	'}
	</source>
end rule

rule unmarkEmbeddedFunctionDefinitions
    replace [function_definition]
	FileName [srcfilename] LineNumber [srclinenumber]
	FunctionHeader [function_header]
	KR_Parms [opt KR_parameter_decls]
	'{
	    FunctionBody [compound_statement_body]
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'}
    by
	FunctionHeader 
	KR_Parms
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
    replace [repeat declaration_or_statement]
	';
	More [repeat declaration_or_statement]
    by
	More
end rule
