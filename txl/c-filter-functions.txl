% Filter given nonterminals from potential clones - C function version

% Jim Cordy, May 2010

% Using Gnu C grammar
include "c.grm"

% Redefinition for potential clones
redefine function_definition
    [function_header]
    [opt KR_parameter_decls]
    '{ 					[IN][NL]
	[compound_statement_body] 	[EX]
    '} 
end redefine

define potential_clone
    [function_definition]
end define

% Generic filter
include "generic-filter.txl"

