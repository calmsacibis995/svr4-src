/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/dumpolp.c	1.2.2.1"

#include "stdio.h"
#include "sys/types.h"

#include "lp.h"

#define	QSTATUS	"qstatus"
#define	PSTATUS	"pstatus"

#define DESTMAX	14

#define Q_RSIZE	81

struct qstat {
	char	q_dest[DESTMAX+1];	/* destination */
	short	q_accept;		/* 1 iff lp accepting requests */
	time_t	q_date;
	char	q_reason[Q_RSIZE];	/* reason for reject */
};

#define	P_RSIZE	81

struct pstat {
	char	p_dest[DESTMAX+1];	/* destination name of printer */
	int	p_pid;
	char	p_rdest[DESTMAX+1];
	int	p_seqno;
	time_t	p_date;
	char	p_reason[P_RSIZE];	/* reason for being disable */
	short	p_flags;
};

#define	P_ENAB	1		/* printer enabled */

extern char		*getenv();

/**
 ** main()
 **/

int			main ()
{
	char			*spooldir,
				*pstatus,
				*qstatus;

	struct pstat		pbuf;

	struct qstat		qbuf;

	register struct pstat	*p	= &pbuf;

	register struct qstat	*q	= &qbuf;

	register FILE		*fp;


	if (!(spooldir = getenv("SPOOLDIR")))
		spooldir = SPOOLDIR;

	pstatus = makepath(spooldir, PSTATUS, (char *)0);
	if ((fp = fopen(pstatus, "r"))) {
		while (fread((char *)p, sizeof(*p), 1, fp) == 1) {
			printf (
				"PRTR %*.*s %c %.*s\n",
				DESTMAX,
				DESTMAX,
				p->p_dest,
				(p->p_flags & P_ENAB? 'E' : 'D'),
				P_RSIZE,
				p->p_reason
			);
		}
		fclose (fp);
	}

	qstatus = makepath(spooldir, QSTATUS, (char *)0);
	if ((fp = fopen(qstatus, "r"))) {
		while (fread((char *)q, sizeof(*q), 1, fp) == 1) {
			printf (
				"DEST %*.*s %c %.*s\n",
				DESTMAX,
				DESTMAX,
				q->q_dest,
				(q->q_accept? 'A' : 'R'),
				Q_RSIZE,
				q->q_reason
			);
		}
		fclose (fp);
	}
}
