/*	@(#)time.h 1.1 85/12/18 SMI; from UCB 4.4 83/07/09	*/
/* Turing Plus version of time.h */

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
type *timeval:
    record
	tv_sec	: int4		/* seconds */
	tv_usec	: int4		/* and microseconds */
    end record

type *timezone:
    record
	tz_minuteswest	: int4	/* minutes west of Greenwich */
	tz_dsttime	: int4	/* type of dst correction */
    end record

const *DST_NONE		:= 0	/* not on dst */
const *DST_USA		:= 1	/* USA style dst */
const *DST_AUST		:= 2	/* Australian style dst */
const *DST_WET		:= 3	/* Western European dst */
const *DST_MET		:= 4	/* Middle European dst */
const *DST_EET		:= 5	/* Eastern European dst */

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
const *ITIMER_REAL	:= 0
const *ITIMER_VIRTUAL	:= 1
const *ITIMER_PROF	:= 2

type *itimerval:
    record
	it_interval	: timeval	/* timer interval */
	it_value	: timeval	/* current value */
    end record

/*
** include "../time.h"
*/
