% Generic filtering of nonterminals
% Jim Cordy, May 2010

define xml_source_coordinate
    '< [SPOFF] 'source [SP] 'file=[stringlit] [SP] 'startline=[stringlit] [SP] 'endline=[stringlit] '> [SPON] [NL]
end define

define end_xml_source_coordinate
    [NL] '< [SPOFF] '/ 'source '> [SPON] [NL]
end define

define source_unit  
    [xml_source_coordinate]
        [potential_clone]
    [end_xml_source_coordinate]
end define

redefine program
    [repeat source_unit]
end redefine

% Main program

rule main
    % Get the list of nonterminals to be filtered
    import TXLargs [stringlit*]
	FilteredNTs [stringlit*]

    % Make a global nothing
    construct Empty [empty]
    deconstruct * [any] Empty
	Nothing [any]
    export Nothing

    skipping [source_unit]
    replace $ [source_unit]
	BeginXML [xml_source_coordinate]
	    PC [potential_clone]
	EndXML [end_xml_source_coordinate]
    by
	BeginXML 
	    PC [filter each FilteredNTs]
	EndXML 
end rule

rule filter FilteredNTstring [stringlit]
    construct FilteredNT [id]
        _ [unquote FilteredNTstring]
    % Replace all the given NTs by nothing
    import Nothing [any]
    replace $ [any]
	Any [any]
    where 
	Any [istype FilteredNT]
    by
	Nothing
end rule
