/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/methutil.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<setjmp.h>
#include	<method.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<brtoc.h>
#include	<signal.h>
#include	<string.h>
#include	<table.h>
#include	<errno.h>

extern int	brcancel();
extern int	brlog();
extern void	brhistory();
extern void	brsnddot();
extern int	brsuspend();
extern int	link();
extern void	*malloc();
extern int	unlink();

extern char	tocname[];
extern int	bklevels;
extern int	brstate;

void
do_history(mp, mhead, TC)
m_info_t	*mp;
media_info_t	*mhead;
int		TC;
{
	ENTRY	eptr = 0l;
	ENTRY	nptr = 0l;
	ENTRY	cur;
	ENTRY	prev;
	ENTRY	wrk;
	int	nv = 0;
	int	sav_toc = 0;
	int	is_toc = 0;
	int	disk_toc = 0;
	int	oldmax;
	int	nxt;
	int	towrite = 0;
	int	k;
	int	l;
	int	ret;
	int	tocstart;
	char	**labels;
	char	**savlab = NULL;
	char	lablist[1025];
	char	*null_toc = "", *fname1;
	char	toclink[PATH_MAX+1];		/* table of contents path */
	media_list_t	*ml;
	unsigned char	*fn;

	if (TC >= 0) {
		if ((eptr = TLgetentry(TC)) == NULL) {
			brlog("do_history: cannot allocate toc entry");
		}
		if ((nptr = TLgetentry(TC)) == NULL) {
			brlog("do_history: cannot allocate toc entry");
		}
	}
	if (mp->flags & tflag) {
		disk_toc = 1;
		sav_toc = 1;
	}
	else if (!(mp->flags & sflag)) {
		disk_toc = 1;
	}
	if (mp->br_type & IS_TOC)
		is_toc = 1;

	ml = mhead->first;

	while (ml != NULL) {
		nv++;
		ml = ml->next;
	}
	if (nv) {
		labels = (char **)malloc( nv * sizeof(char *));

		if ((savlab = labels) == NULL) {
			brlog("malloc failed for labels history: %s", SE);
		}
		else {
			ml = mhead->first;
			while(ml != NULL) {
				*labels++ = ml->label;
				ml = ml->next;
			}
		}
	}
	else {
		if ((mp->dtype == IS_FILE) || (mp->dtype == IS_DPART)) {
			nv = 1;
			savlab = &(mp->dname);
		}
		else {
			labels = NULL;
		}
	}
	cur = nptr;
	prev = eptr;

	for (nxt = 1, oldmax = 0; eptr && nptr ; nxt++) {
		BEGIN_CRITICAL_REGION;

		ret = TLread(TC, nxt, cur);

		END_CRITICAL_REGION;

		if (ret != TLOK)  {
			nxt = -1;
			goto last;
		}
		if ((fn = TLgetfield(TC, cur, TOC_VOL)) == NULL)
			continue;

		(void) sscanf((char *)fn, "%d", &tocstart);
		tocstart--;
last:
		if (towrite) {
			lablist[0] = '\0';

			if (oldmax < 0)
				oldmax = 0;

			for (k = oldmax; k <= tocstart; k++) {
				(void) strcat(lablist, savlab[k]);
				(void) strcat(lablist, ",");
			}
			if (l = strlen(lablist)) {
				lablist[l-1] = '\0';
			}
			(void) TLassign(TC, prev, TOC_VOL, lablist);
			(void) TLwrite(TC, towrite, prev);
		}
		wrk = cur;
		cur = prev;
		prev = wrk;

		towrite = nxt;
		oldmax = tocstart;

		if (nxt < 0)
			break;
	}
	if (TC >= 0)
		(void) TLsync(TC);

	brlog("size=%d nvol=%d tocflag=%d tocname=%s is_toc=%d",
	(int)(mp->blk_count), nv,sav_toc, sav_toc ? tocname : null_toc, is_toc);

	if (!(mp->flags & dflag)) { /* Do a history */
		if (is_toc) { /* -T ==> Do a TOC */
			(void) brhistory(mp->ofsname, mp->ofsdev, mp->bkdate,
				(int)(mp->blk_count), savlab, nv,
				 BR_IS_TMNAMES|BR_IS_OLD_ENTRY, mp->tocfname);
		}
		else { /* Do a real history */
			(void) brhistory(mp->ofsname, mp->ofsdev, mp->bkdate,
				(int)(mp->blk_count), savlab, nv,
				  mp->flags&tflag ? BR_ARCHIVE_TOC : NULL,
				 disk_toc ? tocname : null_toc);
		}
	}
	if (mp->flags & nflag) {
		if (mp->flags & tflag) {
			(void) strcpy(toclink, tocname);
			fname1 = strrchr(toclink, '/');

			if (fname1 == NULL) {
				brlog("tocname %s unknown format", tocname);
				fname1 = toclink;
				/* EXIT ? */
			}
			fname1++;
			*fname1 = 'P';

			if (link(tocname, toclink)) {
				brlog("link %s to %s failed %s",toclink,tocname,SE);
				/* EXIT ? */
			}
		}
		else {
			(void) unlink(tocname);
		}
	}
	if ((mp->flags & sflag) || is_toc) {
		(void) unlink(tocname);
	}
} /* do_history() */

void
br_state(mp, envp)
m_info_t	*mp;
jmp_buf		*envp;
{
	int	ret;

	switch(brstate) {
	case BR_SUSPEND:	/* free dev if any */
		if (ret = brsuspend()) {
#ifdef TRACE
			brlog("br_state brsuspend returned %d", ret);
#endif
			sprintf(ME(mp), "Job ID %s: brsuspend returned %d", mp->jobid, ret);
			longjmp(*envp, BRFAILED);
		}
		break;
	case BR_CANCEL:
		if (ret = brcancel()) {
#ifdef TRACE
			brlog("br_state brcancel returned %d ", ret);
#endif
			sprintf(ME(mp), "Job ID %s: brcancel returned %d", mp->jobid, ret);
			ret = BRFAILED;
		}
		else {
			ret = BRCANCELED;
		}
		sprintf(ME(mp), "Job ID %s: received cancel", mp->jobid);
		longjmp(*envp, ret);
		break;
	}
} /* br_state() */

void
newlabel(mp, m_info, label, envp)
m_info_t	*mp;
media_list_t	*m_info;
char		*label;
jmp_buf		*envp;
{
	char	*nl;

	brlog("newlabel old=%s  new=%s",m_info->label, label);

	nl = (char *) malloc((strlen(label) + 1));

	if (nl == NULL) {
		brlog("newlabel: malloc failed");
		sprintf(ME(mp), "Job ID %s: out of memory", mp->jobid);
		longjmp(*envp, 1);
	}
	(void) strcpy(nl, label);

	m_info->label = nl;
} /* newlabel() */

void
dots(writesize)
int	writesize;
{
	static long	bytesout = 0;
	static long	dots_out = 0;
	long		dots_owed;

	bytesout += writesize;
	dots_owed = (bytesout >> 9) / 100;

	while (dots_owed != dots_out) {
		(void) brsnddot();
		dots_out++;
	}
} /* dots() */
