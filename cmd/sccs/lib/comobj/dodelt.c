/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/dodelt.c	6.5"
#include	"../../hdr/defines.h"

# define ONEYEAR 31536000L


long	Timenow;

char	Pgmr[LOGSIZE];	/* for rmdel & chghist (rmchg) */
int	First_esc;
int	First_cmt;
int	CDid_mrs;		/* for chghist to check MRs */

struct idel *
dodelt(pkt,statp,sidp,type)
register struct packet *pkt;
struct stats *statp;
struct sid *sidp;
char type;
{
	extern  char	had[26];
	extern	char	*satoi();
	char *c, *getline(), *fmalloc();
	struct deltab dt;
	register struct idel *rdp;
	extern char *Sflags[];
	int n, founddel;
	long time();
	void	fmterr(), fredck(), escdodelt(), stats_ab(), doixg();
	int	eqsid();
	char	getadel();
	register char *p;

	pkt->p_idel = 0;
	founddel = 0;

	(void) time(&Timenow);
	stats_ab(pkt,statp);
	while (getadel(pkt,&dt) == BDELTAB) {
		if (pkt->p_idel == 0) {
			if (Timenow < dt.d_datetime)
				fprintf(stderr,"Time stamp later than current clock time (co10)\n");
			pkt->p_idel = (struct idel *)
					fmalloc((unsigned) (n=((dt.d_serial+1)*
					sizeof(*pkt->p_idel))));
			zero((char *) pkt->p_idel,n);
			pkt->p_apply = (struct apply *)
					fmalloc((unsigned) (n=((dt.d_serial+1)*
					sizeof(*pkt->p_apply))));
			zero((char *) pkt->p_apply,n);
			pkt->p_idel->i_pred = dt.d_serial;
		}
		if (dt.d_type == 'D') {
			if (sidp && eqsid(&dt.d_sid,sidp)) {
				copy(dt.d_pgmr,Pgmr);	/* for rmchg */
				zero((char *) sidp,sizeof(*sidp));
				founddel = 1;
				First_esc = 1;
				First_cmt = 1;
				CDid_mrs = 0;
				for (p = pkt->p_line; *p && *p != 'D'; p++)
					;
				if (*p)
					*p = type;
			}
			else
				First_esc = founddel = First_cmt = 0;
			pkt->p_maxr = max(pkt->p_maxr,dt.d_sid.s_rel);
			rdp = &pkt->p_idel[dt.d_serial];
			rdp->i_sid.s_rel = dt.d_sid.s_rel;
			rdp->i_sid.s_lev = dt.d_sid.s_lev;
			rdp->i_sid.s_br = dt.d_sid.s_br;
			rdp->i_sid.s_seq = dt.d_sid.s_seq;
			rdp->i_pred = dt.d_pred;
			rdp->i_datetime = dt.d_datetime;
		}
		while ((c = getline(pkt)) != NULL)
			if (pkt->p_line[0] != CTLCHAR)
				break;
			else {
				switch (pkt->p_line[1]) {
				case EDELTAB:
					break;
				case COMMENTS:
				case MRNUM:
					if (founddel)
					{
						escdodelt(pkt);
						if(type == 'R' && (had[('z' - 'a')]) && pkt->p_line[1] == MRNUM )
						{
							fredck(pkt);
						}
					}
				continue;
				default:
					fmterr(pkt);
				case INCLUDE:
				case EXCLUDE:
				case IGNORE:
					if (dt.d_type == 'D') {
						doixg(pkt->p_line,rdp->i_ixg);
					}
					continue;
				}
				break;
			}
		if (c == NULL || pkt->p_line[0] != CTLCHAR || getline(pkt) == NULL)
			fmterr(pkt);
		if (pkt->p_line[0] != CTLCHAR || pkt->p_line[1] != STATS)
			break;
	}
	return(pkt->p_idel);
}

static char
getadel(pkt,dt)
register struct packet *pkt;
register struct deltab *dt;
{
	void	fmterr();
	char	del_ab();
	char	*getline();
	if (getline(pkt) == NULL)
		fmterr(pkt);
	return(del_ab(pkt->p_line,dt,pkt));
}

static void
doixg(p,ixgp)
char *p;
struct ixg *ixgp;
{
	int *v, *ip;
	int type, cnt, i;
	struct ixg *cur, *prev;
	int xtmp[MAXLINE];
	char	*fmalloc();

	v = ip = xtmp;
	++p;
	type = *p++;
	NONBLANK(p);
	while (numeric(*p)) {
		p = satoi(p,ip++);
		NONBLANK(p);
	}
	cnt = ip - v;
	for (cur = ixgp; cur = (prev = cur)->i_next; )
		;
	prev->i_next = cur = (struct ixg *) fmalloc((unsigned) (sizeof(*cur) +
						(cnt-1)*sizeof(cur->i_ser[0])));
	cur->i_next = 0;
	cur->i_type = (char) type;
	cur->i_cnt = (char) cnt;
	for (i=0; cnt>0; --cnt)
		cur->i_ser[i++] = *v++;
}
