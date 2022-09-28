/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/last_date.c	1.10.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/fcntl.h>
#include	<method.h>
#include	<table.h>
#include	<bkhist.h>
#include	<string.h>
#include	<errno.h>

extern char	*br_get_histlog_path();
extern int	brlog();
extern long	strtol();
extern int	TLsearch1();

long 
last_full_date(mp)
m_info_t *mp;
	{
	char		hist_path[256];
	TLdesc_t	descr;
	char		*erstrng;
	unsigned char	*tbldate;
	unsigned char	*bmeth;
	ENTRY		eptr;
	TLsearch_t	sarray[ 2 ];
	int		entryno;
	int		rc;
	int		h_tid = 0;
	long		d;

	if ((mp->flags & pflag) && (mp->lastfull != 0)) { /* use passed date */
#ifdef TRACE
	brlog("Using passed date = 0x%x", mp->lastfull);
#endif
		return (mp->lastfull);
	}
	(void) strcpy(hist_path, br_get_histlog_path());
#ifdef TRACE
	brlog("hist_path=%s",hist_path);
#endif
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = (unsigned char *)H_ENTRY_F;

	if ((rc = TLopen(&h_tid, hist_path, &descr, O_RDONLY)) != TLOK 
				&& rc != TLBADFS && rc != TLDIFFFORMAT) {
		if (rc == TLFAILED) 
			brlog("last_full(): TLopen of history table %s fails: %s",
							hist_path, SE);
		else brlog("last_full(): TLopen of history table %s returns %d",
							hist_path, rc);
		return((long) 0);
	}
	/* Get an entry element for the new status info */
	if (!(eptr = TLgetentry(h_tid))) {
		brlog("last_full(): unable to initialize status table");
		return((long) 0);
	}
	sarray[ 0 ].ts_fieldname = H_ONAME;
	sarray[ 0 ].ts_pattern = (unsigned char *)mp->ofsname;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)NULL;

	entryno = TLEND;

#ifdef TRACE
	if ((mp->flags & pflag) && (mp->lastfull == 0)) {
		brlog("Using incfile also to find last date");
	}
#endif
	while (1) {
		entryno = TLsearch1(h_tid, sarray, entryno, TLBEGIN, TL_AND);
		if (entryno < 0) {
			erstrng = "no entry";
			break;
		}
		if (TLread(h_tid, entryno, eptr) != TLOK) {
			erstrng = "read error";
			break;
		}
		if ((bmeth = TLgetfield(h_tid, eptr, H_METHOD)) == NULL) {
			entryno--;
			continue;
		}
		if ((mp->flags & pflag) && (mp->lastfull == 0)) {
			/* use incfile also */
			if (strcmp((char *)bmeth, "ffile") &&
				strcmp((char *)bmeth, "fimage") &&
					strcmp((char *)bmeth, "incfile")) {
				entryno--;
				continue;
			}
		}
		else {
			if (strcmp((char *)bmeth, "ffile") &&
					strcmp((char *)bmeth, "fimage")) {
				entryno--;
				continue;
			}
		}
		if ((tbldate = TLgetfield(h_tid, eptr, H_DATE)) == NULL) {
			entryno--;
			continue;
		}
		d = strtol((char *) tbldate, (char **)NULL, 16);
		TLfreeentry(h_tid, eptr);
		TLclose(h_tid);
		return(d);
	}
	brlog(" last_date: %s", erstrng);
	TLfreeentry(h_tid, eptr);
	TLclose(h_tid);
	return((long) 0);
} /* last_full_date() */
