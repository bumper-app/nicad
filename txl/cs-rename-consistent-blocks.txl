% Consistent renaming - C# blocks
% Jim Cordy, May 2010

% Using C# grammar
include "csharp.grm"

% Redefinition for potential clones
redefine block
    { [IN] [NL]
	[opt statement_list] [EX]
    } [NL]
end redefine

define potential_clone
    [block]
end define

% Generic consistent renaming
include "generic-rename-consistent.txl"

