% Extract function defintions from Python code

% Jim Cordy, January 2010

% Revised Aug 2012 - disallow ouput forms in input parse - JRC
% Revised July 2011 - ignore BOM headers in source

% Using Python grammar
include "python.grm"

% Ignore BOM headers from Windows
include "bom.grm"

% Redefinitions to collect source coordinates for function definitions as parsed input,
% and to allow for XML markup of function definitions as output

redefine funcdef
	[block_funcdef]
    |	[simple_funcdef]			% Remove one liners from potential clone output
end redefine

define simple_funcdef
    	'def [id] [parameters] ': [simple_statement] [endofline] 
end define

define block_funcdef
	% input form
	[srcfilename] [srclinenumber] 		% Keep track of starting file and line number
    	'def [id] [parameters] ': 
        [indent] [endofline] 
	    [opt doccomment] 			% Remove doc comments from potential clone output
            [repeat fstatement+] 
	    [srcfilename] [srclinenumber]
	    [repeat newline]
	[dedent] 
    |
	% output form
	[not token]				% disallow output form in input parse
	[opt xml_source_coordinate]
    	'def [id] [parameters] ': 
        [indent] [endofline] 
            [repeat fstatement+] 
	[dedent] 
	[opt end_xml_source_coordinate]
end define

define doccomment
	[longstringlit] [endofline]
    | 	[longcharlit] [endofline]
end define

define fstatement
	[repeat newline] [statement]
end define

redefine indent
	[NL] 'INDENT [IN]
end redefine

redefine dedent
	[EX] 'DEDENT [NL]
end redefine

define xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [NL]
end define

define end_xml_source_coordinate
    '< [SPOFF] '/ 'source '> [SPON] [NL]
end define

redefine program
	...
    | 	[repeat block_funcdef]
end redefine


% Main function - extract and mark up function definitions from parsed input program
function main
    replace [program]
	P [program]
    construct Functions [repeat block_funcdef]
    	_ [^ P] 			% Extract all functions from program
	  [removeEmptyFunctions]	% Any left over didn't have a statement in them
	  [convertFunctionDefinitions] 	% Mark up with XML
    by 
    	Functions
end function

rule convertFunctionDefinitions
    % Find each function definition and match its input source coordinates
    skipping [block_funcdef]
    replace [block_funcdef]
	FileName [srcfilename] LineNumber [srclinenumber]
    	'def FunctionId [id] Parameters [parameters] ': Indent [indent] EOL [endofline] 
	    DocComment [opt doccomment]
            FunctionBody [repeat fstatement+] 
	    EndFileName [srcfilename] EndLineNumber [srclinenumber] 
	    _ [repeat newline]
	Dedent [dedent] 

    % Convert file name and line numbers to strings for XML
    construct FileNameString [stringlit]
	_ [quote FileName] 
    construct LineNumberString [stringlit]
	_ [quote LineNumber] 
    construct EndLineNumberMinus2 [number]
	_ [pragma "--newline"] [parse EndLineNumber] [- 1]	% correct for DEDENT lines
    construct EndLineNumberString [stringlit]
	_ [quote EndLineNumberMinus2] 

    % Output is XML form with attributes indicating input source coordinates
    construct XmlHeader [xml_source_coordinate]
	<source file=FileNameString startline=LineNumberString endline=EndLineNumberString>
    by
	XmlHeader
	    'def FunctionId Parameters ': 
	    Indent EOL 
	        FunctionBody [unmarkEmbeddedFunctionDefinitions] 
			     [reduceEndOfLines] [reduceEndOfLines2]
			     [reduceNewlines] [reduceEmptyStmts]
	    Dedent
	</source>
end rule

rule unmarkEmbeddedFunctionDefinitions
    replace [block_funcdef]
	FileName [srcfilename] LineNumber [srclinenumber]
    	'def FunctionId [id] Parameters [parameters] ': Indent [indent] EOL [endofline] 
	    DocComment [opt doccomment]
            FunctionBody [repeat fstatement+] 
	    EndFileName [srcfilename] EndLineNumber [srclinenumber] 
	    _ [repeat newline]
	Dedent [dedent] 
    by
        'def FunctionId Parameters ': 
        Indent EOL 
	    FunctionBody 
        Dedent
end rule

rule reduceEndOfLines
    replace [repeat endofline]
	EOL [endofline]
	_ [endofline]
	_ [repeat endofline]
    by
	EOL
end rule

rule reduceEndOfLines2
    replace [opt endofline]
	EOL [endofline]
    by
end rule

rule reduceNewlines
    replace [repeat newline]
	Newlne [newline]
	_ [repeat newline]
    by
end rule

rule reduceEmptyStmts
    replace [repeat statement_or_newline]
	EOL [endofline]
	Stmts [repeat statement_or_newline]
    by
	Stmts
end rule

rule removeEmptyFunctions
    replace [repeat block_funcdef]
	FileName [srcfilename] LineNumber [srclinenumber]
    	'def FunctionId [id] Parameters [parameters] ': 
        Indent [indent] EOL [endofline] 
	    DocComment [opt doccomment]
            FunctionBody [repeat fstatement+] 
	    EndFileName [srcfilename] EndLineNumber [srclinenumber] 
	    _ [repeat newline]
	Dedent [dedent] 
	More [repeat block_funcdef]
    deconstruct not * [statement] FunctionBody
	Stmt [statement]
    by
	More
end rule
