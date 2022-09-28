/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/rsnewvol.c	1.11.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	"libadmIO.h"
#include	<method.h>
#include	<brarc.h>
#include	<errno.h>

extern int	brgetvolume();
extern int	brlog();
extern char	*nxt_list();

extern int	bklevels;

int
rsnewvol(mp, fname, bytesleft, checksize, media_name)
m_info_t	*mp;
char		fname[];
long		*bytesleft;
short		*checksize;
char		**media_name;
{
	int	isfile = 0;
	char	*dmname = NULL;
	char	label[81];

	*bytesleft = 0;
	*checksize = 0;

	switch(mp->dtype) {
	case IS_DIR:
		dmname = nxt_list(mp);

		if ((dmname == NULL) || (!(*dmname))) {
			return(-1);
		}
		*media_name = dmname;
		(void) sprintf(fname,"%s/%s",mp->dname,dmname);
		isfile++;

		if ((mp->blks_per_vol) > 0) {
			*bytesleft = mp->blks_per_vol << 9;
			*checksize = 1;
		}
		break;
	case IS_FILE:
		if (!strcmp(fname, mp->dname)) {	/* already used */
			brlog("rsnewvol: %s has been used",fname);
			return(-1);
		}
		(void) strcpy(fname, mp->dname);
		isfile++;
		break;
	case IS_DPART:
		if (!strcmp(fname, mp->dname)) {	/* already used */
			brlog("rsnewvol: %s has been used",fname);
			return(-1);
		}
		if ((mp->blks_per_vol) <= 0) {
			brlog("rsnewvol: size of partition %s not specified",
					mp->dname);
			return(-1);
		}
		(void) strcpy(fname, mp->dname);
		*bytesleft = mp->blks_per_vol << 9;
		*checksize = 1;
		break;
	default:
		if (mp->volpromt) {
			dmname = nxt_list(mp);
			*media_name = dmname;
			if (brgetvolume(dmname, OVRIDE(mp), AUTOM(mp), label) != BRSUCCESS) {
				brlog("rsnewvol: dev %s vol %s not accessible",
					mp->dname, dmname);
				return(-1);
			}
		}
		else {
			if (!strcmp(fname, mp->dname)) {	/* already used */
				brlog("rsnewvol: %s has been used",fname);
				return(-1);
			}
		}
		(void) strcpy(fname, mp->dname);

		if ((mp->blks_per_vol) > 0) {
			*bytesleft = mp->blks_per_vol << 9;
			*checksize = 1;
		}
	}
#ifdef TRACE
	if (isfile || (dmname == NULL)) {
		brlog(" rsnewvol new name = %s",fname);
	}
	else {
		brlog(" rsnewvol dev %s  vol=%s", fname, dmname);
	}
#endif
	return(isfile);
} /* rsnewvol() */

GFILE *
new_Input(mp, name, bytes_left, checksize, ai)
m_info_t		*mp;
char			name[];
long			*bytes_left;
short			*checksize;
struct archive_info	*ai;
{
	long	partsize;
	GFILE	*f = NULL;
	int	isfile = 0;
	int	hdrsize;
	int	flags;
	char	*dmname = NULL;

	isfile = rsnewvol(mp, name, bytes_left, checksize, &dmname);

	if (isfile < 0) {
		brlog("unable to complete archive processing");
		return(NULL);
	}
#ifdef TRACE
	if (isfile) {
		brlog(" new_Input new name = %s",name);
	}
	else {
		brlog(" new_Input opening %s  vol=%s", name, dmname);
	}
#endif
	ai->br_length = 0l;
	flags = BR_LABEL_CHECK | BR_PROMPT_ALLWD;
	(void) rsgethdr(mp->dname, mp->dchar, dmname, ai, flags, &f);

	if (f == NULL) {
		brlog(" do_file open of archive %s failed %s", name, SE);
		return(f);
	}
	partsize = (mp->dtype == IS_DPART) ? ((mp->blks_per_vol << 9)) : 0l;

	if ((hdrsize = (int) (ai->br_length)) < 0) {
		brlog("no archive hdr found %s", isfile ? name : dmname);
	}
	else {
		if ((ai->br_media_cap) > 0) {
			brlog("change cap to hdr val %d",ai->br_media_cap);
			*bytes_left = (ai->br_media_cap) << 9;
		}
		brlog("sys=%s seq=%d meth=%s mname=%s",ai->br_sysname,
			ai->br_seqno,ai->br_method,ai->br_mname);
		brlog("hdr size was %d bytes",hdrsize);
		*bytes_left -= hdrsize;
	}
	return(f);
} /* new_Input() */
