% Blind renaming - Java blocks
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

% Generic blind renaming
include "generic-rename-blind.txl"

