% Blind renaming - C functions
% Jim Cordy, May 2010

% Using Gnu C grammar
include "c.grm"

redefine function_definition
    [function_header]
    [opt KR_parameter_decls]
    '{                              [IN][NL]
        [compound_statement_body]   [EX]
    '}
end redefine

define potential_clone
    [function_definition]
end define

% Generic blind renaming
include "generic-rename-blind.txl"

