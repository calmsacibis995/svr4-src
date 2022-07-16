/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)//usr/src/uts/i386/master.d/timod/space.c.sl 1.1 4.0 12/08/90 57978 AT&T-USL"

#include "sys/types.h"
#include "sys/stream.h"
#include "sys/timod.h"

#include "config.h"	/* to collect tunable parameter below */

struct	tim_tim		tim_tim[NUMTIM] ;
int		tim_cnt = { NUMTIM } ;
