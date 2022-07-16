/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:sysmsg/space.c	1.3"

#include "sys/termio.h"
#include "sys/sysmsg.h"
#include "config.h"

struct smsg_flags smsg_flags = 
{
	CMF,	/* console message flag - tunable */
	CMF,	/* dynamic console message flag - tunable */
	RCMF,	/* remote console message flag - tunable */
	RCMF,	/* dynamic remote console message flag - tunable */
	0,	/* default for current alternate console setting */
	1,	/* default for remote console setting */
	0,	/* default for CMOS alternate console setting */
	B9600,	/* default alternate console baud rate */
	B1200	/* default remote console baud rate */
};

int com2cons = COM2CONS;
