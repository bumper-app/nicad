% ECMA-334 Edition 3 (June 2005) Standard C# Grammar
% Adapted directly from ECMA-334 (June 2005) Appendix A by Jim Cordy (cordy@cs.queensu.ca)
% March 2006 (Revised August 2006)

% Simple null program to test the C# grammar

% TXL ECMA-334 C# Grammar
include "csharp.grm"

% Comment out this line to use grammar without preserving comments
% #define COMMENTS

% Experimental overrides to allow for comment preservation
#ifdef COMMENTS
    include "Csharp_ECMA_334_CommentOverrides.Grm"
#endif

% Comment out this line to use grammar without preserving intentional extra newlines
% #define NEWLINES

% Experimental overrides to allow for preservation of intentional extra newlines
#ifdef NEWLINES
    include "Csharp_PreserveNewlines.Grm"
#endif

% Some of our examples are method fragments, not whole programs
redefine namespace_member_declaration
	...
    |   [method_declaration]
end redefine

% Temporary handling of designated Linq extensions
redefine invocation_operator
	...
    |	'(( [repeat argument_list_or_key] '))
end redefine

define argument_list_or_key
	[argument_list]
    |	'in
end define

% Just parse
function main
    replace [program] 
        P [program]
    by
	P
	#ifdef NEWLINES
	    [preserveNewlines]
	#endif
end function

