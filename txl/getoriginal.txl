% Program to fetch original source given the XML source coordinates
% Jim Cordy, April 2008

tokens
	charlit	""
end tokens

define program
	[xml_source_coordinate]
    |	[source_lines]
end define

define xml_source_coordinate
    '< 'source 'file=[stringlit] 'startline=[stringlit] 'endline=[stringlit] '> 
end define

define source_lines
	[repeat source_line]
end define

define source_line
	[repeat not_newline] [newline]
end define

define not_newline
	[not newline] [token]
end define

function main
	replace [program]
	    '< source 'file=File[stringlit] 'startline=Start[stringlit] 'endline=End[stringlit] '>  
	construct Source [repeat source_line]
	    _ [pragma "-char -nomultiline"] [read File] 
	construct StartLine [number]
	    _ [unquote Start]
	construct EndLine [number] 
	    _ [unquote End]
	by
	    Source [select StartLine EndLine]
end function
