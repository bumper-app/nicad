/*
 * Adapted with permission from :
 *
 * Turing Plus Run-time Support Library
 * (c) 1987,1988,1989,1990 Holt Software Associates Inc.
 * All rights reserved.
 *
 */

void TL_TLS_TLSBXS (target, endOffset, startIndex, source)
    char * target;
    int endOffset, startIndex;
    char * source;
    {
	register char *dst = target;
	register char *src = source + startIndex - 1;
	char *lastAddr = source + strlen(source) + endOffset;

	while (src < lastAddr) {
	    *dst++ = *src++;
	}
	*dst = '\0';
    }

void TL_TLS_TLSBXX (target, endIndex, startIndex, source)
    char * target;
    int endIndex, startIndex;
    char * source;
    {
	register char *dst = target;
	register char *src = source + startIndex - 1;
	char *lastAddr = source + endIndex;

	while (src < lastAddr) {
	    *dst++ = *src++;
	}
	*dst = '\0';
    }

int TL_TLS_TLSIND (source, pattern)
    char * source, * pattern;
    {
	char firstPatternChar = *pattern;
	register char *src;

	if (firstPatternChar == '\0') {
	    return (1);
	}

	src = source;

	while (*src != '\0') {
	    if (*src == firstPatternChar) {
		register char *chk = src + 1;
		register char *pat = pattern + 1;

		for (;;) {
		    if (*pat == '\0') {
			return (src - source + 1);
		    }
		    if (*pat != *chk) break;
		    pat++;
		    chk++;
		}
	    }

	    src++;
	}

	return (0);
    }

void TL_TLS_TLSRPT (source, count, target)
    char *source;
    int count;
    char *target;
    {
	register char *dst = target;
	int c;

	if ((count <= 0) || (*source == '\0')) {
	    *dst = '\0';
	    return;
	}

	c = count;
	while (count--) {
	    register char *src = source;
	    for (;;) {
		*dst++ = *src++;
		if (*src == '\0') break;
	    }
	}
	*dst = '\0';
    }

void TL_TLS_TLSVIS (value, width, base, target)
    int value, width, base;
    char * target;
    {
	char buffer[255];
	register char *buf = buffer + 255;
	register int count	= 1;
	register char *dst;

	if (value < 0) {
	    register int tmp = value;
	    for (;;) {
		buf--;
		*buf = - (tmp % base);
		if (*buf < 10) {
		    *buf += '0';
		} else {
		    *buf += 'A' - 10;
		}
		tmp /= base;
		if (tmp == 0) break;
		count++;
	    }
	    buf--;
	    *buf = '-';
	    count++;
	} else {
	    register int tmp = value;

	    for (;;) {
		buf--;
		*buf = tmp % base;
		if (*buf < 10) {
		    *buf += '0';
		} else {
		    *buf += 'A' - 10;
		}
		tmp /= base;
		if (tmp == 0) break;
		count++;
	    }
	}

	dst = target;
	for (;;) {
	    *dst++ = *buf++;
	    count--;
	    if (count == 0) break;
	}
	*dst = '\0';
    }

void TL_TLS_TLSVNS (value, width, base, target)
    unsigned int value, width, base;
    char * target;
    {
	char buffer[255];
	register char *buf = buffer + 255;
	register int count	= 1;
	register char *dst;

	register unsigned int tmp = value;

	for (;;) {
	    buf--;
	    *buf = tmp % base;
	    if (*buf < 10) {
		*buf += '0';
	    } else {
		*buf += 'A' - 10;
	    }
	    tmp /= base;
	    if (tmp == 0) break;
	    count++;
	}

	dst = target;
	for (;;) {
	    *dst++ = *buf++;
	    count--;
	    if (count == 0) break;
	}
	*dst = '\0';
    }

double TL_TLS_TLSVS8 (source)
char *source;
{
    double	answer;
    sscanf (source, "%le", &answer);
    return (answer);
}

int TL_TLS_TLSVSI (source, base)
char *source;
int base; /* 10 */
{
    int	answer;
    sscanf (source, "%li", &answer);
    return (answer);
}
