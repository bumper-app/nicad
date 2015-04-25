% Example custom context-dependent normalization: 
% abstract only if-condition expressions in Java 
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

% Generic normalization
include "generic-normalize.txl"


% The custom context-dependent normalizing rule you want applied

rule normalize
    replace $ [if_statement]
        'if '( Condition [expression] ')     
            ThenPart [statement]
        ElsePart [opt else_clause]      
    by
        'if '( 'expression ')     
            ThenPart 
        ElsePart 
end rule
