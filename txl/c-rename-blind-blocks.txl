% Blind renaming - C blocks
% Jim Cordy, May 2010

% Using Gnu C grammar
include "c.grm"

redefine compound_statement
    { [IN] [NL]
        [compound_statement_body] [EX]
    } [NL]
end redefine

define potential_clone
    [compound_statement]
end define

% Generic blind renaming
include "generic-rename-blind.txl"

