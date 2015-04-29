% Abstract given nonterminals from potential clones - C# function version

% Jim Cordy, May 2010

% Using C# grammar
include "csharp.grm"

% Redefinition for potential clones

% Temporary handling of designated Linq extensions
redefine invocation_operator
	...
    |	'(( [repeat argument_list_or_key] '))
end redefine

define argument_list_or_key
	[argument_list]
    |	'in
end define

define method_definition
    [method_header]				
    '{  [NL][IN] 
	[opt statement_list]  [EX]
    '}  
end define

define potential_clone
    [method_definition]
end define

% Generic abstract
include "generic-abstract.txl"

