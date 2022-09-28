/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/msgbuf.h	1.4"
/*
** this file is used for both passes of lint
*/
#ifdef NODBG
#define LNBUG(cond, print)
#else
extern int ln_dbflag;
#define LNBUG(cond, print) if (cond) { DPRINTF print; DPRINTF("\n"); fflush(stderr); }
#endif

#define NOTHING	0
#define ERRMSG	01
#define FATAL	02
#define CCLOSE	04
#define HCLOSE	010
#define LFNM	16
#define LONE	0
#define MULTI	1
#define MAXLEN  256
#ifdef LINT2
#define NUMMSG	19
#else
#define NUMMSG	17
#endif

extern int ln_flags[];
#define LN_FLAG(x)	(ln_flags[(x<'Z') ? (x-'A') : (x-'a'+26)])
#define BWERROR 	bwerror
#define BUERROR 	buerror
#define WERRORLN	lwlerror
#define BWERRORLN	bwerrorln

typedef struct hdritem {
	char	*hname;
	char	*sname;
} HDRITEM;

typedef struct citem {
	long		offset;
	struct citem	*next;
} CITEM;

struct msgent {
	char *compound;
	char *format;
	short colform;
	char *simple;
};

extern struct msgent msg[];

/*
** From messages.c
*/
#ifdef __STDC__
extern void tmpopen(void);
extern void hdrclose(void);
extern void lerror();
extern void bwerror();
#ifndef LINT2
extern void bwerrorln();
extern void lwlerror();
extern void lwerror();
extern void luerror();
extern void lulerror();
extern void lierror();
extern char *strip(char *);
#endif
#else
extern void tmpopen();
extern void hdrclose();
extern void lerror();
extern void bwerror();
#ifndef LINT2
extern void bwerrorln();
extern void lwlerror();
extern void lwerror();
extern void luerror();
extern void lulerror();
extern void lierror();
extern char *strip();
#endif
#endif
