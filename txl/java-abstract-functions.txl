% Abstract given nonterminals from potential clones - Java functions version

% Jim Cordy, May 2010

% Using Java grammar
include "java.grm"

define method_definition
    [method_header]
    '{  [NL][IN] 
	[repeat declaration_or_statement]  [EX]
    '} 
end define

define method_header
	[repeat modifier] [opt generic_parameter] [opt type_specifier] [method_declarator] [opt throws] 
end define

define potential_clone
    [method_definition]
end define

% Generic nonterminal abstraction
include "generic-abstract.txl"
