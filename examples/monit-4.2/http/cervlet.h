/*
 * Copyright (C), 2000-2004 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef CERVLET_H
#define CERVLET_H

#include <config.h>

#include "monitor.h"

#define HEAD_HTML \
"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"\
"<html>"\
"<head>"\
"  <title>monit: service manager</title>"\
"  <style type=\"text/css\">"\
"  <!--"\
"  BODY {"\
"	margin: 0;"\
"  }"\
"  BODY, P, DIV, TD, TH, TR, FORM, OL, UL, LI, INPUT, TEXTAREA, SELECT, A {"\
"  	font-family: Tahoma, Arial, Helvetica, sans-serif;"\
"  	font-size: 14px; "\
"  }"\
"  A:hover {"\
"  	text-decoration: none;"\
"  }"\
"  A  {"\
"  	text-decoration: underline;"\
"  }"\
"  .grey {"\
"        color: #666666;"\
"  } "\
"  -->"\
"  </style>"\
"<meta HTTP-EQUIV=\"REFRESH\" CONTENT=%d>"\
"<meta HTTP-EQUIV=\"Expires\" Content=0>"\
"<meta HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">"\
"</head>"\
"<body bgcolor=\"#ffffff\" link=\"#000000\" vlink=\"#000000\""\
" alink=\"#000000\" text=\"#000000\">"\
"<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%%\" border=\"0\">"\
" <tr bgcolor=\"#6F6F6F\">"\
"  <td valign=\"bottom\"><img src=\"/_pixel\" width=\"1\" height=\"1\" alt=\"\"></td>"\
"  </tr>"\
"</table>"\
"<table cellspacing=\"0\" cellpadding=\"10\" width=\"100%%\" border=\"0\">"\
"  <tr bgcolor=\"#DDDDDD\">"\
"    <td align=\"left\" valign=\"top\" width=\"221\" bgcolor=\"#DDDDDD\">"\
"      </a><font color=\"#000000\"><a href=\"http://www.tildeslash.com\"><small><a href='/'>Home</a>&gt;"\
"             <a href='/%s'>%s</a></small></a></font>"\
"	</td>"\
"    <td align=\"left\" valign=\"top\" width=\"918\" bgcolor=\"#DDDDDD\">"\
"      <p align=\"right\">"\
"      <small><a href='/_about'>monit " VERSION "</a></small>"\
"	</td>"\
"  </tr>"\
"</table>"\
"<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%%\" border=\"0\">"\
"  <tr bgcolor=\"#6F6F6F\">"\
"    <td><img src=\"/_pixel\" width=\"1\" height=\"1\" alt=\"\"></td>"\
"  </tr>"\
"</table>"\
"<center>"


#define FOOT_HTML "</center>"\
"<p>&nbsp;</p>"\
"<p>&nbsp;</p>"\
"<p><br>"\
"</p>"\
"<div align=\"center\"><font color=\"#666666\">"\
"<small>"\
"&copy; Copyright 2000-2004 by <a class=grey "\
"href=\"http://www.tildeslash.com/monit/who.html\">the monit project group</a>."\
" All Rights Reserved. </small></font></div><br></body></html>"


#define HEAD(location, refresh) \
   out_print(res, HEAD_HTML, refresh, location, location);

#define FOOT  out_print(res, FOOT_HTML);


#define PIXEL_GIF "R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw=="


/* Public prototypes */
void init_service();

#endif
