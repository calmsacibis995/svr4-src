/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/pf_ab.c	6.5"
# include	"../../hdr/defines.h"

void
pf_ab(s,pp,all)
char *s;
register struct pfile *pp;
int all;
{
	register char *p;
	register int i;
	extern char *Datep;
	char *xp, *sid_ab(), *fmalloc(), *strncpy();
	int	index(), fatal(), date_ab();
	unsigned	strlen();
	char stmp[MAXLINE];

	xp = p = stmp;
	copy(s,p);
	for (; *p; p++)
		if (*p == '\n') {
			*p = 0;
			break;
		}
	p = xp;
	p = sid_ab(p,&pp->pf_gsid);
	++p;
	p = sid_ab(p,&pp->pf_nsid);
	++p;
	i = index(p," ");
	pp->pf_user[0] = 0;
	if (((unsigned)i) < LOGSIZE) {
		strncpy(pp->pf_user,p,(unsigned) i);
		pp->pf_user[i] = 0;
	}
	else
		fatal("bad p-file format (co17)");
	p = p + i + 1;
	date_ab(p,&pp->pf_date);
	p = Datep;
	pp->pf_ilist = 0;
	pp->pf_elist = 0;
	pp->pf_cmrlist = 0;
	if (!all || !*p)
		return;
	p += 2;
	xp = fmalloc(size(p));
	copy(p,xp);
	p = xp;
	if (*p == 'i') {
		pp->pf_ilist = ++p;
		for (; *p; p++)
			if (*p == ' ') {
				*p++ = 0;
				p++;
				break;
			}
	}
	if (*p == 'x')
		{
		pp->pf_elist = ++p;
		for(;*p;p++)
			if(*p == ' ')
			{
				*p++ = 0;
				p++;
				break;
			}
	}
	if(*p == 'z')
		{
		pp->pf_cmrlist = ++p;
		}
}
