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
static TLstring	command;
static TLint4	unique;
static TLstring	tempinfile;
static TLstring	tempoutfile;
static TLstring	line;
void TProg () {
    {
	TLstring	__x59;
	{
	    TLstring	__x58;
	    TL_TLI_TLIFA((TLint4) 1, __x58);
	    if ((strcmp(__x58, "") == 0) || ((TL_TLI_TLIFA((TLint4) 2, __x59), strcmp(__x59, "") != 0))) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "Usage:  streamprocess.x normalizing_command < system_functions.xml > system_functions-normalized.xml", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	};
    };
    {
	TLstring	__x60;
	TL_TLI_TLIFA((TLint4) 1, __x60);
	TLSTRASS(255, command, __x60);
    };
    unique = getpid();
    {
	TLstring	__x63;
	TL_TLS_TLSVIS((TLint4) unique, (TLint4) 1, (TLint4) 10, __x63);
	{
	    TLstring	__x62;
	    TL_TLS_TLSCAT("/tmp/nicad", __x63, __x62);
	    {
		TLstring	__x61;
		TL_TLS_TLSCAT(__x62, ".in", __x61);
		TLSTRASS(255, tempinfile, __x61);
	    };
	};
    };
    {
	TLstring	__x66;
	TL_TLS_TLSVIS((TLint4) unique, (TLint4) 1, (TLint4) 10, __x66);
	{
	    TLstring	__x65;
	    TL_TLS_TLSCAT("/tmp/nicad", __x66, __x65);
	    {
		TLstring	__x64;
		TL_TLS_TLSCAT(__x65, ".out", __x64);
		TLSTRASS(255, tempoutfile, __x64);
	    };
	};
    };
    TL_TLI_TLISSI ();
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) -2);
    for(;;) {
	TLint4	tf;

	extern TLint4 system ();
	TLint4	rc;
	TLstring	commandline;
	if (TL_TLI_TLIEOF((TLint4) -2)) {
	    break;
	};
	if (TL_TLS_TLSIND(line, "<source") != 1) {
	    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on potential clones file", (TLint2) 0);
	    TL_TLI_TLIPK ((TLint2) 0);
	    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	};
	TL_TLI_TLIOF ((TLnat2) 4, tempinfile, &tf);
	if (tf == 0) {
	    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "*** Error: can\'t create temp file ", (TLint2) 0);
	    TL_TLI_TLIPK ((TLint2) 0);
	    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	};
	{
	    register TLint4	i;
	    for (i = 1; i <= 100; i++) {
		TL_TLI_TLISS ((TLint4) tf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) tf);
		TL_TLI_TLIPK ((TLint2) tf);
		for(;;) {
		    for(;;) {
			TL_TLI_TLISSI ();
			TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) -2);
			if (TL_TLI_TLIEOF((TLint4) -2) || (TL_TLS_TLSLEN(line) < 255)) {
			    break;
			};
			TL_TLI_TLISS ((TLint4) tf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) tf);
		    };
		    if (TL_TLI_TLIEOF((TLint4) -2) || (TL_TLS_TLSIND(line, "</source>") == 1)) {
			break;
		    };
		    TL_TLI_TLISS ((TLint4) tf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) tf);
		    TL_TLI_TLIPK ((TLint2) tf);
		};
		TL_TLI_TLISS ((TLint4) tf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) tf);
		TL_TLI_TLIPK ((TLint2) tf);
		if (TL_TLI_TLIEOF((TLint4) -2)) {
		    break;
		};
		if (TL_TLS_TLSIND(line, "</source>") != 1) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on potential clones file", (TLint2) 0);
		    TL_TLI_TLIPK ((TLint2) 0);
		    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
		};
		TL_TLI_TLISSI ();
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) -2);
	    };
	};
	TL_TLI_TLICL ((TLint4) tf);
	rc = 0;
	{
	    TLstring	__x68;
	    TL_TLS_TLSCAT(command, " < ", __x68);
	    {
		TLstring	__x67;
		TL_TLS_TLSCAT(__x68, tempinfile, __x67);
		TLSTRASS(255, commandline, __x67);
	    };
	};
	rc = system(commandline);
	if (rc != 0) {
	    {
		TLstring	__x69;
		TL_TLS_TLSCAT("*** Error: command failed: ", commandline, __x69);
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, __x69, (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
	    };
	    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	};
    };
}
