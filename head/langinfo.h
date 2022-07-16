/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:langinfo.h	1.2"

/*
 * The seven days of the week in their full beauty
 */

#define	DAY_1	  1	/* sunday */
#define	DAY_2	  2	/* monday */
#define	DAY_3	  3	/* tuesday */
#define	DAY_4	  4	/* wednesday */
#define	DAY_5	  5	/* thursday */
#define	DAY_6	  6	/* friday */
#define	DAY_7	  7	/* saturday */

/*
 * The abbreviated seven days of the week
 */

#define	ABDAY_1	  8  /* sun */
#define	ABDAY_2	  9  /* mon */
#define	ABDAY_3	  10 /* tue */
#define	ABDAY_4	  11 /* wed */
#define	ABDAY_5	  12 /* thu */
#define	ABDAY_6	  13 /* fri */
#define	ABDAY_7	  14 /* sat */

/*
 * The full names of the twelve months...
 */

#define	MON_1	  15 /* january */
#define	MON_2	  16 /* february */
#define	MON_3	  17 /* march */
#define	MON_4	  18 /* april */
#define	MON_5	  19 /* may */
#define	MON_6	  20 /* june */
#define	MON_7	  21 /* july */
#define	MON_8	  22 /* august */
#define	MON_9	  23 /* september */
#define	MON_10	  24 /* october */
#define	MON_11	  25 /* november */
#define	MON_12	  26 /* december */

/*
 * ... and their abbreviated form
 */

#define	ABMON_1	  27 /* jan */
#define	ABMON_2	  28 /* feb */
#define	ABMON_3	  29 /* mar */
#define	ABMON_4	  30 /* apr */
#define	ABMON_5	  31 /* may */
#define	ABMON_6	  32 /* jun */
#define	ABMON_7	  33 /* jul */
#define	ABMON_8	  34 /* aug */
#define	ABMON_9	  35 /* sep */
#define	ABMON_10  36 /* oct */
#define	ABMON_11  37 /* nov */
#define	ABMON_12  38 /* dec */

/*
 * plus some special strings you might need to know
 */

#define	RADIXCHAR 39	/* radix character */
#define	THOUSEP	  40	/* separator for thousand */
#define	YESSTR	  41    /* affirmative response for yes/no queries */
#define	NOSTR	  42  	/* negative response for yes/no queries */
#define CRNCYSTR  43 	/* currency symbol */

/*
 * Default string used to format date and time
 *	e.g. Sunday, August 24 21:08:38 MET 1986
 */

#define	D_T_FMT	  44 	/* string for formatting date and time */
#define D_FMT     45	/* date format */
#define T_FMT     46	/* time format */
#define AM_STR	  47	/* am string */
#define PM_STR	  48	/* pm string */

#define	MAXSTRMSG	48 /* Maximum number of strings in langinfo */ 
/*
 * and the definitions of functions langinfo(3C)
 */
#if defined(__STDC__)
#include <nl_types.h>
char   *nl_langinfo(nl_item);	/* get a string from the database	*/
#else 
char   *nl_langinfo();		/* get a string from the database	*/
#endif
