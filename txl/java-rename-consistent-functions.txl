% Consistent renaming - Java functions
% Jim Cordy, May 2010

% Using Java 5 grammar
include "java.grm"

define method_definition
    [method_header]
    '{                                        [NL][IN] 
	[repeat declaration_or_statement]     [EX]
    '}
end define

define method_header
    [repeat modifier] [opt generic_parameter] [opt type_specifier] [method_declarator] [opt throws] 
end define

define potential_clone
    [method_definition]
end define

% Generic consistent renaming
include "generic-rename-consistent.txl"

