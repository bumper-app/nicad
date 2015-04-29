% Extract and expand WSDL operations definitions from WSDL files
% Doug Martin, Queen's University
% Summer 2009

% Revised July 2011 - ignore BOM headers in source

include "wsdl.grm"

% Ignore BOM headers from Windows
include "bom.grm"

define source
    [SPOFF] '<'source [opt attribute_list] '> [SPON]                       [NL][IN]
        [repeat element]                                                   [EX]
    [SPOFF] '</'source> [SPON]                                             [NL]
end define

redefine program 
        ...
    |    [repeat source]
end redefine

redefine complex_type
    [SPOFF] '< [opt prefix] 'complexType [opt attribute_list] '> [SPON]     [NL][IN]
        [repeat element]                                                    [EX]
    [SPOFF] '</ [opt prefix] 'complexType> [SPON]                           [NL]
end redefine

redefine message
    [SPOFF] '< [opt prefix] 'message [opt attribute_list] '> [SPON]         [NL][IN]
        [repeat element]                                                    [EX]
    [SPOFF] '</ [opt prefix] 'message> [SPON]                               [NL]
end redefine

redefine element
        ...
    |    [SPOFF] '< [opt prefix] 'element [opt attribute_list] '> [SPON]    [NL][IN]
            [repeat element]                                                [EX]
        [SPOFF] '</ [opt prefix] 'element> [SPON]                           [NL]
end redefine

redefine operation 
        [attr srcfilename] [attr srclinenumber] 
        [SPOFF] '< [opt prefix] 'operation [opt attribute_list] [SPON] '>   [NL][IN]
            [repeat operation_scope]                                        [EX]
        [attr srclinenumber] 
	[SPOFF] '</ [opt prefix] 'operation> [SPON]                         [NL]
    |  
        [attr srcfilename] [attr srclinenumber] 
	[SPOFF] '< [opt prefix] 'operation [opt attribute_list] '/> [SPON]  [NL]
end redefine

redefine schema
        ...
    |    [xml_def]
end redefine
    
function main
    replace [program]
        P [program]
    construct AllPortTypes [repeat port_type]
        _ [^ P]
    construct AllOperations [repeat operation]
        _ [^ AllPortTypes]
    construct AllSchema [repeat schema]
        _ [^ P]
    construct AllSchemaElmts [repeat schema_element]
        _ [^ P]
    construct AllComplexTypes [repeat complex_type]
        _ [^ P]
    construct AllMessages [repeat message]
        _ [^ P]
    deconstruct P
        _ [opt xml_def]
        '< _ [opt prefix] 'definitions DefAttrList [opt attribute_list] '>
            _ [repeat top_level_elements] 
        '</ _ [opt prefix] 'definitions>
    construct DefNamespaces [repeat xmlns_attr]
        _ [^ DefAttrList]
    export Schema [repeat schema]
        AllSchema
    export AllSchemaElements [repeat schema_element]
        AllSchemaElmts
    export ComplexTypes [repeat complex_type]
        AllComplexTypes
    export Messages [repeat message]
        AllMessages
    export AllNamespaces [repeat xmlns_attr]
        DefNamespaces
    construct Operations [repeat source]
        _ [constructOperations each AllOperations]
    by
        Operations
end function 

function constructOperations Operation [operation]
    replace [repeat source]
        PreviousOperations [repeat source]
    deconstruct Operation 
        File [srcfilename] BeginLine [srclinenumber] '< _ [opt prefix] 'operation AttrList [opt attribute_list] '>     
            Contents [repeat operation_scope]     
        EndLine [srclinenumber] '</ _ [opt prefix] 'operation>
    construct Filename [stringlit]
        _ [quote File]
    construct BeginLineNumber [stringlit]
        _ [quote BeginLine]
    construct EndLineNumber [stringlit]
        _ [quote EndLine]
    construct Inputs [repeat input]
        _ [^ Contents]
    construct Outputs [repeat output]
        _ [^ Contents]
    construct Infaults [repeat infault]
        _ [^ Contents]
    construct Outfaults [repeat outfault]
        _ [^ Contents]
    construct Faults [repeat fault]
        _ [^ Contents]
    construct NewInputs [repeat operation_scope]
        _ [constructInputs each Inputs]
    construct NewOutputs [repeat operation_scope]
        _ [constructOutputs each Outputs]
    construct NewInfaults [repeat operation_scope]
        _ [constructInfaults each Infaults]
    construct NewOutfaults [repeat operation_scope]
        _ [constructOutfaults each Outfaults]    
    construct NewFaults [repeat operation_scope]
        _ [constructFaults each Faults]
    construct NewOperationContents [repeat operation_scope]
        NewInputs [. NewOutputs] [. NewInfaults] [. NewOutfaults] [. NewFaults]
    construct NewOperation [operation]
        '<'operation AttrList '>
            NewOperationContents
        '</'operation>
    construct SourceTag [source]
        '<'source 'file= Filename 'startline= BeginLineNumber 'endline= EndLineNumber '>
            NewOperation
        '</'source>
    by 
        PreviousOperations [. SourceTag]
end function

function constructInputs Input [input]
    replace [repeat operation_scope]
        PreviousInputs [repeat operation_scope]
    import Messages [repeat message]
    construct AttrList [repeat attribute]
        _ [^ Input]
    construct MessageAttr [repeat message_attr]
        _ [^ AttrList]
    deconstruct MessageAttr
        _ [opt prefix] 'message = MessageName [stringlit] 
        _ [repeat message_attr]
    construct MessageNameNoPrefix [stringlit]
        MessageName [deletePrefix]
    construct NewInputContents [repeat element]
        _ [constructMessages MessageNameNoPrefix each Messages]
    construct NewInput [operation_scope]
        '<'input AttrList '>
            NewInputContents
        '</'input>
    by 
        PreviousInputs [. NewInput]
end function 

function constructOutputs Output [output]
    replace [repeat operation_scope]
        PreviousOutputs [repeat operation_scope]
    import Messages [repeat message]
    construct AttrList [repeat attribute]
        _ [^ Output]
    construct MessageAttr [repeat message_attr]
        _ [^ AttrList]
    deconstruct MessageAttr
        _ [opt prefix] 'message = MessageName [stringlit] 
        _ [repeat message_attr]
    construct MessageNameNoPrefix [stringlit]
        MessageName [deletePrefix]
    construct NewOutputContents [repeat element]
        _ [constructMessages MessageNameNoPrefix each Messages]
    construct NewOutput [operation_scope]
        '<'output AttrList '>
            NewOutputContents
        '</'output>
    by 
        PreviousOutputs [. NewOutput]
end function 

function constructInfaults Infault [infault]
    replace [repeat operation_scope]
        PreviousInfaults [repeat operation_scope]
    import Messages [repeat message]
    construct AttrList [repeat attribute]
        _ [^ Infault]
    construct MessageAttr [repeat message_attr]
        _ [^ AttrList]
    deconstruct MessageAttr
        _ [opt prefix] 'message = MessageName [stringlit] 
        _ [repeat message_attr]
    construct MessageNameNoPrefix [stringlit]
        MessageName [deletePrefix]
    construct NewInfaultContents [repeat element]
        _ [constructMessages MessageNameNoPrefix each Messages]
    construct NewInfault [operation_scope]
        '<'infault AttrList '>
            NewInfaultContents
        '</'infault>
    by 
        PreviousInfaults [. NewInfault]
end function 

function constructOutfaults Outfault [outfault]
    replace [repeat operation_scope]
        PreviousOutfaults [repeat operation_scope]
    import Messages [repeat message]
    construct AttrList [repeat attribute]
        _ [^ Outfault]
    construct MessageAttr [repeat message_attr]
        _ [^ AttrList]
    deconstruct MessageAttr
        _ [opt prefix] 'message = MessageName [stringlit] 
        _ [repeat message_attr]
    construct MessageNameNoPrefix [stringlit]
        MessageName [deletePrefix]
    construct NewOutfaultContents [repeat element]
        _ [constructMessages MessageNameNoPrefix each Messages]
    construct NewOutfault [operation_scope]
        '<'outfault AttrList '>
            NewOutfaultContents
        '</'outfault>
    by 
        PreviousOutfaults [. NewOutfault]
end function 

function constructFaults Fault [fault]
    replace [repeat operation_scope]
        PreviousFaults [repeat operation_scope]
    import Messages [repeat message]
    construct AttrList [repeat attribute]
        _ [^ Fault]
    construct MessageAttr [repeat message_attr]
        _ [^ AttrList]
    deconstruct MessageAttr
        _ [opt prefix] 'message = MessageName [stringlit] 
        _ [repeat message_attr]
    construct MessageNameNoPrefix [stringlit]
        MessageName [deletePrefix]
    construct NewFaultContents [repeat element]
        _ [constructMessages MessageNameNoPrefix each Messages]
    construct NewFault [operation_scope]
        '<'fault AttrList '>
            NewFaultContents
        '</'fault>
    by 
        PreviousFaults [. NewFault]
end function 

function constructMessages Name [stringlit] Message [message]
    replace [repeat element]
        PreviousMessages [repeat element]
    deconstruct Message
        '< _ [opt prefix] 'message AttrList [opt attribute_list] '>     
            MessageContents [repeat element]     
        '</ _ [opt prefix] 'message>
    construct NameAttr [repeat name_attr]
        _ [^ AttrList]
    deconstruct NameAttr 
        _ [opt prefix] 'name = MessageName [stringlit] 
        _ [repeat name_attr]
    deconstruct MessageName
        Name
    construct Parts [repeat part]
        _ [^ MessageContents]
    construct NewMessageContents [repeat element]
        _ [constructParts each Parts]
    construct NewMessage [element]
        '<'message AttrList '>     
            NewMessageContents    
        '</'message>
    by 
        PreviousMessages [. NewMessage]
end function

function constructParts Part [part]
    replace [repeat element]
        PreviousParts [repeat element]
    import AllNamespaces [repeat xmlns_attr]
    import Schema [repeat schema]
    construct AttrList [repeat attribute]
        _ [^ Part]
    construct ElmtAttr [repeat element_attr]
        _ [^ AttrList]
    deconstruct ElmtAttr
        _ [opt prefix] 'element = ElmtName [stringlit] 
        _ [repeat element_attr]
    construct Prefix [stringlit]
        ElmtName [deleteSuffix]
    construct MatchingNamespace [stringlit]
        _ [getMatchingNamespaceURI Prefix each AllNamespaces]
    construct MatchingSchema [repeat schema]
        _ [getMatchingSchema MatchingNamespace each Schema]
    construct SchemaElements [repeat schema_element]
        _ [^ MatchingSchema]
    construct ElementNameNoPrefix [stringlit]
        ElmtName [deletePrefix] 
    construct MatchingElements [repeat element]
        _ [getMatchingSchemaElements ElementNameNoPrefix each SchemaElements]
    construct ProcessedMatchingElements [repeat element]
        MatchingElements [stripComplexTypes1] [stripComplexTypes2] [stripComplexTypes3] [stripComplexTypes4] [stripComplexTypes5] [stripComplexTypes6]
    construct NewPart [element]
        '<'part AttrList '> 
            ProcessedMatchingElements
        '</'part> 
    by
        PreviousParts [. NewPart]
end function

function getMatchingSchemaElements Name [stringlit] Elmt [schema_element]
    replace [repeat element]
        PreviousElements [repeat element]
    construct ElmtAttrList [opt attribute_list]
        _ [getSingletonElementAttrList Elmt] [getElementAttrList Elmt]
    construct NameAttr [repeat name_attr]
        _ [^ Elmt]
    construct TypeAttr [repeat type_attr]
        _ [^ Elmt]
    deconstruct NameAttr
        _ [opt prefix] 'name = ElmtName [stringlit] 
        _ [repeat name_attr]
    deconstruct Name
        ElmtName
        
    import AllNamespaces [repeat xmlns_attr]
    import Schema [repeat schema]
    
    % this whole thing fails if there is no type
    % deconstruct TypeAttr
        % _ [opt prefix] 'type = TypeString [stringlit] 
        % _ [repeat type_attr]
    
    % construct Prefix [stringlit]
        % TypeString [deleteSuffix]
    % construct MatchingNamespace [stringlit]
        % _ [getMatchingNamespaceURI Prefix each AllNamespaces]
    % construct MatchingSchema [repeat schema]
        % _ [getMatchingSchema MatchingNamespace each Schema]
    construct SchemaTypes [repeat complex_type]
        _ [^ Schema]

    
    construct ParentAndChildren [repeat schema_element]
        _ [^ Elmt]
    
    deconstruct ParentAndChildren
        _ [schema_element] %  throw away Parent
        Children [repeat schema_element]
        
    construct ComplexChildren [repeat element]
        _ [processChildren each Children]
    construct NewElmt [element]
        Elmt
    construct ComplexElement [element]
        NewElmt [createComplexElement ComplexChildren ElmtAttrList] [stripPrefix]
    by
        PreviousElements [. ComplexElement]
end function

function processChildren Elmt [schema_element] 
    replace [repeat element]
        PreviousElements [repeat element]
    construct ElmtAttrList [opt attribute_list]
        _ [getSingletonElementAttrList Elmt] [getElementAttrList Elmt]
    construct TypeAttr [repeat type_attr]
        _ [^ ElmtAttrList]
    import AllNamespaces [repeat xmlns_attr]
    import Schema [repeat schema]
    
    deconstruct TypeAttr
        _ [opt prefix] 'type = TypeString [stringlit] 
        _ [repeat type_attr]
    
    construct Prefix [stringlit]
        TypeString [deleteSuffix]
    construct MatchingNamespace [stringlit]
        _ [getMatchingNamespaceURI Prefix each AllNamespaces]
    construct MatchingSchema [repeat schema]
        _ [getMatchingSchema MatchingNamespace each Schema]
    construct SchemaTypes [repeat complex_type]
        _ [^ MatchingSchema]
        
    construct Types [repeat element]
        _ [getMatchingTypes TypeAttr each SchemaTypes]
    
    construct NewElmt [element]
        Elmt
    construct ComplexElement [element]
        NewElmt [createComplexElement Types ElmtAttrList] [stripPrefix]
    
    by
        PreviousElements [. ComplexElement]
end function

function getMatchingTypes TypeAttr [repeat type_attr] ComplexType [complex_type]
    deconstruct ComplexType
        '< _ [opt prefix] 'complexType AttrList [opt attribute_list] '> 
            Contents [repeat element]
        '</ _ [opt prefix] 'complexType>
    replace [repeat element]
        PreviousTypes [repeat element]
    deconstruct TypeAttr
        _ [opt prefix] 'type = ElmtType [stringlit] 
        _ [repeat type_attr]
    construct ElmtTypeNoPrefix [stringlit]
        ElmtType [deletePrefix]
    construct TypeNameAttr [repeat name_attr]
        _ [^ AttrList]
    deconstruct TypeNameAttr
        _ [opt prefix] 'name = TypeName [stringlit] 
        _ [repeat name_attr]
    deconstruct TypeName
        ElmtTypeNoPrefix
    
    construct SchemaElements [repeat schema_element]
        _ [^ Contents]
    construct ComplexElements [repeat element]
        _ [constructComplexElements each SchemaElements]
    by
        PreviousTypes [. ComplexElements]
end function

function constructComplexElements SchemaElement [schema_element]
    replace [repeat element]
        PreviousElements [repeat element]
    construct AttrList [opt attribute_list]
        _ [getSingletonElementAttrList SchemaElement] [getElementAttrList SchemaElement]
    construct TypeAttr [repeat type_attr]
        _ [^ AttrList]
    
    import AllNamespaces [repeat xmlns_attr]
    import Schema [repeat schema]
    
    deconstruct TypeAttr
        _ [opt prefix] 'type = TypeString [stringlit] 
        _ [repeat type_attr]
    
    construct Prefix [stringlit]
        TypeString [deleteSuffix]
    construct MatchingNamespace [stringlit]
        _ [getMatchingNamespaceURI Prefix each AllNamespaces]
    construct MatchingSchema [repeat schema]
        _ [getMatchingSchema MatchingNamespace each Schema]
    construct SchemaTypes [repeat complex_type]
        _ [^ MatchingSchema]
    
    construct Types [repeat element]
        _ [getMatchingTypes TypeAttr each SchemaTypes]
    construct NewElmt [element]
        SchemaElement
    construct ComplexElement [element]
        NewElmt [createComplexElement Types AttrList] [stripPrefix]
    by
        PreviousElements [. ComplexElement]
end function

function deletePrefix 
    replace [stringlit]
        NameWithPrefix [stringlit]
    construct IndexOfColon [number]
        _ [index NameWithPrefix ":"] [+ 1]
    deconstruct not IndexOfColon
        1
    by
        NameWithPrefix [: IndexOfColon 999]
end function

function deleteSuffix 
    replace [stringlit]
        NameWithSuffix [stringlit]
    construct IndexOfColon [number]
        _ [index NameWithSuffix ":"] [- 1]
    deconstruct not IndexOfColon
        1
    by
        NameWithSuffix [: 0 IndexOfColon]
end function

function getSingletonElementAttrList Elmt [schema_element]
    deconstruct Elmt
        '< _ [opt prefix] 'element AttrList [opt attribute_list] '/> 
    replace [opt attribute_list]
        _ [opt attribute_list]
    by 
        AttrList
end function

function getElementAttrList Elmt [schema_element]
    deconstruct Elmt
        '< _ [opt prefix] 'element AttrList [opt attribute_list] '> 
            _ [repeat schema_scope]
        '</ _ [opt prefix] 'element>
    replace [opt attribute_list]
        _ [opt attribute_list]
    by 
        AttrList
end function

function createComplexElement Contents [repeat element] AttrList [opt attribute_list]
    deconstruct Contents
        _ [element]
        _ [repeat element]
    replace [element]
        _ [element]
    by
        '<'element AttrList '>
            Contents
        '</'element>
end function

function stripPrefix
    replace [element]
        '< _[opt prefix] 'element AttrList [attribute_list] '/>
    by
        '<'element AttrList '/>
end function

function getMatchingNamespaceURI Prefix [stringlit] Namespace [xmlns_attr]
    replace [stringlit]
        _ [stringlit]
    deconstruct Namespace 
        'xmlns: PrefixID [id] '= NamespaceURI [stringlit]
    construct PrefixString [stringlit]
        _ [quote PrefixID]
    deconstruct PrefixString 
        Prefix
    by 
        NamespaceURI
end function

function getMatchingSchema Namespace [stringlit] Schema [schema]
    replace [repeat schema]
        PreviousSchema [repeat schema]
    construct TargetNamespace [repeat target_namespace_attr]
        _ [^ Schema]
    deconstruct TargetNamespace 
        _ [opt prefix] 'targetNamespace = URI [stringlit] 
        _ [repeat target_namespace_attr]
    deconstruct URI
        Namespace
    by 
        PreviousSchema [. Schema]
end function

rule stripComplexTypes1
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'sequence>
                    Contents [repeat element]
                '</ _[opt prefix] 'sequence>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '> 
            Contents
        '</'element>
    by
        NewElmt
end rule

rule stripComplexTypes2
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'sequence>
                '</ _[opt prefix] 'sequence>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '/> 
    by
        NewElmt
end rule

rule stripComplexTypes3
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'sequence/>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '/> 
    by
        NewElmt
end rule

rule stripComplexTypes4
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'all>
                    Contents [repeat element]
                '</ _[opt prefix] 'all>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '> 
            Contents
        '</'element>
    by
        NewElmt
end rule

rule stripComplexTypes5
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'all>
                '</ _[opt prefix] 'all>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '/> 
    by
        NewElmt
end rule

rule stripComplexTypes6
    replace [element]
        '< _[opt prefix] 'element AttrList [opt attribute_list] '> 
            '< _[opt prefix] 'complexType>
                '< _[opt prefix] 'all/>
            '</ _[opt prefix] 'complexType>
        '</ _ [opt prefix] 'element>
    construct NewElmt [element]
        '<'element AttrList '/> 
    by
        NewElmt
end rule


