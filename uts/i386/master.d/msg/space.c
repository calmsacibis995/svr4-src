/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:msg/space.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "sys/map.h"

#include "config.h"	/* to collect tunable parameters */

struct	map	msgmap[MSGMAP] ;
struct	msqid_ds	msgque[MSGMNI] ;
struct	msg	msgh[MSGTQL] ;
char	msglock[MSGMNI];
struct	msginfo	msginfo
		      ={MSGMAP,
			MSGMAX,
			MSGMNB,
			MSGMNI,
			MSGSSZ,
			MSGTQL,
			MSGSEG} ;
