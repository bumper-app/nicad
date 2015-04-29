% Consistent renaming - Java blocks
% Jim Cordy, May 2010

% Using Java 5 grammar
include "java.grm"

redefine block
    { [IN] [NL]
        [repeat declaration_or_statement] [EX]
    } [NL]
end redefine

define potential_clone
    [block]
end define

% Generic consistent renaming
include "generic-rename-consistent.txl"

