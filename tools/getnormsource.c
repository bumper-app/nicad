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
static TLstring	pcfile;
static TLstring	ccfile;
static TLstring	rccfile;
static TLint4	minclonelines;
static TLint4	maxclonelines;

static void useerr () {
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Usage :  getnormsource.x pcfile.xml inclonesfile.xml outclonesfile.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "  e.g.:  getnormsource.x linux_functions.xml linux_functions-clones.xml linux_functions-clones-withnormsource.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
}
typedef	TLchar	linetable___x128[750000001];
static linetable___x128	linetable_lineText;
static TLint4	linetable_lineTextSize;
typedef	TLint4	linetable___x131[15000000];
static linetable___x131	linetable_lineTable;
static TLint4	linetable_lineTableSize;
typedef	TLnat4	linetable_LN;

static TLnat4 linetable_hash (s)
TLstring	s;
{
    typedef	TLnat1	__x135[256];
    typedef	__x135	nat256;
    register TLnat4	h;
    register TLnat4	j;
    h = TL_TLS_TLSLEN(s);
    j = h - 1;
    if (h > 0) {
	{
	    register TLint4	i;
	    TLint4	__x137;
	    __x137 = h >> 1;
	    i = 0;
	    if (i <= __x137) {
		for(;;) {
		    h += ((TLint4) h << 1) + ((* (nat256 *) s)[i]);
		    h += ((TLint4) h << 1) + ((* (nat256 *) s)[j]);
		    j -= 1;
		    if (i == __x137) break;
		    i++;
		}
	    };
	};
    };
    return (h);
    /* NOTREACHED */
}
static TLint4	linetable_secondaryHash;
typedef	TLint4	linetable___x142[10];
static linetable___x142	linetable_primes = 
    {1021, 2027, 4091, 8191, 16381, 32003, 65003, 131009, 262007, 524047};
typedef	TLboolean	linetable___x146[256];
static linetable___x146	linetable_spaceP;

static linetable_LN linetable_install (rawline)
TLstring	rawline;
{
    TLstring	line;
    TLint4	rawlinelength;
    TLint4	first;
    TLint4	last;
    TLnat4	lineIndex;
    TLnat4	startIndex;
    TLint4	linelength;
    TLSTRASS(255, line, rawline);
    rawlinelength = TL_TLS_TLSLEN(rawline);
    first = 1;
    for(;;) {
	{
	    TLchar	__x149[2];
	    if ((first > rawlinelength) || ((TL_TLS_TLSBX(__x149, (TLint4) first, line), !(linetable_spaceP[((TLnat4) TLCVTTOCHR(__x149))])))) {
		break;
	    };
	};
	first += 1;
    };
    last = rawlinelength;
    for(;;) {
	{
	    TLchar	__x150[2];
	    if ((last < 1) || ((TL_TLS_TLSBX(__x150, (TLint4) last, line), !(linetable_spaceP[((TLnat4) TLCVTTOCHR(__x150))])))) {
		break;
	    };
	};
	last -= 1;
    };
    {
	TLstring	__x151;
	TL_TLS_TLSBXX(__x151, (TLint4) last, (TLint4) first, line);
	TLSTRASS(255, line, __x151);
    };
    lineIndex = linetable_hash(line);
    if (lineIndex >= 15000000) {
	lineIndex = lineIndex % 15000000;
    };
    startIndex = lineIndex;
    linelength = TL_TLS_TLSLEN(line);
    for(;;) {
	if ((linetable_lineTable[lineIndex]) == 0) {
	    if (((linetable_lineTextSize + linelength) + 1) > 750000000) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: too much total unique line text ( > ", (TLint2) 0);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) 750000000, (TLint2) 0);
		TL_TLI_TLIPS ((TLint4) 0, " chars)", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    TLSTRASS(255, (* (TLstring *) &linetable_lineText[linetable_lineTextSize - 1]), line);
	    linetable_lineTable[lineIndex] = linetable_lineTextSize;
	    linetable_lineTextSize += linelength + 1;
	    linetable_lineTableSize += 1;
	    break;
	};
	if (strcmp((* (TLstring *) &linetable_lineText[(linetable_lineTable[lineIndex]) - 1]), line) == 0) {
	    break;
	};
	lineIndex += linetable_secondaryHash;
	if (lineIndex >= 15000000) {
	    lineIndex = lineIndex % 15000000;
	};
	if (lineIndex == startIndex) {
	    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	    TL_TLI_TLIPS ((TLint4) 0, "*** Error: too many unique lines ( > ", (TLint2) 0);
	    TL_TLI_TLIPI ((TLint4) 0, (TLint4) 15000000, (TLint2) 0);
	    TL_TLI_TLIPS ((TLint4) 0, ")", (TLint2) 0);
	    TL_TLI_TLIPK ((TLint2) 0);
	    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	};
    };
    return (lineIndex);
    /* NOTREACHED */
}

static void linetable_gettext (lineIndex, __x89)
linetable_LN	lineIndex;
TLstring	__x89;
{
    {
	TLSTRASS(255, __x89, (* (TLstring *) &linetable_lineText[(linetable_lineTable[lineIndex]) - 1]));
	return;
    };
    /* NOTREACHED */
}

static void linetable_printstats () {
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Used ", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) linetable_lineTextSize, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, "/", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) 750000000, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, " unique line text chars", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " and ", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) linetable_lineTableSize, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, "/", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) 15000000, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, " unique lines", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
}

static void linetable () {
    linetable_lineTextSize = 1;
    linetable_lineTableSize = 0;
    {
	register TLint4	i;
	for (i = 0; i <= 14999999; i++) {
	    linetable_lineTable[i] = 0;
	};
    };
    TLSTRASS(255, (* (TLstring *) &linetable_lineText[0]), " ");
    linetable_lineTextSize += 2;
    linetable_lineTable[0] = ((unsigned long)&(linetable_lineText[1]));
    linetable_lineTableSize = 1;
    linetable_secondaryHash = 11;
    {
	register TLint4	i;
	for (i = 1; i <= 10; i++) {
	    if ((i == 10) || ((linetable_primes[i - 1]) > 15000000)) {
		break;
	    };
	    linetable_secondaryHash = linetable_primes[i - 1];
	};
    };
    {
	register TLint4	c;
	for (c = 0; c <= 255; c++) {
	    linetable_spaceP[c] = 0;
	};
    };
    linetable_spaceP[32] = 1;
    linetable_spaceP[9] = 1;
    linetable_spaceP[12] = 1;
}
struct	PC {
    TLint4	num;
    linetable_LN	info;
    linetable_LN	srcfile;
    TLint4	srcstartline, srcendline;
    TLint4	firstline;
    TLint4	nlines;
    TLint4	embedding;
};
typedef	struct PC	__x156[7500000];
static __x156	pcs;
static TLint4	npcs;
typedef	linetable_LN	__x157[125000000];
static __x157	lines;
static TLint4	nlines;

static void readpcs () {
    TLint4	pcf;
    TL_TLI_TLIOF ((TLnat2) 2, pcfile, &pcf);
    if (pcf == 0) {
	useerr();
    };
    {
	register TLint4	i;
	for (i = 1; i <= 7500000; i++) {
	    TLBIND((*pc), struct PC);
	    TLstring	sourceheader;
	    TLint4	sfindex;
	    TLint4	sfend;
	    TLint4	slindex;
	    TLint4	slend;
	    TLint4	elindex;
	    TLint4	elend;
	    if ((i % 1000) == 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) 0);
	    };
	    if (TL_TLI_TLIEOF((TLint4) pcf)) {
		break;
	    };
	    npcs += 1;
	    pc = &(pcs[npcs - 1]);
	    (*pc).num = i;
	    (*pc).embedding = 0;
	    TL_TLI_TLISS ((TLint4) pcf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, sourceheader, (TLint2) pcf);
	    (*pc).info = linetable_install(sourceheader);
	    if (TL_TLS_TLSIND(sourceheader, "<source ") != 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on pc file", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    sfindex = (TL_TLS_TLSIND(sourceheader, "file=") + TL_TLS_TLSLEN("file=")) + 1;
	    sfend = TL_TLS_TLSIND(sourceheader, " startline=") - 2;
	    {
		TLstring	__x158;
		TL_TLS_TLSBXX(__x158, (TLint4) sfend, (TLint4) sfindex, sourceheader);
		(*pc).srcfile = linetable_install(__x158);
	    };
	    slindex = (TL_TLS_TLSIND(sourceheader, "startline=") + TL_TLS_TLSLEN("startline=")) + 1;
	    slend = TL_TLS_TLSIND(sourceheader, " endline=") - 2;
	    {
		TLstring	__x159;
		TL_TLS_TLSBXX(__x159, (TLint4) slend, (TLint4) slindex, sourceheader);
		(*pc).srcstartline = TL_TLS_TLSVSI(__x159, (TLint4) 10);
	    };
	    elindex = (TL_TLS_TLSIND(sourceheader, "endline=") + TL_TLS_TLSLEN("endline=")) + 1;
	    elend = TL_TLS_TLSLEN(sourceheader) - 2;
	    {
		TLstring	__x160;
		TL_TLS_TLSBXX(__x160, (TLint4) elend, (TLint4) elindex, sourceheader);
		(*pc).srcendline = TL_TLS_TLSVSI(__x160, (TLint4) 10);
	    };
	    (*pc).firstline = nlines;
	    for(;;) {
		TLstring	line;
		if (TL_TLI_TLIEOF((TLint4) pcf)) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on pc file", (TLint2) 0);
		    TL_TLI_TLIPK ((TLint2) 0);
		    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
		};
		TL_TLI_TLISS ((TLint4) pcf, (TLint2) 1);
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) pcf);
		if (strcmp(line, "</source>") == 0) {
		    break;
		};
		if (strcmp(line, "") != 0) {
		    nlines += 1;
		    if (nlines > 125000000) {
			TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
			TL_TLI_TLIPS ((TLint4) 0, "*** Error: too many total lines ( > ", (TLint2) 0);
			TL_TLI_TLIPI ((TLint4) 0, (TLint4) 125000000, (TLint2) 0);
			TL_TLI_TLIPS ((TLint4) 0, ")", (TLint2) 0);
			TL_TLI_TLIPK ((TLint2) 0);
			TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
		    };
		    lines[nlines - 1] = linetable_install(line);
		};
	    };
	    (*pc).nlines = nlines - ((*pc).firstline);
	};
    };
    if (!TL_TLI_TLIEOF((TLint4) pcf)) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: too many potential clones ( > ", (TLint2) 0);
	TL_TLI_TLIPI ((TLint4) 0, (TLint4) 7500000, (TLint2) 0);
	TL_TLI_TLIPS ((TLint4) 0, ")", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
    };
    TL_TLI_TLICL ((TLint4) pcf);
}

static void getprogramarguments () {
    if (TL_TLI_TLIARC != 3) {
	useerr();
    };
    {
	TLstring	__x161;
	TL_TLI_TLIFA((TLint4) 1, __x161);
	TLSTRASS(255, pcfile, __x161);
    };
    {
	TLstring	__x162;
	TL_TLI_TLIFA((TLint4) 2, __x162);
	TLSTRASS(255, ccfile, __x162);
    };
    {
	TLstring	__x163;
	TL_TLI_TLIFA((TLint4) 3, __x163);
	TLSTRASS(255, rccfile, __x163);
    };
}

static void resolvepcsources () {
    TLint4	ccf;
    TLint4	rccf;
    TL_TLI_TLIOF ((TLnat2) 2, ccfile, &ccf);
    if (ccf == 0) {
	useerr();
    };
    TL_TLI_TLIOF ((TLnat2) 4, rccfile, &rccf);
    if (rccf == 0) {
	useerr();
    };
    {
	register TLint4	i;
	for (i = 1; i <= 999999999; i++) {
	    TLstring	line;
	    if ((i % 100) == 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) 0);
	    };
	    if (TL_TLI_TLIEOF((TLint4) ccf)) {
		break;
	    };
	    TL_TLI_TLISS ((TLint4) ccf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) ccf);
	    if (TL_TLS_TLSIND(line, "<source ") == 1) {
		TLstring	sourceheader;
		TLstring	textpcid;
		TLint4	pcid;
		TLBIND((*pc), struct PC);
		{
		    TLstring	__x164;
		    TL_TLS_TLSBXX(__x164, (TLint4) TL_TLS_TLSIND(line, "></source>"), (TLint4) 1, line);
		    TLSTRASS(255, sourceheader, __x164);
		};
		TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, sourceheader, (TLint2) rccf);
		TL_TLI_TLIPK ((TLint2) rccf);
		{
		    TLstring	__x165;
		    TL_TLS_TLSBXX(__x165, (TLint4) (TL_TLS_TLSLEN(sourceheader) - 1), (TLint4) ((TL_TLS_TLSIND(sourceheader, "pcid=") + TL_TLS_TLSLEN("pcid=")) + 1), sourceheader);
		    TLSTRASS(255, textpcid, __x165);
		};
		pcid = TL_TLS_TLSVSI(textpcid, (TLint4) 10);
		pc = &(pcs[pcid - 1]);
		if (((*pc).num) != pcid) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, "*** Error in pc number", (TLint2) 0);
		    TL_TLI_TLIPK ((TLint2) 0);
		    TL_TLE_TLEQUIT ((TLint4) 99, (char *) 0, 0);
		};
		{
		    register TLint4	k;
		    TLint4	__x166;
		    __x166 = (*pc).nlines;
		    k = 1;
		    if (k <= __x166) {
			for(;;) {
			    {
				TLstring	__x167;
				linetable_gettext((linetable_LN) (lines[(((*pc).firstline) + k) - 1]), __x167);
				TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
				TL_TLI_TLIPS ((TLint4) 0, __x167, (TLint2) rccf);
				TL_TLI_TLIPK ((TLint2) rccf);
			    };
			    if (k == __x166) break;
			    k++;
			}
		    };
		};
		TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "</source>", (TLint2) rccf);
		TL_TLI_TLIPK ((TLint2) rccf);
	    } else {
		TL_TLI_TLISS ((TLint4) rccf, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, line, (TLint2) rccf);
		TL_TLI_TLIPK ((TLint2) rccf);
	    };
	};
    };
    TL_TLI_TLICL ((TLint4) rccf);
}
void TProg () {
    minclonelines = 5;
    maxclonelines = 2500;
    linetable();
    npcs = 0;
    nlines = 0;
    getprogramarguments();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Reading pcs ", (TLint2) 0);
    readpcs();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " done.", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Resolving normalized pc sources ", (TLint2) 0);
    resolvepcsources();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " done.", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
}
