/*
 * Adapted with permission from :
 *
 * Turing Plus Run-time Support Library
 * (c) 1987,1988,1989,1990 Holt Software Associates Inc.
 * All rights reserved.
 *
 */

/* Fixed bug in file table overflow handling -- JRC 21.8.95 */
/* Fixed bug in fetcharg implementation -- JRC 14.2.96 */
/* TL_TLI_TLIFS added to allow flushing of buffered streams -- JRC 18.3.97 */
/* Added support for character append files -- JRC 15.6.05 */

#include "cinterface"

#define CHAR_READ	1
#define CHAR_WRITE	2
#define BINARY_READ	3
#define BINARY_WRITE	4

#define OPEN_CHAR_READ		2
#define OPEN_CHAR_WRITE		4
#define OPEN_CHAR_APPEND	37	/* 1 + 4 + 32 */
#define OPEN_BINARY_READ	8
#define OPEN_BINARY_WRITE	16

void TL_TLI_TLIOF (openMode, fileName, streamNo)
    unsigned short openMode;
    char *fileName;
    int *streamNo;
    {
	char * duplicate;
	int sn;
	for (sn = TL_nextfile; sn < 25 - 3; sn++) {
	    if (TL_files [sn + 2] == NULL) break;
	}
	if (sn == 25 - 3) {
	    *streamNo = 0;
	    return;
	}
	*streamNo = sn;
	duplicate = (char *) (malloc (256));
	strcpy (duplicate, fileName);
	TL_filenames [*streamNo+2] = duplicate;

	switch (openMode) {
	    case OPEN_BINARY_WRITE:
		 TL_files [*streamNo+2] = fopen (fileName, "wb");
		 TL_filemode [*streamNo+2] = WRITE_MODE;
		 break;
	    case OPEN_CHAR_WRITE:
		 TL_files [*streamNo+2] = fopen (fileName, "w");
		 TL_filemode [*streamNo+2] = WRITE_MODE;
		 break;
	    case OPEN_CHAR_APPEND:
		 TL_files [*streamNo+2] = fopen (fileName, "a");
		 TL_filemode [*streamNo+2] = WRITE_MODE;
		 break;
	    case OPEN_BINARY_READ:
		 TL_files [*streamNo+2] = fopen (fileName, "rb");
		 TL_filemode [*streamNo+2] = READ_MODE;
		 break;
	    case OPEN_CHAR_READ:
		 TL_files [*streamNo+2] = fopen (fileName, "r");
		 TL_filemode [*streamNo+2] = READ_MODE;
		 break;
	}

	if (TL_files [*streamNo+2] == NULL) {
	    *streamNo = 0;
	}
    }

void TL_TLI_TLISS (streamNo, ioMode)
    int streamNo;
    short ioMode;
    {
	if (TL_files [streamNo+2] == NULL) {
	    switch (ioMode) {
		case BINARY_WRITE:
		     TL_files [streamNo+2] = fopen (TL_filenames [streamNo+2], "wb");
		     TL_filemode [streamNo+2] = WRITE_MODE;
		     break;
		case CHAR_WRITE:
		     TL_files [streamNo+2] = fopen (TL_filenames [streamNo+2], "w");
		     TL_filemode [streamNo+2] = WRITE_MODE;
		     break;
		case BINARY_READ:
		     TL_files [streamNo+2] = fopen (TL_filenames [streamNo+2], "rb");
		     TL_filemode [streamNo+2] = READ_MODE;
		     break;
		case CHAR_READ:
		     TL_files [streamNo+2] = fopen (TL_filenames [streamNo+2], "r");
		     TL_filemode [streamNo+2] = READ_MODE;
		     break;
	    }

	    if (TL_files [streamNo+2] == NULL) {
		fprintf (stderr, "ERROR: unable to open file\n");
		exit (1);
	    }
	}
    }

void TL_TLI_TLIFS ()
    {
	/* Flush all output streams, including stdout and stderr */
	int i;
	for (i=1; i<25; i++) {
	    if (TL_files[i] != NULL && TL_filemode[i] == WRITE_MODE) fflush (TL_files[i]);
	}
    }

void TL_TLI_TLIGK (streamNo)
short	streamNo;
{
    unsigned char ch;
    for(;;) {
	ch = fgetc (TL_files[streamNo+2]);
	if (ch == (unsigned char) EOF) {
	    return;
	};
	if ((((ch != ' ') && (ch != '\t')) && (ch != '\n')) && (ch != '\f')) {
	    break;
	};
    };
    ungetc (ch, TL_files[streamNo+2]);
}

void TL_TLI_TLIFA (argNo, argString)
int argNo;
char * argString;
{
    if (argNo <= TL_TLI_TLIARC) {
	strcpy (argString, TL_TLI_TLIARV [argNo]);
    } else {
	strcpy (argString, "");
    };
}
