% Filter given nonterminals from potential clones - C block version

% Jim Cordy, May 2010

% Using Gnu C grammar
include "c.grm"

% Redefinition for potential clones

redefine compound_statement
    { [IN] [NL]
        [compound_statement_body] [EX]
    } [NL]
end redefine

define potential_clone
    [compound_statement]
end define

% Generic filter
include "generic-filter.txl"
