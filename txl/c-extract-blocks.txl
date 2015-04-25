% Example using TXL 10.5a source coordinate extensions to extract
% a table of all compound statements with source coordinates

% Jim Cordy, October 2009

% Revised Aug 2012 - disallow output forms in input parse - JRC
% Revised July 2011 - ignore BOM headers in source

% Using Gnu C grammar
include "c.grm"

% Ignore BOM headers from Windows
include "bom.grm"

% Redefinitions to collect source coordinates for function definitions as parsed input,
% and to allow for XML markup of function definitions as output

redefine compound_statement
	% Input form 
	[srcfilename] [srclinenumber] 		% Keep track of starting file and line number
	{ [IN] [NL]
	    [compound_statement_body] [EX]
	    [srcfilename] [srclinenumber] 	% Keep track of ending file and line number
        } [opt ';] [NL]
    |
	% Output form 
	[not token]				% disallow input parse of this form
	[opt xml_source_coordinate]
	{ [IN] [NL]
	    [compound_statement_body] [EX]
        } [opt ';] [NL]
	[opt end_xml_source_coordinate]
end redefine

define xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [NL]
end define

define end_xml_source_coordinate
    '< [SPOFF] '/ 'source '> [SPON] [NL]
end define

redefine program
	...
    | 	[repeat compound_statement]
end redefine


% Main function - extract and mark up compound statements from parsed input program
function main
    replace [program]
	P [program]
    construct Compounds [repeat compound_statement]
    	_ [^ P] 	% Extract all compound statements from program
	  [convertCompoundStatements]
    by 
    	Compounds [removeOptSemis]
	          [removeEmptyStatements]
end function

rule convertCompoundStatements
    % Find each compound statement and match its input source coordinates
    skipping [compound_statement]
    replace [compound_statement]
	FileName [srcfilename] LineNumber [srclinenumber]
	'{ 
	    CompoundBody [compound_statement_body] 
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'} Semi [opt ';]

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
	{ 
	    CompoundBody [unmarkEmbeddedCompoundStatements] 
	'}
	</source>
end rule

rule unmarkEmbeddedCompoundStatements
    replace [compound_statement]
	FileName [srcfilename] LineNumber [srclinenumber]
	'{ 
	    CompoundBody [compound_statement_body] 
	    EndFileName [srcfilename] EndLineNumber [srclinenumber]
	'} Semi [opt ';]
    construct Empty [opt xml_source_coordinate] 
	% none, to force output form
    by
	Empty
	'{
	    CompoundBody 
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
