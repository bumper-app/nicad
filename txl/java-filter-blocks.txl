% Filter given nonterminals from potential clones - Java block version

% Jim Cordy, May 2010

% Using Java grammar
include "java.grm"

% Redefinition for potential clones
redefine block
    { [IN] [NL]
	[repeat declaration_or_statement] [EX]
    } [NL]
end redefine

define potential_clone
    [block]
end define

% Generic nonterminal filtering
include "generic-filter.txl"
