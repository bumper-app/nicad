% Example using TXL 10.5a source coordinate extensions to extract
% a table of all method definitions with source coordinates

% Jim Cordy, January 2008

% Revised Nov 2012 - remove @Override annotations from clone comparison - JRC
% Revised Aug 2012 - disallow ouput forms in input parse - JRC
% Revised July 2011 - ignore BOM headers in source
% Revised 25.03.11 - match constructors as methods - JRC
% Revised 30.04.08 - unmark embedded functions - JRC

% Using Java 5 grammar
include "java.grm"

% Ignore BOM headers from Windows
include "bom.grm"

% Redefinitions to collect source coordinates for function definitions as parsed input,
% and to allow for XML markup of function definitions as output

% Modified to match constructors as well.  Even though the grammar still
% has constructor_declaration in it, this one will match first. - JRC 25mar11

redefine method_declaration
	[repeat annotation]			% Remove @Override annotations from clone comparison
	[method_definition]
    |  
    	[method_header]
    	[opt annotation_default] ';               [NL][NL]
end redefine

define method_definition
	% Input form 
	[srcfilename] [srclinenumber] 		% Keep track of starting file and line number
	[method_header]
	'{                                        [NL][IN] 
	    [repeat declaration_or_statement]     [EX]
	    [srcfilename] [srclinenumber] 	% Keep track of ending file and line number
	'}
    |
	% Output form 
	[not token]				% disallow output form in input parse
	[opt xml_source_coordinate]
	[method_header]
	'{                                        [NL][IN] 
	    [repeat declaration_or_statement]     [EX]
	'}
	[opt end_xml_source_coordinate]
end define

define method_header
	[repeat modifier] [opt generic_parameter] [opt type_specifier] [method_declarator] [opt throws] 
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
	    FunctionBody [repeat declaration_or_statement]
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
	'{
	    FunctionBody [unmarkEmbeddedFunctionDefinitions] 
	'}
	</source>
end rule

rule unmarkEmbeddedFunctionDefinitions
    replace [method_definition]
	FileName [srcfilename] LineNumber [srclinenumber]
	FunctionHeader [method_header]
	'{
	    FunctionBody [repeat declaration_or_statement]
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
    replace [repeat declaration_or_statement]
	';
	More [repeat declaration_or_statement]
    by
	More
end rule
