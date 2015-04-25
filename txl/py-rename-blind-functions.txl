% Blind renaming - Python functions
% Jim Cordy, May 2010

% Using Python grammar
include "python.grm"

% Redefinition for potential clones
define block_funcdef
    'def [id] [parameters] ': 
    [indent] [endofline] 
	[repeat fstatement+] 
    [dedent] 
end define

define fstatement
    [repeat newline] [statement]
end define

define potential_clone
    [block_funcdef]
end define

redefine indent
	[newline] 'INDENT [IN]
end redefine

redefine dedent
	[EX] 'DEDENT [newline]
end redefine

% Generic blind renaming
include "generic-rename-blind.txl"

% Specialize for Python
redefine xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [newline]
end redefine

redefine end_xml_source_coordinate
    '< [SPOFF] '/ 'source '> [SPON] [newline]
end redefine
