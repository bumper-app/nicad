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
static TLstring	ccfile;

static void useerr () {
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Usage :  splitclasses.x cloneclassfile.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "  e.g.:  splitclasses.x linux_functions-clones-0.3-classes-withsource.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "  (Output in folders linux_functions-clones-0.3-classes-withsource/*)", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
}
static TLint4	ccf;

extern TLint4 system ();
static TLstring	ccdir;
static TLint4	rc;
static TLstring	commandline;
void TProg () {
    {
	TLstring	__x60;
	TL_TLI_TLIFA((TLint4) 1, __x60);
	TLSTRASS(255, ccfile, __x60);
    };
    TL_TLI_TLIOF ((TLnat2) 2, ccfile, &ccf);
    {
	TLstring	__x61;
	if ((ccf == 0) || ((TL_TLS_TLSBXS(__x61, (TLint4) 0, (TLint4) (TL_TLS_TLSLEN(ccfile) - 3), ccfile), strcmp(__x61, ".xml") != 0))) {
	    useerr();
	};
    };
    {
	TLstring	__x62;
	TL_TLS_TLSBXS(__x62, (TLint4) -4, (TLint4) 1, ccfile);
	TLSTRASS(255, ccdir, __x62);
    };
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Making output folder ", (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, ccdir, (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    rc = 0;
    {
	TLstring	__x64;
	TL_TLS_TLSCAT("/bin/rm -rf ", ccdir, __x64);
	{
	    TLstring	__x63;
	    TL_TLS_TLSCAT(__x64, " > /dev/null 2>&1", __x63);
	    TLSTRASS(255, commandline, __x63);
	};
    };
    rc = system(commandline);
    {
	TLstring	__x65;
	TL_TLS_TLSCAT("/bin/mkdir ", ccdir, __x65);
	TLSTRASS(255, commandline, __x65);
    };
    rc = system(commandline);
    if (rc != 0) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error, cannot create output directory", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 99, (char *) 0, 0);
    };
    for(;;) {
	TLstring	line;
	if (TL_TLI_TLIEOF((TLint4) ccf)) {
	    break;
	};
	TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
	TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
	if (TL_TLS_TLSIND(line, "<class ") == 1) {
	    TLstring	classid;
	    TLstring	classdir;
	    {
		TLstring	__x66;
		TL_TLS_TLSBXX(__x66, (TLint4) (TL_TLS_TLSIND(line, " nclones=\"") - 2), (TLint4) (TL_TLS_TLSIND(line, "classid=\"") + TL_TLS_TLSLEN("classid=\"")), line);
		TLSTRASS(255, classid, __x66);
	    };
	    {
		TLstring	__x68;
		TL_TLS_TLSCAT(ccdir, "/", __x68);
		{
		    TLstring	__x67;
		    TL_TLS_TLSCAT(__x68, classid, __x67);
		    TLSTRASS(255, classdir, __x67);
		};
	    };
	    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "Making clone class folder ", (TLint2) 0);
	    TL_TLI_TLIPS ((TLint4) 0, classdir, (TLint2) 0);
	    TL_TLI_TLIPK ((TLint2) 0);
	    {
		TLstring	__x69;
		TL_TLS_TLSCAT("/bin/mkdir ", classdir, __x69);
		TLSTRASS(255, commandline, __x69);
	    };
	    rc = system(commandline);
	    if (rc != 0) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error, cannot create class directory", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 99, (char *) 0, 0);
	    };
	    for(;;) {
		if (TL_TLI_TLIEOF((TLint4) ccf)) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "*** Error, synchronization error on class file", (TLint2) 0);
		    TL_TLI_TLIPK ((TLint2) 0);
		    TL_TLE_TLEQUIT ((TLint4) 99, (char *) 0, 0);
		};
		TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
		if (TL_TLS_TLSIND(line, "</class>") == 1) {
		    break;
		};
		if (TL_TLS_TLSIND(line, "<source ") == 1) {
		    TLstring	pcid;
		    TLstring	pcfile;
		    TLint4	pcf;
		    {
			TLstring	__x70;
			TL_TLS_TLSBXX(__x70, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "pcid=\"") + TL_TLS_TLSLEN("pcid=\"")), line);
			TLSTRASS(255, pcid, __x70);
		    };
		    {
			TLstring	__x72;
			TL_TLS_TLSCAT(classdir, "/", __x72);
			{
			    TLstring	__x71;
			    TL_TLS_TLSCAT(__x72, pcid, __x71);
			    TLSTRASS(255, pcfile, __x71);
			};
		    };
		    TL_TLI_TLIOF ((TLnat2) 4, pcfile, &pcf);
		    if (pcf == 0) {
			TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "*** Error, cannot create pc file in class directory", (TLint2) 0);
			TL_TLI_TLIPK ((TLint2) 0);
			TL_TLE_TLEQUIT ((TLint4) 99, (char *) 0, 0);
		    };
		    TL_TLI_TLISS ((TLint4) pcf, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) pcf);
		    TL_TLI_TLIPK ((TLint2) pcf);
		    for(;;) {
			TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
			TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
			if (TL_TLS_TLSIND(line, "</source>") == 1) {
			    break;
			};
			TL_TLI_TLISS ((TLint4) pcf, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) pcf);
			TL_TLI_TLIPK ((TLint2) pcf);
		    };
		    TL_TLI_TLICL ((TLint4) pcf);
		};
	    };
	};
    };
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Done", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
}
