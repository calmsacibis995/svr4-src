/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/bknewvol.c	1.6.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>
#include	<setjmp.h>

extern void	br_state();
extern int	brinvlbl();
extern int	brgetvolume();
extern int	brlog();
extern int	get_media_name();
extern void	newlabel();

extern int	bklevels;

int
bknewvol(mp, fname, reuse, envp, mi)
m_info_t	*mp;
char		fname[];
short		*reuse;
jmp_buf		*envp;
media_info_t	*mi;
{
	int		isfile = 0;
	int		ret;
	char		*dmname = NULL;
	char		label[81];
	media_list_t	*m_info;
#ifdef TRACE
brlog("bknewvol: dtype=%d", mp->dtype);
#endif
	switch(mp->dtype) {
	case IS_DIR:
		if (get_media_name(reuse, &m_info, mi, mp)) {
			return(-1);
		}
		(void) sprintf(fname,"%s/%s",mp->dname,m_info->label);
		isfile++;
		break;
	case IS_FILE:
		if (!strcmp(fname, mp->dname)) {	/* already used */
			brlog("bknewvol: %s has been used",fname);
			sprintf(ME(mp), "Job ID %s: %s has been used", mp->jobid, fname);
			return(-1);
		}
		(void) strcpy(fname, mp->dname);
		isfile++;
		break;
	case IS_DPART:
		if ((mp->blks_per_vol) <= 0) {
			brlog("bknewvol: size of partition %s not specified",
					mp->dname);
			sprintf(ME(mp), "Job ID %s: size of partition %s not specified", mp->jobid, mp->dname);
			return(-1);
		}
		if (!strcmp(fname, mp->dname)) {	/* already used */
			brlog("bknewvol: %s has been used",fname);
			sprintf(ME(mp), "Job ID %s: %s has been used", mp->jobid, fname);
			return(-1);
		}
		(void) strcpy(fname, mp->dname);
		break;
	default:
		if (mp->volpromt) {
			if (get_media_name(reuse, &m_info, mi, mp)) {
				return(-1);
			}
			dmname = m_info->label;
trylab:
#ifdef TRACE
			brlog("call brgetvolume for %s",dmname);
#endif
			ret = brgetvolume(dmname, OVRIDE(mp), AUTOM(mp), label);

			if (ret != BRSUCCESS) {
				brlog("brgetvol returned %d", ret);
				if (ret == BRFATAL)
					sprintf(ME(mp), "Job ID %s: get volume failed for %s", mp->jobid, dmname);
				br_state(mp, envp);
				goto trylab;	/* longjmped out if fatal */
			}
			if (strcmp(dmname, label)) {	/* not what asked for */
				newlabel(mp, m_info, label, envp);
			}
			(void) brinvlbl (label);
		}
		else {
			if (!strcmp(fname, mp->dname)) {	/* already used */
				brlog("bknewvol: %s has been used",fname);
				sprintf(ME(mp), "Job ID %s: %s has been used", mp->jobid, fname);
				return(-1);
			}
		}
		(void) strcpy(fname, mp->dname);
	}
#ifdef TRACE
	if (isfile) {
		brlog("bknewvol: create %s ",fname);
	}
	else if (dmname != NULL) {
		brlog("bknewvol: dev %s vol=%s",fname,dmname);
	}
	else {
		brlog("bknewvol: dev %s ",fname);
	}
#endif
	return(isfile);
} /* bknewvol() */
