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
static TLstring	cpfile;

static void useerr () {
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Usage :  cloneclass.x cpfile.xml > clone-clusters-file.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "  e.g.:  cloneclass.x linux_functions-clonepairs.xml > linux_functions-clone-clusters.xml", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
}
typedef	TLchar	linetable___x150[75000001];
static linetable___x150	linetable_lineText;
static TLint4	linetable_lineTextSize;
typedef	TLint4	linetable___x153[15000000];
static linetable___x153	linetable_lineTable;
static TLint4	linetable_lineTableSize;
typedef	TLnat4	linetable_LN;

static TLnat4 linetable_hash (s)
TLstring	s;
{
    typedef	TLnat1	__x157[256];
    typedef	__x157	nat256;
    register TLnat4	h;
    register TLnat4	j;
    h = TL_TLS_TLSLEN(s);
    j = h - 1;
    if (h > 0) {
	{
	    register TLint4	i;
	    TLint4	__x159;
	    __x159 = h >> 1;
	    i = 0;
	    if (i <= __x159) {
		for(;;) {
		    h += ((TLint4) h << 1) + ((* (nat256 *) s)[i]);
		    h += ((TLint4) h << 1) + ((* (nat256 *) s)[j]);
		    j -= 1;
		    if (i == __x159) break;
		    i++;
		}
	    };
	};
    };
    return (h);
    /* NOTREACHED */
}
static TLint4	linetable_secondaryHash;
typedef	TLint4	linetable___x164[10];
static linetable___x164	linetable_primes = 
    {1021, 2027, 4091, 8191, 16381, 32003, 65003, 131009, 262007, 524047};
typedef	TLboolean	linetable___x168[256];
static linetable___x168	linetable_spaceP;

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
	    TLchar	__x171[2];
	    if ((first > rawlinelength) || ((TL_TLS_TLSBX(__x171, (TLint4) first, line), !(linetable_spaceP[((TLnat4) TLCVTTOCHR(__x171))])))) {
		break;
	    };
	};
	first += 1;
    };
    last = rawlinelength;
    for(;;) {
	{
	    TLchar	__x172[2];
	    if ((last < 1) || ((TL_TLS_TLSBX(__x172, (TLint4) last, line), !(linetable_spaceP[((TLnat4) TLCVTTOCHR(__x172))])))) {
		break;
	    };
	};
	last -= 1;
    };
    {
	TLstring	__x173;
	TL_TLS_TLSBXX(__x173, (TLint4) last, (TLint4) first, line);
	TLSTRASS(255, line, __x173);
    };
    lineIndex = linetable_hash(line);
    if (lineIndex >= 15000000) {
	lineIndex = lineIndex % 15000000;
    };
    startIndex = lineIndex;
    linelength = TL_TLS_TLSLEN(line);
    for(;;) {
	if ((linetable_lineTable[lineIndex]) == 0) {
	    if (((linetable_lineTextSize + linelength) + 1) > 75000000) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: too much total unique line text ( > ", (TLint2) 0);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) 75000000, (TLint2) 0);
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

static void linetable_gettext (lineIndex, __x84)
linetable_LN	lineIndex;
TLstring	__x84;
{
    {
	TLSTRASS(255, __x84, (* (TLstring *) &linetable_lineText[(linetable_lineTable[lineIndex]) - 1]));
	return;
    };
    /* NOTREACHED */
}

static void linetable_printstats () {
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Used ", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) linetable_lineTextSize, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, "/", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) 75000000, (TLint2) 0);
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
static TLint4	systeminfo;
static TLint4	cloneinfo;
static TLint4	runinfo;
struct	CP {
    TLint4	info;
    TLint4	nlines;
    TLint4	similarity;
    TLint4	pc1info, pc2info;
    TLint4	pc1, pc2;
    TLint4	cluster;
};
typedef	struct CP	__x178[12500000];
static __x178	pairs;
static TLint4	npairs;

static void readcps () {
    TLint4	cpf;
    TLstring	line;
    TL_TLI_TLIOF ((TLnat2) 2, cpfile, &cpf);
    if (cpf == 0) {
	useerr();
    };
    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
    if (strcmp(line, "<clones>") != 0) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: input file not a clone pair file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	useerr();
    };
    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
    if (TL_TLS_TLSIND(line, "<systeminfo ") != 1) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    } else {
	systeminfo = linetable_install(line);
    };
    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
    if (TL_TLS_TLSIND(line, "<cloneinfo ") != 1) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    } else {
	cloneinfo = linetable_install(line);
    };
    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
    if (TL_TLS_TLSIND(line, "<runinfo ") != 1) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
	TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
    } else {
	runinfo = linetable_install(line);
    };
    {
	register TLint4	i;
	for (i = 1; i <= 12500000; i++) {
	    TLBIND((*pair), struct CP);
	    TLstring	nlines;
	    TLstring	similarity;
	    TLstring	pcid1;
	    TLstring	pcid2;
	    if ((i % 1000) == 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) 0);
	    };
	    if (TL_TLI_TLIEOF((TLint4) cpf)) {
		break;
	    };
	    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    if (strcmp(line, "") == 0) {
		TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    };
	    if (TL_TLS_TLSIND(line, "</clones>") == 1) {
		break;
	    };
	    if (TL_TLS_TLSIND(line, "<clone ") != 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    npairs += 1;
	    pair = &(pairs[npairs - 1]);
	    (*pair).info = linetable_install(line);
	    {
		TLstring	__x179;
		TL_TLS_TLSBXX(__x179, (TLint4) (TL_TLS_TLSIND(line, " similarity=\"") - 2), (TLint4) (TL_TLS_TLSIND(line, "nlines=\"") + TL_TLS_TLSLEN("nlines=\"")), line);
		TLSTRASS(255, nlines, __x179);
	    };
	    {
		TLstring	__x180;
		TL_TLS_TLSBXX(__x180, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "similarity=\"") + TL_TLS_TLSLEN("similarity=\"")), line);
		TLSTRASS(255, similarity, __x180);
	    };
	    (*pair).nlines = TL_TLS_TLSVSI(nlines, (TLint4) 10);
	    (*pair).similarity = TL_TLS_TLSVSI(similarity, (TLint4) 10);
	    (*pair).cluster = 0;
	    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    if (strcmp(line, "") == 0) {
		TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    };
	    if (TL_TLS_TLSIND(line, "<source ") != 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    (*pair).pc1info = linetable_install(line);
	    {
		TLstring	__x181;
		TL_TLS_TLSBXX(__x181, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "pcid=\"") + TL_TLS_TLSLEN("pcid=\"")), line);
		TLSTRASS(255, pcid1, __x181);
	    };
	    (*pair).pc1 = TL_TLS_TLSVSI(pcid1, (TLint4) 10);
	    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    if (strcmp(line, "") == 0) {
		TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
		TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    };
	    if (TL_TLS_TLSIND(line, "<source ") != 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	    (*pair).pc2info = linetable_install(line);
	    {
		TLstring	__x182;
		TL_TLS_TLSBXX(__x182, (TLint4) (TL_TLS_TLSIND(line, "\">") - 1), (TLint4) (TL_TLS_TLSIND(line, "pcid=\"") + TL_TLS_TLSLEN("pcid=\"")), line);
		TLSTRASS(255, pcid2, __x182);
	    };
	    (*pair).pc2 = TL_TLS_TLSVSI(pcid2, (TLint4) 10);
	    TL_TLI_TLISS ((TLint4) cpf, (TLint2) 1);
	    TL_TLI_TLIGSS((TLint4) 255, line, (TLint2) cpf);
	    if (TL_TLS_TLSIND(line, "</clone>") != 1) {
		TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		TL_TLI_TLIPS ((TLint4) 0, "*** Error: synchronization error on clone pair file", (TLint2) 0);
		TL_TLI_TLIPK ((TLint2) 0);
		TL_TLE_TLEQUIT ((TLint4) 1, (char *) 0, 0);
	    };
	};
    };
    if (!TL_TLI_TLIEOF((TLint4) cpf)) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, "*** Error: too many clone pairs ( > ", (TLint2) 0);
	TL_TLI_TLIPI ((TLint4) 0, (TLint4) 12500000, (TLint2) 0);
	TL_TLI_TLIPS ((TLint4) 0, ")", (TLint2) 0);
	TL_TLI_TLIPK ((TLint2) 0);
    };
    TL_TLI_TLICL ((TLint4) cpf);
}

static void swapclonepairs (i, j)
TLint4	i;
TLint4	j;
{
    struct CP	t;
    TLSTRCTASS(t, pairs[i - 1], struct CP);
    TLSTRCTASS(pairs[i - 1], pairs[j - 1], struct CP);
    TLSTRCTASS(pairs[j - 1], t, struct CP);
}
static TLint4	depth;

static void quicksortclonepairs (first, last)
TLint4	first;
TLint4	last;
{
    depth = depth + 1;
    if ((depth % 1000) == 1) {
	TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
	TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) 0);
    };
    if (first < last) {
	TLint4	low;
	TLint4	high;
	TLint4	pivot;
	low = first;
	high = last + 1;
	pivot = low;
	for(;;) {
	    for(;;) {
		high -= 1;
		if (low == high) {
		    break;
		};
		if ((pairs[high - 1].pc1) < (pairs[pivot - 1].pc1)) {
		    break;
		};
	    };
	    if (low == high) {
		break;
	    };
	    for(;;) {
		low += 1;
		if (low == high) {
		    break;
		};
		if ((pairs[pivot - 1].pc1) < (pairs[low - 1].pc1)) {
		    break;
		};
	    };
	    if (low == high) {
		break;
	    };
	    swapclonepairs((TLint4) low, (TLint4) high);
	};
	swapclonepairs((TLint4) high, (TLint4) pivot);
	quicksortclonepairs((TLint4) first, (TLint4) (high - 1));
	quicksortclonepairs((TLint4) (high + 1), (TLint4) last);
    };
    depth = depth - 1;
}
struct	CL {
    TLint4	pairs;
    TLint4	npairs;
};
typedef	struct CL	__x183[2500000];
static __x183	clusters;
static TLint4	nclusters;
typedef	TLint4	__x184[12500000];
static __x184	clusterpairs;
static TLint4	nclusterpairs;

static void makeclusterpairs () {
    {
	register TLint4	i;
	TLint4	__x187;
	__x187 = npairs;
	i = 1;
	if (i <= __x187) {
	    for(;;) {
		TLBIND((*pair), struct CP);
		pair = &(pairs[i - 1]);
		if (((*pair).cluster) == 0) {
		    TLBIND((*cluster), struct CL);
		    TLint4	cp;
		    nclusters += 1;
		    (*pair).cluster = nclusters;
		    cluster = &(clusters[nclusters - 1]);
		    (*cluster).pairs = nclusterpairs;
		    (*cluster).npairs = 1;
		    nclusterpairs += 1;
		    clusterpairs[nclusterpairs - 1] = i;
		    cp = (*cluster).pairs;
		    for(;;) {
			if (cp > nclusterpairs) {
			    break;
			};
			{
			    register TLint4	j;
			    TLint4	__x188;
			    __x188 = npairs;
			    j = i + 1;
			    if (j <= __x188) {
				for(;;) {
				    if ((pairs[j - 1].cluster) == 0) {
					TLBIND((*pj), struct CP);
					TLBIND((*pcp), struct CP);
					pj = &(pairs[j - 1]);
					pcp = &(pairs[(clusterpairs[cp - 1]) - 1]);
					if ((((((*pj).pc1) == ((*pcp).pc1)) || (((*pj).pc1) == ((*pcp).pc2))) || (((*pj).pc2) == ((*pcp).pc1))) || (((*pj).pc2) == ((*pcp).pc2))) {
					    pairs[j - 1].cluster = nclusters;
					    (*cluster).npairs += 1;
					    nclusterpairs += 1;
					    clusterpairs[nclusterpairs - 1] = j;
					};
				    };
				    if (j == __x188) break;
				    j++;
				}
			    };
			};
			cp += 1;
		    };
		    cp = (*cluster).pairs;
		    for(;;) {
			TLBIND((*pcp), struct CP);
			TLint4	ccp;
			if (cp > nclusterpairs) {
			    break;
			};
			pcp = &(pairs[(clusterpairs[cp - 1]) - 1]);
			ccp = cp + 1;
			for(;;) {
			    TLBIND((*pccp), struct CP);
			    if (ccp > nclusterpairs) {
				break;
			    };
			    pccp = &(pairs[(clusterpairs[ccp - 1]) - 1]);
			    if ((((*pccp).pc1) == ((*pcp).pc1)) || (((*pccp).pc1) == ((*pcp).pc2))) {
				(*pccp).pc1 = 0;
			    };
			    if ((((*pccp).pc2) == ((*pcp).pc1)) || (((*pccp).pc2) == ((*pcp).pc2))) {
				(*pccp).pc2 = 0;
			    };
			    ccp += 1;
			};
			cp += 1;
		    };
		};
		if (i == __x187) break;
		i++;
	    }
	};
    };
}

static void showclusters () {
    TL_TLI_TLISSO ();
    TL_TLI_TLIPS ((TLint4) 0, "<clones>", (TLint2) -1);
    TL_TLI_TLIPK ((TLint2) -1);
    {
	TLstring	__x189;
	linetable_gettext((linetable_LN) systeminfo, __x189);
	TL_TLI_TLISSO ();
	TL_TLI_TLIPS ((TLint4) 0, __x189, (TLint2) -1);
	TL_TLI_TLIPK ((TLint2) -1);
    };
    {
	TLstring	__x190;
	linetable_gettext((linetable_LN) cloneinfo, __x190);
	TL_TLI_TLISSO ();
	TL_TLI_TLIPS ((TLint4) 0, __x190, (TLint2) -1);
	TL_TLI_TLIPK ((TLint2) -1);
    };
    {
	TLstring	__x191;
	linetable_gettext((linetable_LN) runinfo, __x191);
	TL_TLI_TLISSO ();
	TL_TLI_TLIPS ((TLint4) 0, __x191, (TLint2) -1);
	TL_TLI_TLIPK ((TLint2) -1);
    };
    TL_TLI_TLISSO ();
    TL_TLI_TLIPS ((TLint4) 0, "<classinfo nclasses=\"", (TLint2) -1);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) nclusters, (TLint2) -1);
    TL_TLI_TLIPS ((TLint4) 0, "\"/>", (TLint2) -1);
    TL_TLI_TLIPK ((TLint2) -1);
    TL_TLI_TLISSO ();
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) -1);
    TL_TLI_TLIPK ((TLint2) -1);
    {
	register TLint4	i;
	TLint4	__x192;
	__x192 = nclusters;
	i = 1;
	if (i <= __x192) {
	    for(;;) {
		TLBIND((*cl), struct CL);
		TLint4	nlines;
		TLint4	nclones;
		TLint4	similarity;
		if ((i % 100) == 1) {
		    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
		    TL_TLI_TLIPS ((TLint4) 0, ".", (TLint2) 0);
		};
		cl = &(clusters[i - 1]);
		nlines = pairs[(clusterpairs[(((*cl).pairs) + 1) - 1]) - 1].nlines;
		nclones = 0;
		similarity = 100;
		{
		    register TLint4	j;
		    TLint4	__x193;
		    __x193 = (*cl).npairs;
		    j = 1;
		    if (j <= __x193) {
			for(;;) {
			    TLBIND((*cp), struct CP);
			    cp = &(pairs[(clusterpairs[(((*cl).pairs) + j) - 1]) - 1]);
			    if (((*cp).pc1) != 0) {
				nclones += 1;
			    };
			    if (((*cp).pc2) != 0) {
				nclones += 1;
			    };
			    if (((*cp).similarity) < similarity) {
				similarity = (*cp).similarity;
			    };
			    if (j == __x193) break;
			    j++;
			}
		    };
		};
		TL_TLI_TLISSO ();
		TL_TLI_TLIPS ((TLint4) 0, "<class classid=\"", (TLint2) -1);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) i, (TLint2) -1);
		TL_TLI_TLIPS ((TLint4) 0, "\" nclones=\"", (TLint2) -1);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) nclones, (TLint2) -1);
		TL_TLI_TLIPS ((TLint4) 0, "\" nlines=\"", (TLint2) -1);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) nlines, (TLint2) -1);
		TL_TLI_TLIPS ((TLint4) 0, "\" similarity=\"", (TLint2) -1);
		TL_TLI_TLIPI ((TLint4) 0, (TLint4) similarity, (TLint2) -1);
		TL_TLI_TLIPS ((TLint4) 0, "\">", (TLint2) -1);
		TL_TLI_TLIPK ((TLint2) -1);
		{
		    register TLint4	j;
		    TLint4	__x194;
		    __x194 = (*cl).npairs;
		    j = 1;
		    if (j <= __x194) {
			for(;;) {
			    TLBIND((*cp), struct CP);
			    cp = &(pairs[(clusterpairs[(((*cl).pairs) + j) - 1]) - 1]);
			    if (((*cp).pc1) != 0) {
				{
				    TLstring	__x195;
				    linetable_gettext((linetable_LN) ((*cp).pc1info), __x195);
				    TL_TLI_TLISSO ();
				    TL_TLI_TLIPS ((TLint4) 0, __x195, (TLint2) -1);
				    TL_TLI_TLIPK ((TLint2) -1);
				};
			    };
			    if (((*cp).pc2) != 0) {
				{
				    TLstring	__x196;
				    linetable_gettext((linetable_LN) ((*cp).pc2info), __x196);
				    TL_TLI_TLISSO ();
				    TL_TLI_TLIPS ((TLint4) 0, __x196, (TLint2) -1);
				    TL_TLI_TLIPK ((TLint2) -1);
				};
			    };
			    if (j == __x194) break;
			    j++;
			}
		    };
		};
		TL_TLI_TLISSO ();
		TL_TLI_TLIPS ((TLint4) 0, "</class>", (TLint2) -1);
		TL_TLI_TLIPK ((TLint2) -1);
		TL_TLI_TLISSO ();
		TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) -1);
		TL_TLI_TLIPK ((TLint2) -1);
		if (i == __x192) break;
		i++;
	    }
	};
    };
    TL_TLI_TLISSO ();
    TL_TLI_TLIPS ((TLint4) 0, "</clones>", (TLint2) -1);
    TL_TLI_TLIPK ((TLint2) -1);
}

static void getprogramarguments () {
    if (TL_TLI_TLIARC != 1) {
	useerr();
    };
    {
	TLstring	__x197;
	TL_TLI_TLIFA((TLint4) 1, __x197);
	TLSTRASS(255, cpfile, __x197);
    };
}
void TProg () {
    linetable();
    npairs = 0;
    depth = 0;
    nclusters = 0;
    nclusterpairs = 0;
    getprogramarguments();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Processing ", (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, cpfile, (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Reading clone pairs ", (TLint2) 0);
    readcps();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " done.", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Total ", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) npairs, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, " pairs", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Computing classes ", (TLint2) 0);
    makeclusterpairs();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " done.", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Clustered clone pairs into ", (TLint2) 0);
    TL_TLI_TLIPI ((TLint4) 0, (TLint4) nclusters, (TLint2) 0);
    TL_TLI_TLIPS ((TLint4) 0, " classes", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "Generating XML output ", (TLint2) 0);
    showclusters();
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, " done.", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
    TL_TLI_TLISS ((TLint4) 0, (TLint2) 2);
    TL_TLI_TLIPS ((TLint4) 0, "", (TLint2) 0);
    TL_TLI_TLIPK ((TLint2) 0);
}
