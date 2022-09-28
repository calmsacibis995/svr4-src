/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/setup.c	6.3"
# include	"../../hdr/defines.h"

static void	ixgsetup();

void
setup(pkt,serial)
register struct packet *pkt;
int serial;
{
	register int n;
	register struct apply *rap;
	int	first_app   =   1;
	void	fmterr(), condset();

	pkt->p_apply[serial].a_inline = 1;
	for (n = maxser(pkt); n; n--) {
		rap = &pkt->p_apply[n];
		if (rap->a_inline) {
			if (n != 1 && pkt->p_idel[n].i_pred == 0)
				fmterr(pkt);
			pkt->p_apply[pkt->p_idel[n].i_pred].a_inline = 1;
			if (pkt->p_idel[n].i_datetime > pkt->p_cutoff)
				condset(rap,NOAPPLY,CUTOFF);
			else {
				if (first_app)
					pkt->p_gotsid = pkt->p_idel[n].i_sid;
				first_app = 0;
				condset(rap,APPLY,SX_EMPTY);
			}
		}
		else
			condset(rap,NOAPPLY,SX_EMPTY);
		if (rap->a_code == APPLY) {
/*
** LINT complains about this, but it only works with the ampersand.
*/
			ixgsetup(pkt->p_apply,&(pkt->p_idel[n].i_ixg));
		}
	}
}


static void
ixgsetup(ap,ixgp)
struct apply *ap;
struct ixg *ixgp;
{
	int n;
	int code, reason;
	register int *ip;
	register struct ixg *cur;
	void	condset();

	for (cur = ixgp; cur = cur->i_next; ) {
		switch (cur->i_type) {

		case INCLUDE:
			code = APPLY;
			reason = INCL;
			break;
		case EXCLUDE:
			code = NOAPPLY;
			reason = EXCL;
			break;
		case IGNORE:
			code = SX_EMPTY;
			reason = IGNR;
			break;
		}
		ip = cur->i_ser;
		for (n = cur->i_cnt; n; n--)
			condset(&ap[*ip++],code,reason);
	}
}


void
condset(ap,code,reason)
register struct apply *ap;
int code, reason;
{
	if (code == SX_EMPTY)
		ap->a_reason |= reason;
	else if (ap->a_code == SX_EMPTY) {
		ap->a_code = code;
		ap->a_reason |= reason;
	}
}
