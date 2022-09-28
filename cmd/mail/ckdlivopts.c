/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:ckdlivopts.c	1.6.3.1"
/*
    NAME
	ckdlivopts - check delivery notification options

    SYNOPSIS
	int ckdlivopts(int tcopy_hdr, int *svopts)

    DESCRIPTION
	Check if delivery notification requested for message being
	processed. Returns specified options as combined from H_DEFOPTS,
	H_TROPTS, & H_TCOPY lines.

	(Positive notification options)
		001 ==> /delivery requested
		002 ==> /nodelivery requested
	(Negative notification options)
		010 ==> /report requested
		020 ==> /return requested
		040 ==> /ignore requested

	Combinations are expected, i.e. - 011 ==> /delivery/report
	If not specified, the assumed defaults are /nodelivery/return (rc=022)

	The options discovered in the header are stored into svopts.
 */
#include "mail.h"

static void getopts();
static void mergeopts();

struct dlvopts {
	int	deliv;
	int	nodeliv;
	int	rept;
	int	rtrn;
	int	ign;
};

int ckdlivopts(tcopy_hdr, svopts)
int	tcopy_hdr;
int	*svopts;
{
	static char	pn[] = "ckdlivopts";
	struct	hdrs	*hp;
	struct dlvopts	toopts, tropts, defopts;
	int		rc;

	/* already done this once. no need to repeat..... */
	if (svopts && *svopts != 0) {
		Dout(pn, 0, "*svopts = o%o\n", *svopts);
		return (*svopts);
	}
	memset((char *)&defopts, 0, sizeof(struct dlvopts));
	if ((hp = hdrlines[H_DEFOPTS].head) != (struct hdrs *)NULL) {
		Dout(pn, 3, "H_DEFOPTS line = '%s'\n", hp->value);
		getopts(hp->value, &defopts);
	}
	memset((char *)&tropts, 0, sizeof(struct dlvopts));
	if ((hp = hdrlines[H_TROPTS].head) != (struct hdrs *)NULL) {
		Dout(pn, 3, "H_TROPTS line = '%s'\n", hp->value);
		getopts(hp->value, &tropts);
	}
	memset((char *)&toopts, 0, sizeof(struct dlvopts));
	if ((hp = hdrlines[tcopy_hdr].head) != (struct hdrs *)NULL) {
		Dout(pn, 3,"H_TCOPY line = '%s'\n", hp->value);
		getopts(hp->value, &toopts);
	}
	/* Combine options from different header lines. Precedence is */
	/* toopts --> tropts --> defopts. Results left in defopts */
	mergeopts(&tropts,&defopts);
	mergeopts(&toopts,&defopts);

	if (defopts.deliv)	rc = DELIVERY;
	else			rc = NODELIVERY;

	if (defopts.rtrn)	rc += RETURN;
	else if (defopts.rept)	rc += REPORT;
	else if (defopts.ign)	rc += IGNORE;
	else			rc += RETURN;

	Dout(pn, 0,"returning = o%o\n", rc);
	if (svopts)
		*svopts = rc;
	return (rc);
}

/*
 * Pick transport options off of header line.
 * If conflicting options found, use MOST demanding; i.e. - /delivery/return.
 */
static void getopts(s, optr)
register char	*s;
register struct dlvopts *optr;
{
	register char	*op;

	for (op = strchr (s, '/'); op++; op = strchr(op, '/')) {
		if (casncmp(op, "delivery", 7) == 0) {
			optr->deliv = 1;
			optr->nodeliv = 0;
		} else if (casncmp(op, "nodelivery", 10) == 0) {
			if (optr->deliv == 0) {
				optr->nodeliv = 1;
			}
		} else if (casncmp(op, "report", 6) == 0) {
			optr->ign = 0;
			if (optr->rtrn == 0) {
				optr->rept = 1;
			}
		} else if (casncmp(op, "return", 6) == 0) {
			optr->rtrn = 1;
			optr->rept = optr->ign = 0;
		} else if (casncmp(op, "ignore", 6) == 0) {
			optr->rept = 0;
			if (optr->rtrn == 0) {
				optr->ign = 1;
			}
		}
	}
}

/*
 * Merge options between 2 sets. Higher set has precedence.
 * Results left in lower set.
 */
static void mergeopts(higher, lower)
register struct dlvopts *higher, *lower;
{
	if (higher->deliv == 1) {
		lower->deliv = 1;
		lower->nodeliv = 0;
	}
	if (higher->nodeliv == 1) {
		lower->nodeliv = 1;
		lower->deliv = 0;
	}
	if (higher->rept == 1) {
		lower->rept = 1;
		lower->rtrn = lower->ign = 0;
	}
	if (higher->rtrn == 1) {
		lower->rtrn = 1;
		lower->rept = lower->ign = 0;
	}
	if (higher->ign == 1) {
		lower->ign = 1;
		lower->rept = lower->rtrn = 0;
	}
}
