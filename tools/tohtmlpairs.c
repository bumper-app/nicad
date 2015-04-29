#include <LINUX/cinterface>
extern TLint4	TL_TLI_TLIARC;

extern void TL_TLI_TLIFA ();

extern void TL_TLX_TLXGE ();

extern void TL_TLX_TLXDT ();

extern void TL_TLX_TLXTM ();

extern void TL_TLX_TLXCL ();

extern void TL_TLX_TLXSC ();

extern void time ();

extern void TL_TLX_TLXSYS ();

extern TLint4 getpid ();

extern void TL_TLI_TLIFS ();

extern void TL_TLK_TLKUEXIT ();
extern TLnat4	TL_TLK_TLKTIME;
extern TLnat4	TL_TLK_TLKEPOCH;

extern void TL_TLK_TLKUDMPA ();

extern void TL_TLK_TLKCINI ();
extern TLboolean	TL_TLK_TLKCLKON;
extern TLnat4	TL_TLK_TLKHZ;
extern TLnat4	TL_TLK_TLKCRESO;
extern TLnat4	TL_TLK_TLKTIME;
extern TLnat4	TL_TLK_TLKEPOCH;

extern void TL_TLK_TLKPSID ();

extern TLnat4 TL_TLK_TLKPGID ();

extern void TL_TLK_TLKRSETP ();

static void encode (line, __x45)
TLstring	line;
TLstring	__x45;
{
    TLstring	eline;
    TLSTRASS(255, eline, "");
    {
	register TLint4	i;
	TLint4	__x80;
	__x80 = TL_TLS_TLSLEN(line);
	i = 1;
	if (i <= __x80) {
	    for(;;) {
		{
		    TLchar	__x81[2];
		    TL_TLS_TLSBX(__x81, (TLint4) i, line);
		    switch (((TLnat4) TLCVTTOCHR(__x81))) {
			case 60:
			    {
				TLSTRCATASS(eline, "&lt;", 255);
			    }
			    break;
			case 62:
			    {
				TLSTRCATASS(eline, "&gt;", 255);
			    }
			    break;
			case 38:
			    {
				TLSTRCATASS(eline, "&amp;", 255);
			    }
			    break;
			default :
			    {
				{
				    TLchar	__x82[2];
				    TL_TLS_TLSBX(__x82, (TLint4) i, line);
				    TLSTRCATASS(eline, __x82, 255);
				};
			    }
			    break;
		    };
		};
		if (i == __x80) break;
		i++;
	    }
	};
    };
    {
	TLSTRASS(255, __x45, eline);
	return;
    };
    /* NOTREACHED */
}
static TLstring	ccfile;
static TLint4	ccf;
static TLstring	rccfile;
static TLint4	rccf;
static TLstring	line;
static TLstring	systemname;
static TLstring	granularity;
static TLstring	threshold;
static TLstring	minlines;
static TLstring	maxlines;
static TLstring	npcs;
static TLstring	npairs;
static TLstring	ncompares;
static TLstring	cputime;
static TLint4	cputotalms;
static TLint4	cpums;
static TLint4	cpusec;
static TLint4	cpumin;
void TProg () {
    {
	TLstring	__x84;
	{
	    TLstring	__x83;
	    TL_TLI_TLIFA((TLint4) 1, __x83);
	    if ((strcmp(__x83, "") == 0) || ((TL_TLI_TLIFA((TLint4) 2, __x84), strcmp(__x84, "") == 0))) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "Usage:  tothml.x system_functions-clonepairs-withsource.xml system_functions-clonepairs-withsource.html", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	};
    };
    {
	TLstring	__x85;
	TL_TLI_TLIFA((TLint4) 1, __x85);
	TLSTRASS(255, ccfile, __x85);
    };
    TL_TLI_TLIOF ((TLnat2) 2, ccfile, &ccf);
    if (ccf == 0) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: can\'t open sourced clone classes input file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    };
    {
	TLstring	__x86;
	TL_TLI_TLIFA((TLint4) 2, __x86);
	TLSTRASS(255, rccfile, __x86);
    };
    TL_TLI_TLIOF ((TLnat2) 4, rccfile, &rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<html>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<head>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<style type=\"text/css\">", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "body {font-family:Arial}", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "table {background-color:white; border:0px solid white; width:95%; margin-left:auto; margin-right: auto}", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "td {background-color:#b0c4de; padding:16px; border:4px solid white}", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "pre {background-color:white; padding:4px}", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</style>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<title>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "NiCad Clone Report", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</title>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</head>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<body>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<h2>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "NiCad Clone Report", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</h2>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
    if (strcmp(line, "<clones>") != 0) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: malformed sourced clone classes input file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    };
    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
    {
	TLstring	__x87;
	TL_TLS_TLSBXX(__x87, (TLint4) (TL_TLS_TLSIND(line, "\" granularity=") - 1), (TLint4) (TL_TLS_TLSIND(line, "system=\"") + TL_TLS_TLSLEN("system=\"")), line);
	TLSTRASS(255, systemname, __x87);
    };
    {
	TLstring	__x88;
	TL_TLS_TLSBXX(__x88, (TLint4) (TL_TLS_TLSIND(line, "\" threshold=") - 1), (TLint4) (TL_TLS_TLSIND(line, "granularity=\"") + TL_TLS_TLSLEN("granularity=\"")), line);
	TLSTRASS(255, granularity, __x88);
    };
    {
	TLstring	__x89;
	TL_TLS_TLSBXX(__x89, (TLint4) (TL_TLS_TLSIND(line, "\" minlines=") - 1), (TLint4) (TL_TLS_TLSIND(line, "threshold=\"") + TL_TLS_TLSLEN("threshold=\"")), line);
	TLSTRASS(255, threshold, __x89);
    };
    {
	TLstring	__x90;
	TL_TLS_TLSBXX(__x90, (TLint4) (TL_TLS_TLSIND(line, "\" maxlines=") - 1), (TLint4) (TL_TLS_TLSIND(line, "minlines=\"") + TL_TLS_TLSLEN("minlines=\"")), line);
	TLSTRASS(255, minlines, __x90);
    };
    {
	TLstring	__x91;
	TL_TLS_TLSBXX(__x91, (TLint4) (TL_TLS_TLSIND(line, "\"/>") - 1), (TLint4) (TL_TLS_TLSIND(line, "maxlines=\"") + TL_TLS_TLSLEN("maxlines=\"")), line);
	TLSTRASS(255, maxlines, __x91);
    };
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "System: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, systemname, (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>Granularity: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, granularity, (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>Max difference threshold: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, threshold, (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>Clone size: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, minlines, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, " - ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, maxlines, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, " lines", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
    {
	TLstring	__x92;
	TL_TLS_TLSBXX(__x92, (TLint4) (TL_TLS_TLSIND(line, "\" npairs=") - 1), (TLint4) (TL_TLS_TLSIND(line, "npcs=\"") + TL_TLS_TLSLEN("npcs=\"")), line);
	TLSTRASS(255, npcs, __x92);
    };
    {
	TLstring	__x93;
	TL_TLS_TLSBXX(__x93, (TLint4) (TL_TLS_TLSIND(line, "\"/>") - 1), (TLint4) (TL_TLS_TLSIND(line, "npairs=\"") + TL_TLS_TLSLEN("npairs=\"")), line);
	TLSTRASS(255, npairs, __x93);
    };
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>Total ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, granularity, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, ": ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, npcs, (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>Clone pairs found: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, npairs, (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
    {
	TLstring	__x94;
	TL_TLS_TLSBXX(__x94, (TLint4) (TL_TLS_TLSIND(line, "\" cputime=") - 1), (TLint4) (TL_TLS_TLSIND(line, "ncompares=\"") + TL_TLS_TLSLEN("ncompares=\"")), line);
	TLSTRASS(255, ncompares, __x94);
    };
    {
	TLstring	__x95;
	TL_TLS_TLSBXX(__x95, (TLint4) (TL_TLS_TLSIND(line, "\"/>") - 1), (TLint4) (TL_TLS_TLSIND(line, "cputime=\"") + TL_TLS_TLSLEN("cputime=\"")), line);
	TLSTRASS(255, cputime, __x95);
    };
    cputotalms = TL_TLS_TLSVSI(cputime, (TLint4) 10) / 1000;
    cpums = cputotalms % 1000;
    cpusec = (cpums / 1000) % 60;
    cpumin = (cpums / 1000) / 60;
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>LCS compares: ", (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, ncompares, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, " &nbsp;&nbsp;&nbsp; CPU time: ", (TLint2) rccf);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) cpumin, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, " min ", (TLint2) rccf);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) cpusec, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) rccf);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) cpums, (TLint2) rccf);
    TL_TLI_TLIPS ((TLint4) 0, " sec", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "<br>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    for(;;) {
	if (TL_TLI_TLIEOF((TLint4) ccf)) {
	    break;
	};
	TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
	TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
	if (TL_TLS_TLSIND(line, "<classinfo ") == 1) {
	    TLstring	nclasses;
	    {
		TLstring	__x96;
		TL_TLS_TLSBXX(__x96, (TLint4) (TL_TLS_TLSIND(line, "\"/>") - 1), (TLint4) (TL_TLS_TLSIND(line, "nclasses=\"") + TL_TLS_TLSLEN("nclasses=\"")), line);
		TLSTRASS(255, nclasses, __x96);
	    };
	    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "<br>Number of  classes: ", (TLint2) rccf);
	    TL_TLI_TLIPS ((TLint4) 0, nclasses, (TLint2) rccf);
	    TL_TLI_TLIPK ((TLint2) rccf);
	    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "<br>", (TLint2) rccf);
	    TL_TLI_TLIPK ((TLint2) rccf);
	} else {
	    if ((TL_TLS_TLSIND(line, "<clone ") == 1) || (TL_TLS_TLSIND(line, "<class ") == 1)) {
		if (TL_TLS_TLSIND(line, "<clone ") == 1) {
		    TLstring	pairlines;
		    TLstring	similarity;
		    {
			TLstring	__x97;
			TL_TLS_TLSBXX(__x97, (TLint4) (TL_TLS_TLSIND(line, "\" similarity=") - 1), (TLint4) (TL_TLS_TLSIND(line, "nlines=\"") + TL_TLS_TLSLEN("nlines=\"")), line);
			TLSTRASS(255, pairlines, __x97);
		    };
		    {
			TLstring	__x98;
			TL_TLS_TLSBXX(__x98, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "similarity=\"") + TL_TLS_TLSLEN("similarity=\"")), line);
			TLSTRASS(255, similarity, __x98);
		    };
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "<h3>", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "Clone pair, nominal size ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, pairlines, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, " lines, similarity ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, similarity, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, "%", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "</h3>", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "<table cellpadding=12 border=2 frame=\"box\" width=\"90%\">", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		} else {
		    TLstring	classid;
		    TLstring	nclassclones;
		    TLstring	classlines;
		    TLstring	similarity;
		    {
			TLstring	__x99;
			TL_TLS_TLSBXX(__x99, (TLint4) (TL_TLS_TLSIND(line, "\" nclones=") - 1), (TLint4) (TL_TLS_TLSIND(line, "classid=\"") + TL_TLS_TLSLEN("classid=\"")), line);
			TLSTRASS(255, classid, __x99);
		    };
		    {
			TLstring	__x100;
			TL_TLS_TLSBXX(__x100, (TLint4) (TL_TLS_TLSIND(line, "\" nlines=") - 1), (TLint4) (TL_TLS_TLSIND(line, "nclones=\"") + TL_TLS_TLSLEN("nclones=\"")), line);
			TLSTRASS(255, nclassclones, __x100);
		    };
		    {
			TLstring	__x101;
			TL_TLS_TLSBXX(__x101, (TLint4) (TL_TLS_TLSIND(line, "\" similarity=") - 1), (TLint4) (TL_TLS_TLSIND(line, "nlines=\"") + TL_TLS_TLSLEN("nlines=\"")), line);
			TLSTRASS(255, classlines, __x101);
		    };
		    {
			TLstring	__x102;
			TL_TLS_TLSBXX(__x102, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "similarity=\"") + TL_TLS_TLSLEN("similarity=\"")), line);
			TLSTRASS(255, similarity, __x102);
		    };
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "<h3>Clone class ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, classid, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, ", ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, nclassclones, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, " fragments, nominal size ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, classlines, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, " lines, similarity ", (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, similarity, (TLint2) rccf);
		    TL_TLI_TLIPS ((TLint4) 0, "%", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "</h3>", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "<table cellpadding=12 border=2 frame=\"box\" width=\"90%\">", (TLint2) rccf);
		    TL_TLI_TLIPK ((TLint2) rccf);
		};
		for(;;) {
		    if (TL_TLI_TLIEOF((TLint4) ccf)) {
			break;
		    };
		    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
		    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
		    if ((strcmp(line, "</clone>") == 0) || (strcmp(line, "</class>") == 0)) {
			break;
		    };
		    if (TL_TLS_TLSIND(line, "<source") == 1) {
			TLstring	srcfile;
			TLstring	startline;
			TLstring	endline;
			TLstring	pcid;
			{
			    TLstring	__x103;
			    TL_TLS_TLSBXX(__x103, (TLint4) (TL_TLS_TLSIND(line, "\" startline=") - 1), (TLint4) (TL_TLS_TLSIND(line, "file=\"") + TL_TLS_TLSLEN("file=\"")), line);
			    TLSTRASS(255, srcfile, __x103);
			};
			{
			    TLstring	__x104;
			    TL_TLS_TLSBXX(__x104, (TLint4) (TL_TLS_TLSIND(line, "\" endline=") - 1), (TLint4) (TL_TLS_TLSIND(line, "startline=\"") + TL_TLS_TLSLEN("startline=\"")), line);
			    TLSTRASS(255, startline, __x104);
			};
			{
			    TLstring	__x105;
			    TL_TLS_TLSBXX(__x105, (TLint4) (TL_TLS_TLSIND(line, "\" pcid=") - 1), (TLint4) (TL_TLS_TLSIND(line, "endline=\"") + TL_TLS_TLSLEN("endline=\"")), line);
			    TLSTRASS(255, endline, __x105);
			};
			{
			    TLstring	__x106;
			    TL_TLS_TLSBXX(__x106, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "pcid=\"") + TL_TLS_TLSLEN("pcid=\"")), line);
			    TLSTRASS(255, pcid, __x106);
			};
			TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "<tr><td>", (TLint2) rccf);
			TL_TLI_TLIPK ((TLint2) rccf);
			TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "Lines ", (TLint2) rccf);
			TL_TLI_TLIPS ((TLint4) 0, startline, (TLint2) rccf);
			TL_TLI_TLIPS ((TLint4) 0, " - ", (TLint2) rccf);
			TL_TLI_TLIPS ((TLint4) 0, endline, (TLint2) rccf);
			TL_TLI_TLIPS ((TLint4) 0, " of ", (TLint2) rccf);
			TL_TLI_TLIPS ((TLint4) 0, srcfile, (TLint2) rccf);
			TL_TLI_TLIPK ((TLint2) rccf);
			TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "<pre>", (TLint2) rccf);
			TL_TLI_TLIPK ((TLint2) rccf);
			for(;;) {
			    if (TL_TLI_TLIEOF((TLint4) ccf)) {
				break;
			    };
			    for(;;) {
				TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
				TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
				if (TL_TLI_TLIEOF((TLint4) ccf) || (TL_TLS_TLSLEN(line) < 255)) {
				    break;
				};
				{
				    TLchar	__x107[102];
				    TL_TLS_TLSBXX(__x107, (TLint4) 100, (TLint4) 1, line);
				    {
					TLstring	__x108;
					encode(__x107, __x108);
					TL_TLI_TLISS ((TLint4) ccf, (TLint2) 2);
					TL_TLI_TLIPS ((TLint4) 0, __x108, (TLint2) ccf);
				    };
				};
				{
				    TLstring	__x109;
				    TL_TLS_TLSBXS(__x109, (TLint4) 0, (TLint4) 100, line);
				    {
					TLstring	__x110;
					encode(__x109, __x110);
					TL_TLI_TLISS ((TLint4) ccf, (TLint2) 2);
					TL_TLI_TLIPS ((TLint4) 0, __x110, (TLint2) ccf);
				    };
				};
			    };
			    if (strcmp(line, "</source>") == 0) {
				break;
			    };
			    if (TL_TLS_TLSLEN(line) > 100) {
				{
				    TLchar	__x111[102];
				    TL_TLS_TLSBXX(__x111, (TLint4) 100, (TLint4) 1, line);
				    {
					TLstring	__x112;
					encode(__x111, __x112);
					TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
					TL_TLI_TLIPS ((TLint4) 0, __x112, (TLint2) rccf);
				    };
				};
				{
				    TLstring	__x113;
				    TL_TLS_TLSBXS(__x113, (TLint4) 0, (TLint4) 100, line);
				    {
					TLstring	__x114;
					encode(__x113, __x114);
					TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
					TL_TLI_TLIPS ((TLint4) 0, __x114, (TLint2) rccf);
					TL_TLI_TLIPK ((TLint2) rccf);
				    };
				};
			    } else {
				{
				    TLstring	__x115;
				    encode(line, __x115);
				    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
				    TL_TLI_TLIPS ((TLint4) 0, __x115, (TLint2) rccf);
				    TL_TLI_TLIPK ((TLint2) rccf);
				};
			    };
			};
			TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "</pre>", (TLint2) rccf);
			TL_TLI_TLIPK ((TLint2) rccf);
			TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "</td></tr>", (TLint2) rccf);
			TL_TLI_TLIPK ((TLint2) rccf);
		    };
		    if (strcmp(line, "</source>") != 0) {
			TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "*** Error: clone source file synchronization error", (TLint2) 0);
			TL_TLI_TLIPK ((TLint2) 0);
			TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
		    };
		};
		if ((strcmp(line, "</clone>") != 0) && (strcmp(line, "</class>") != 0)) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "*** Error: clone pair / class file synchronization error", (TLint2) 0);
		    TL_TLI_TLIPK ((TLint2) 0);
		    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
		};
		TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "</table>", (TLint2) rccf);
		TL_TLI_TLIPK ((TLint2) rccf);
	    };
	};
    };
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</body>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "</html>", (TLint2) rccf);
    TL_TLI_TLIPK ((TLint2) rccf);
    TL_TLI_TLICL ((TLint4) ccf);
    TL_TLI_TLICL ((TLint4) rccf);
}
