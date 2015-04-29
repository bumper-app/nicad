% Consistent renaming - C blocks
% Jim Cordy, May 2010

% Using Gnu C grammmar
include "c.grm"

redefine compound_statement
    { [IN] [NL]
        [compound_statement_body] [EX]
    } [NL]
end redefine

define potential_clone
    [compound_statement]
end define

% Generic consistent renaming
include "generic-rename-consistent.txl"

