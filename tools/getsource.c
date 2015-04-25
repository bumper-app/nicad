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

static void clean (line, __x45)
TLstring	line;
TLstring	__x45;
{
    TLstring	eline;
    TLSTRASS(255, eline, line);
    for(;;) {
	TLint4	idindex;
	idindex = TL_TLS_TLSIND(eline, " INDENT");
	if (idindex == 0) {
	    idindex = TL_TLS_TLSIND(eline, "DEDENT ");
	};
	if (idindex == 0) {
	    break;
	};
	{
	    TLstring	__x71;
	    TL_TLS_TLSBXS(__x71, (TLint4) 0, (TLint4) (idindex + TL_TLS_TLSLEN(" INDENT")), eline);
	    {
		TLstring	__x70;
		TL_TLS_TLSBXX(__x70, (TLint4) (idindex - 1), (TLint4) 1, eline);
		{
		    TLstring	__x69;
		    TL_TLS_TLSCAT(__x70, __x71, __x69);
		    TLSTRASS(255, eline, __x69);
		};
	    };
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
void TProg () {
    {
	TLstring	__x73;
	{
	    TLstring	__x72;
	    TL_TLI_TLIFA((TLint4) 1, __x72);
	    if ((strcmp(__x72, "") == 0) || ((TL_TLI_TLIFA((TLint4) 2, __x73), strcmp(__x73, "") == 0))) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "Usage:  getsource.x system_functions-clones.xml system_functions-clones-sourced.xml", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	};
    };
    {
	TLstring	__x74;
	TL_TLI_TLIFA((TLint4) 1, __x74);
	TLSTRASS(255, ccfile, __x74);
    };
    TL_TLI_TLIOF ((TLnat2) 2, ccfile, &ccf);
    if (ccf == 0) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: can\'t open clone classes file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    };
    {
	TLstring	__x75;
	TL_TLI_TLIFA((TLint4) 2, __x75);
	TLSTRASS(255, rccfile, __x75);
    };
    TL_TLI_TLIOF ((TLnat2) 4, rccfile, &rccf);
    for(;;) {
	TLstring	line;
	if (TL_TLI_TLIEOF((TLint4) ccf)) {
	    break;
	};
	TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
	TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
	if (TL_TLS_TLSIND(line, "<source") == 1) {
	    TLstring	sourceheader;
	    TLint4	sfindex;
	    TLint4	sfend;
	    TLstring	srcfile;
	    TLint4	sf;
	    TLint4	slindex;
	    TLint4	slend;
	    TLint4	startline;
	    TLint4	elindex;
	    TLint4	elend;
	    TLint4	endline;
	    {
		TLstring	__x76;
		TL_TLS_TLSBXX(__x76, (TLint4) (TL_TLS_TLSIND(line, "</source>") - 1), (TLint4) 1, line);
		TLSTRASS(255, sourceheader, __x76);
	    };
	    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, sourceheader, (TLint2) rccf);
	    TL_TLI_TLIPK ((TLint2) rccf);
	    sfindex = (TL_TLS_TLSIND(sourceheader, "file=") + TL_TLS_TLSLEN("file=")) + 1;
	    sfend = TL_TLS_TLSIND(sourceheader, " startline=") - 2;
	    {
		TLstring	__x77;
		TL_TLS_TLSBXX(__x77, (TLint4) sfend, (TLint4) sfindex, sourceheader);
		TLSTRASS(255, srcfile, __x77);
	    };
	    TL_TLI_TLIOF ((TLnat2) 2, srcfile, &sf);
	    if (sf == 0) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: can\'t open source file ", (TLint2) 0);
		TL_TLI_TLIPS ((TLint4) 0, srcfile, (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    slindex = (TL_TLS_TLSIND(sourceheader, "startline=") + TL_TLS_TLSLEN("startline=")) + 1;
	    slend = TL_TLS_TLSIND(sourceheader, " endline=") - 2;
	    {
		TLstring	__x78;
		TL_TLS_TLSBXX(__x78, (TLint4) slend, (TLint4) slindex, sourceheader);
		startline = TL_TLS_TLSVSI(__x78, (TLint4) 10);
	    };
	    {
		register TLint4	i;
		TLint4	__x79;
		__x79 = startline - 1;
		i = 1;
		if (i <= __x79) {
		    for(;;) {
			for(;;) {
			    TL_TLI_TLISS ((TLint4) sf, (TLint2) 1);
			    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) sf);
			    if (TL_TLI_TLIEOF((TLint4) sf) || (TL_TLS_TLSLEN(line) < 255)) {
				break;
			    };
			};
			if (i == __x79) break;
			i++;
		    }
		};
	    };
	    elindex = (TL_TLS_TLSIND(sourceheader, "endline=") + TL_TLS_TLSLEN("endline=")) + 1;
	    elend = TL_TLS_TLSIND(sourceheader, " pcid=") - 2;
	    {
		TLstring	__x80;
		TL_TLS_TLSBXX(__x80, (TLint4) elend, (TLint4) elindex, sourceheader);
		endline = TL_TLS_TLSVSI(__x80, (TLint4) 10);
	    };
	    {
		register TLint4	i;
		TLint4	__x81;
		__x81 = endline;
		i = startline;
		if (i <= __x81) {
		    for(;;) {
			for(;;) {
			    TL_TLI_TLISS ((TLint4) sf, (TLint2) 1);
			    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) sf);
			    if (TL_TLI_TLIEOF((TLint4) sf) || (TL_TLS_TLSLEN(line) < 255)) {
				break;
			    };
			    {
				TLstring	__x82;
				clean(line, __x82);
				TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
				TL_TLI_TLIPS ((TLint4) 0, __x82, (TLint2) rccf);
			    };
			};
			{
			    TLstring	__x83;
			    clean(line, __x83);
			    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
			    TL_TLI_TLIPS ((TLint4) 0, __x83, (TLint2) rccf);
			    TL_TLI_TLIPK ((TLint2) rccf);
			};
			if (i == __x81) break;
			i++;
		    }
		};
	    };
	    TL_TLI_TLICL ((TLint4) sf);
	    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "</source>", (TLint2) rccf);
	    TL_TLI_TLIPK ((TLint2) rccf);
	} else {
	    TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) rccf);
	    TL_TLI_TLIPK ((TLint2) rccf);
	};
    };
    TL_TLI_TLICL ((TLint4) rccf);
}
