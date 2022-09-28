/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bnu:dtnamer.c	1.2.3.1"

/*
 *	Translate a channel number into the proper filename
 *	to access that Datakit channel's "dktty" device driver.
 */
#ifndef DIAL
	static char	SCCSID[] = "@(#)dtnamer.c	2.2+BNU  DKHOST 85/08/27";
#endif
/*
 *	COMMKIT(TM) Software - Datakit(R) VCS Interface Release 2.0 V1
 *			Copyright 1984 AT&T
 *			All Rights Reserved
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *     The copyright notice above does not evidence any actual
 *          or intended publication of such source code.
 */

#include "dk.h"

GLOBAL char *
dtnamer(chan)
{
	static char	dtname[12];

	sprintf(dtname, "/dev/dk%.3dt", chan);

	return(dtname);
}
