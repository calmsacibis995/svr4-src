/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)//usr/src/uts/i386/master.d/log/space.c.sl 1.1 4.0 12/08/90 8433 AT&T-USL"

#include "sys/types.h"
#include "sys/stream.h"
#include "sys/log.h"

#include "config.h"	/* to collect tunable parameters */

int	log_cnt ={ NLOG+6 } ;
struct	log	log_log[NLOG+6] ;
