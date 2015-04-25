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


%{
  
/*
 * DESCRIPTION
 *   Lexical grammar for tokenizing the control file. 
 *
 *  AUTHOR
 *   Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *   Olivier Beyssac, <ob@r14.freenix.org>
 *   Christian Hopp, <chopp@iei.tu-clausthal.de>
 *   Martin Pala, <martinp@tildeslash.com>
 *
 *  CVS INFO
 *     $Id: l.l,v 1.76 2004/03/02 10:31:51 chopp Exp $
 */

#include <config.h>
  
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "monitor.h"
#include "y.tab.h"

#define MAX_STACK_DEPTH 128

  int buffer_stack_ptr=0;

  struct buffer_stack_s {
    int             lineno;
    char           *currentfile;
    YY_BUFFER_STATE buffer;
  } buffer_stack[MAX_STACK_DEPTH];
  
  
  int lineno= 1;
  int arglineno= 1;
  char *currentfile=NULL;
  char *argcurrentfile=NULL;
  char *argyytext=NULL;

  
  /* Prototypes */
  extern void yyerror(const char*,...);
  static void steplinenobycr(char *);
  static void save_arg(void);
  static void include_file(char *);
  static char *handle_quoted_string(char *);
  static void push_buffer_state(YY_BUFFER_STATE, char*);
  static int  pop_buffer_state(void);
  
%}

ws          [ \t]+
wws         [ \t=;,()]+
number      [0-9]+
real        [0-9]+([.][0-9]+)?
str         [^\000-\041@:{}"';(),]+
hostname    {str}(\.{str})*
greater     ("greater"|"gt"|">")
less        ("less"|"lt"|"<")
equal       ("equal"|"eq"|"==")
notequal    ("notequal"|"ne"|"!=")
loadavg1    load(avg)[ ]*(\([ ]*1[ ]*(m|min)?[ ]*\))?
loadavg5    load(avg)[ ]*\([ ]*5[ ]*(m|min)?[ ]*\)
loadavg15   load(avg)[ ]*\([ ]*15[ ]*(m|min)?[ ]*\)
startarg    start{wws}?(program)?{wws}?["]
stoparg     stop{wws}?(program)?{wws}?["]
execarg     exec(ute)?{wws}?["]
percent     ([ ]*"percent"|[ ]*"%")
byte        ("byte"|"b")
kilobyte    ("kilobyte"|"kb")
megabyte    ("megabyte"|"mb")
gigabyte    ("gigabyte"|"gb")

%x ARGUMENT_COND DEPEND_COND SERVICE_COND INCLUDE

%%

{wws}             { /* Wide white space */ }
(#.*)?\\?\n       { lineno++; } 

is                {/* EMPTY */}
as                {/* EMPTY */}
are               {/* EMPTY */}
for               {/* EMPTY */}
on(ly)?           {/* EMPTY */}
with(in)?         {/* EMPTY */}
program(s)?       {/* EMPTY */}
and               {/* EMPTY */}
has               {/* EMPTY */}
using             {/* EMPTY */}
use               {/* EMPTY */}
the               {/* EMPTY */}
sum               {/* EMPTY */}
than              {/* EMPTY */}
usage             {/* EMPTY */}
was               {/* EMPTY */}

{startarg}        { BEGIN(ARGUMENT_COND); return START; }
{stoparg}         { BEGIN(ARGUMENT_COND); return STOP; }
{execarg}         { BEGIN(ARGUMENT_COND); return EXEC; }

if                { return IF; }
then              { return THEN; }
failed            { return FAILED; }
ssl               { return HTTPDSSL; }
enable            { return ENABLE; }
disable           { return DISABLE; }
set               { return SET; }
daemon            { return DAEMON; }
logfile           { return LOGFILE; }
syslog            { return SYSLOG; }
facility          { return FACILITY; }
httpd             { return HTTPD; }
address           { return ADDRESS; }
clientpemfile     { return CLIENTPEMFILE; }
allowselfcertification  { return ALLOWSELFCERTIFICATION; }
certmd5           { return CERTMD5; }
pemfile           { return PEMFILE; }
init              { return INIT; }
allow             { return ALLOW; }
read[-]?only      { return READONLY; }
pidfile           { return PIDFILE; }
statefile         { return STATEFILE; }
path              { return PATHTOK; }
start             { return START; }
stop              { return STOP; }
port(number)?     { return PORT; }
unix(socket)?     { return UNIXSOCKET; }
type              { return TYPE; }
proto(col)?       { return PROTOCOL; }
tcp               { return TCP; }
tcpssl            { return TCPSSL; }
udp               { return UDP; }
alert             { return ALERT; }
mail-format       { return MAILFORMAT; }
resource          { return RESOURCE; }
restart(s)?       { return RESTART; }
cycle(s)?         { return CYCLE;}
timeout           { return TIMEOUT; }
checksum          { return CHECKSUM; }
mailserver        { return MAILSERVER; }
every             { return EVERY; }
host              { return HOST; }
default           { return DEFAULT; }
http              { return HTTP; }
ftp               { return FTP; }
smtp              { return SMTP; }
pop               { return POP; }
imap              { return IMAP; }
nntp              { return NNTP; }
ssh               { return SSH; }
dwp               { return DWP; }
ldap2             { return LDAP2; }
ldap3             { return LDAP3; }
rdate             { return RDATE; }
rsync             { return RSYNC; }
mode              { return MODE; }
active            { return ACTIVE; }
passive           { return PASSIVE; }
manual            { return MANUAL; }
group             { return GROUP; }
uid               { return UID; }
gid               { return GID; }
request           { return REQUEST; }
cpuusage          { return CPUUSAGE; }
memusage          { return MEMUSAGE; }
memkbyte          { return MEMKBYTE; }
mem(ory)?         { return MEMORY; }
totalmem(ory)?    { return TOTALMEMORY; }
cpu               { return CPU; }
child(ren)        { return CHILDREN; }
timestamp         { return TIMESTAMP; }
changed           { return CHANGED; }
second(s)?        { return SECOND; }
minute(s)?        { return MINUTE; }
hour(s)?          { return HOUR; }
day(s)?           { return DAY; }
sslv2             { return SSLV2; }
sslv3             { return SSLV3; }
tlsv1             { return TLSV1; }
sslauto           { return SSLAUTO; }
inode(s)?         { return INODE; }
space             { return SPACE; }
perm(ission)?     { return PERMISSION; }
exec(ute)?        { return EXEC; }
size              { return SIZE; }
connection        { return CONNECTION; }
unmonitor         { return UNMONITOR; }
icmp              { return ICMP; }
echo              { return ICMPECHO; }
send              { return SEND; }
expect            { return EXPECT; }
cleartext         { return CLEARTEXT; }
md5               { return MD5HASH; }
sha1              { return SHA1HASH; }
crypt             { return CRYPT; }
signature         { return SIGNATURE; }
include           { BEGIN(INCLUDE); }

{byte}            { return BYTE; }
{kilobyte}        { return KILOBYTE; }
{megabyte}        { return MEGABYTE; }
{gigabyte}        { return GIGABYTE; }

{loadavg1}        { return LOADAVG1; }
{loadavg5}        { return LOADAVG5; }
{loadavg15}       { return LOADAVG15; }

{greater}         { return GREATER; }
{less}            { return LESS; }
{equal}           { return EQUAL; }
{notequal}        { return NOTEQUAL; }

depend(s)?[ \t]+(on[ \t]*)?  {
                    BEGIN(DEPEND_COND);
                    return DEPENDS;
                  } 

check[ \t]+(process[ \t])? {
                    BEGIN(SERVICE_COND);
                    return CHECKPROC;
                  }

check[ \t]+device {
                    BEGIN(SERVICE_COND);
                    return CHECKDEV;
                  }

check[ \t]+file   {
                    BEGIN(SERVICE_COND);
                    return CHECKFILE;
                  }

check[ \t]+directory {
                    BEGIN(SERVICE_COND);
                    return CHECKDIR;
                  }

check[ \t]+host   {
                    BEGIN(SERVICE_COND);
                    return CHECKHOST;
                  }

{number}          {
                    yylval.number= atoi(yytext);
                    save_arg(); return NUMBER;
                  }

{real}            {
                    yylval.real= atof(yytext);
                    save_arg(); return REAL;
                  }

{real}{percent}   {
                    if(sscanf(yytext, "%f%*s", &yylval.real) != 1) {
                      yyerror("internal lexer error, parsing '%s'");
                      exit(1);
                    }
                    save_arg(); return PERCENT;
                  }

[a-zA-Z0-9]{str}  {
                    yylval.string= xstrdup(yytext);
                    save_arg(); return STRING;
                  }

\"[/][^\"\n]*\"   {
                    yylval.string= handle_quoted_string(yytext);
                    save_arg(); return PATH;
                  }

\'[/][^\'\n]*\'   {
                    yylval.string= handle_quoted_string(yytext);
                    save_arg(); return PATH;
                  }

\"[^\"]*\"        {
                    steplinenobycr(yytext);
                    yylval.string= handle_quoted_string(yytext);
                    save_arg(); return STRING;
                  }

\'[^\']*\'        {
                    steplinenobycr(yytext);
                    yylval.string= handle_quoted_string(yytext);
                    save_arg(); return STRING;
                  }

{str}[@]{str}     {
                    yylval.string= xstrdup(yytext);
                    save_arg(); return MAILADDR;
                  }

[/]{str}          {
                     yylval.string= xstrdup(yytext);
                     save_arg(); return PATH;
                  }

"/"               {
                     yylval.string= xstrdup(yytext);
                     save_arg(); return PATH;
                  }

"from:"[ \t]*{str}[@]{str} {
                      char *p= yytext+strlen("from:");
                      yylval.string = trim(xstrdup(p));
                      save_arg(); return MAILFROM;
                  }
                     
"subject:"[^}\n]* {
                      char *p= yytext+strlen("subject:");
                      yylval.string = trim(xstrdup(p));
                      save_arg(); return MAILSUBJECT;
                  }

"message:"[^}]*   {
                      char *p= yytext+strlen("message:");
                      steplinenobycr(yytext);
                      yylval.string = trim(xstrdup(p));
                      save_arg(); return MAILBODY;
                  }

{hostname}        {
                      yylval.string = xstrdup(yytext);
                      save_arg(); return STRING;
                  }

[\"\']            {
                      yyerror("unbalanced quotes");
                  }

<SERVICE_COND>{

  {ws}            ;

  [\n]            {
                    lineno++;
                  }

  {str}           {
                    yylval.string= xstrdup(yytext);
                    BEGIN(INITIAL);
                    save_arg(); return SERVICENAME;
                  }

  \"{str}\"       {
                    yylval.string= handle_quoted_string(yytext);
                    BEGIN(INITIAL);
                    save_arg(); return SERVICENAME;
                  }

  \'{str}\'       {
                    yylval.string= handle_quoted_string(yytext);
                    BEGIN(INITIAL);
                    save_arg(); return SERVICENAME;
                  }

  [\"\']          {
                      yyerror("unbalanced quotes");
                  }

}

<DEPEND_COND>{

  {wws}           ;

  {wws}?[\n]{wws}? {
                    lineno++;
                  }

  {str}           {
                    yylval.string= xstrdup(yytext);
                    save_arg(); return SERVICENAME;
                  }

  [ \n\t]+[^,]    {
                    steplinenobycr(yytext);
                    unput(yytext[strlen(yytext)-1]);
                    BEGIN(INITIAL);
                  }

}

<ARGUMENT_COND>{

  {ws}            ;

  [\n]            {
                    lineno++;
                  }

  \"              {
                      BEGIN(INITIAL);
                  }

  \'[^\']*\'      {
                      steplinenobycr(yytext);
                      yylval.string= handle_quoted_string(yytext);
                      save_arg(); return STRING;
                  }

  \'              {
                      yyerror("unbalanced quotes");
                  }

  [^ \t\n\"]+     {
                      yylval.string= xstrdup(yytext);
                      save_arg(); return STRING;
                  }

}

<INITIAL,ARGUMENT_COND,SERVICE_COND,DEPEND_COND>. {
                      return yytext[0];
                  }  


<INCLUDE>[ \t]*      /* eat the whitespace */

<INCLUDE>\"[^\"\n]+\" { /* got the include file name with double quotes */
                     char *temp=xstrdup(yytext);
                     trim_quotes(temp);    
                     include_file(temp);
                     free(temp);
                     BEGIN(INITIAL);
                   }

<INCLUDE>\'[^\'\n]+\' { /* got the include file name with single quotes*/
                     char *temp=xstrdup(yytext);
                     trim_quotes(temp);    
                     include_file(temp);
                     free(temp);
                     BEGIN(INITIAL);
                   }

<INCLUDE>[^ \t\n]+ { /* got the include file name without quotes*/
                     char *temp=xstrdup(yytext);
                     include_file(temp);
                     free(temp);
                     BEGIN(INITIAL);
                   }


<<EOF>>           {
                       
                       if ( !pop_buffer_state() )
                       {
                         yyterminate();
                       }
                  }

%%

/*
 * Do lineno++ for every occurrence of '\n' in a string.  This is
 * necessary whenever a yytext has an unknown number of CRs.
 */

static void steplinenobycr(char *string) {

  char *pos= string;

  while(*pos)
    if('\n'==*pos++) {
      lineno++;
    }

}


static char *handle_quoted_string(char *string) {

  char *buf= xstrdup(string);

  trim_quotes(buf);
  handle_string_escapes(buf);

  return buf;

}


static void include_file(char *pattern) {
   
  glob_t globbuf;
  int i;

  if ( glob(pattern,  GLOB_MARK, NULL, &globbuf) != 0 ) {

    yyerror("include files not found");
    
    return;

  } 

  for(i=0; i<globbuf.gl_pathc; i++) {
  
    FILE *yyin;
    int filename_length = strlen(globbuf.gl_pathv[i]);

    /* check whenever we have caught a directory
     * or file backup copy */

    if ((filename_length == 0) ||
        (globbuf.gl_pathv[i][filename_length-1] == '~' ) ||
        (globbuf.gl_pathv[i][filename_length-1] == '/' )) {

      continue;

    }

    yyin=fopen( globbuf.gl_pathv[i], "r" );
    
    if ( ! yyin ) {
      
      yyerror( "failed to include file" );
      
    } else {

      push_buffer_state(yy_create_buffer( yyin, YY_BUF_SIZE ),
                        globbuf.gl_pathv[i]);

    }
  }

  globfree(&globbuf);
}

static void push_buffer_state(YY_BUFFER_STATE buffer, char *filename) {

  if ( buffer_stack_ptr >= MAX_STACK_DEPTH )
  {

    yyerror( "include files are nested too deeply" );
    exit( 1 );
    
  }

  buffer_stack[buffer_stack_ptr].lineno=lineno;
  buffer_stack[buffer_stack_ptr].currentfile=currentfile;
  buffer_stack[buffer_stack_ptr].buffer=YY_CURRENT_BUFFER;

  buffer_stack_ptr++;

  lineno=1;
  currentfile=xstrdup(filename);
      
  yy_switch_to_buffer(buffer);

  BEGIN(INITIAL);
  
}

static int pop_buffer_state(void) {

  if ( --buffer_stack_ptr < 0 ) {

    return 0;

  } else {

    lineno=buffer_stack[buffer_stack_ptr].lineno;

    FREE(currentfile);
    currentfile=buffer_stack[buffer_stack_ptr].currentfile;

    yy_delete_buffer( YY_CURRENT_BUFFER );
    yy_switch_to_buffer( buffer_stack[buffer_stack_ptr].buffer );
    
  }

  return 1;
 
}

static void save_arg(void) {
  
  arglineno=lineno;
  argcurrentfile=currentfile;
  FREE(argyytext);
  argyytext=xstrdup(yytext);
  
}
