/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/permiss.c	6.4"
# include	"../../hdr/defines.h"

static void	ck_lock();

void
finduser(pkt)
register struct packet *pkt;
{
	register char *p;
	char	*user, *logname();
	char	*strend(), *getline(), *repl();
	int	strcmp();
	unsigned	strlen();
	char groupid[6];
	int none;
	int ok_user;
	void	fmterr();

	none = 1;
	user = logname();
	sprintf(groupid,"%u",getgid());
	while ((p = getline(pkt)) != NULL && *p != CTLCHAR) {
		none = 0;
		ok_user = 1;
		repl(p,'\n','\0');	/* this is done for equal test below */
		if(*p == '!') {
			++p;
			ok_user = 0;
			}
		if (!pkt->p_user)
			if (equal(user,p) || equal(groupid,p))
				pkt->p_user = ok_user;
		*(strend(p)) = '\n';	/* repl \0 end of line w/ \n again */
	}
	if (none)
		pkt->p_user = 1;
	if (p == NULL || p[1] != EUSERNAM)
		fmterr(pkt);
}


char	*Sflags[NFLAGS];

void
doflags(pkt)
struct packet *pkt;
{
	register char *p;
	register int k;
	char *getline(), *fmalloc();
	unsigned	strlen();

	for (k = 0; k < NFLAGS; k++)
		Sflags[k] = 0;
	while ((p = getline(pkt)) != NULL && *p++ == CTLCHAR && *p++ == FLAG) {
		NONBLANK(p);
		k = *p++ - 'a';
		NONBLANK(p);
		Sflags[k] = fmalloc(size(p));
		copy(p,Sflags[k]);
		for (p = Sflags[k]; *p++ != '\n'; )
			;
		*--p = 0;
	}
}

void
permiss(pkt)
register struct packet *pkt;
{
	extern char *Sflags[];
	register char *p;
	register int n;
	int	patoi(), fatal(), strcmp();
	char	*repl();
	unsigned	strlen();
	void	fmterr();

	if (!pkt->p_user)
		fatal("not authorized to make deltas (co14)");
	if (p = Sflags[FLORFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) < (n = patoi(p))) {
			sprintf(Error,"release %d < %d (floor) (co15)",
				pkt->p_reqsid.s_rel,n);
			fatal(Error);
		}
	}
	if (p = Sflags[CEILFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) > (n = patoi(p))) {
			sprintf(Error,"release %d > %d (ceiling) (co16)",
				pkt->p_reqsid.s_rel,n);
			fatal(Error);
		}
	}
	/*
	check to see if the file or any particular release is
	locked against editing. (Only if the `l' flag is set)
	*/
	if ((p = Sflags[LOCKFLAG - 'a']))
		ck_lock(p,pkt);
}



static char	l_str[]    =    "SCCS file locked against editing (co23)";

static void
ck_lock(p,pkt)
register char *p;
register struct packet *pkt;
{
	int l_rel;
	int locked;

	locked = 0;
	if (*p == 'a')
		locked++;
	else while(*p) {
		p = satoi(p,&l_rel);
		++p;
		if (l_rel == pkt->p_gotsid.s_rel || l_rel == pkt->p_reqsid.s_rel) {
			locked++;
			sprintf(l_str,"release `%d' locked against editing (co23)",
				l_rel);
			break;
		}
	}
	if (locked)
		fatal(l_str);
}
