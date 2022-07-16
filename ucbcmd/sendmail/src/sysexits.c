/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:src/sysexits.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
*/

# include <sysexits.h>
# include "useful.h"

SCCSID(@(#)sysexits.c 1.7 88/02/08 SMI); /* from UCB 5.2 6/7/85 */

/*
**  SYSEXITS.C -- error messages corresponding to sysexits.h
*/

char	*SysExMsg[] =
{
	/* 64 USAGE */		"500 Bad usage",
	/* 65 DATAERR */	"501 Data format error",
	/* 66 NOINPUT */	"550 Cannot open input",
	/* 67 NOUSER */		"550 User unknown",
	/* 68 NOHOST */		"550 Host unknown",
	/* 69 UNAVAILABLE */	"554 Service unavailable",
	/* 70 SOFTWARE */	"554 Internal error",
	/* 71 OSERR */		"451 Operating system error",
	/* 72 OSFILE */		"554 System file missing",
	/* 73 CANTCREAT */	"550 Can't create output",
	/* 74 IOERR */		"451 I/O error",
	/* 75 TEMPFAIL */	"250 Deferred",
	/* 76 PROTOCOL */	"554 Remote protocol error",
	/* 77 NOPERM */		"550 Insufficient permission",
};

int	N_SysEx = sizeof SysExMsg / sizeof SysExMsg[0];
/*
**  STATSTRING -- return string corresponding to an error status
**
**	Parameters:
**		stat -- the status to decode.
**
**	Returns:
**		The string corresponding to that status
**
**	Side Effects:
**		none.
*/

char *
statstring(stat)
	int stat;
{
	static char ebuf[100];

	stat -= EX__BASE;
	if (stat < 0 || stat >= N_SysEx)
	{
		(void) sprintf(ebuf, "554 Unknown status %d", stat + EX__BASE);
		return (ebuf);
	}

	return (SysExMsg[stat]);
}
