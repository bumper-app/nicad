% Generic abstraction of nonterminals
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
    % Get the list of nonterminals to be abstracted
    import TXLargs [stringlit*]
	AbstractedNTs [stringlit*]

    % Abstract them in each potential clone
    skipping [source_unit]
    replace $ [source_unit]
	BeginXML [xml_source_coordinate]
	    PC [potential_clone]
	EndXML [end_xml_source_coordinate]
    by
	BeginXML 
	    PC [abstract each AbstractedNTs]
	EndXML 
end rule

rule abstract AbstractedNTstring [stringlit]
    construct AbstractedNT [id]
        _ [unquote AbstractedNTstring]
    % Replace all the given NTs by their nonterminal name
    deconstruct * [any] AbstractedNT
	AbstractedNTname [any]
    replace $ [any]
	Any [any]
    where 
	Any [istype AbstractedNT]
    by
	AbstractedNTname
end rule
