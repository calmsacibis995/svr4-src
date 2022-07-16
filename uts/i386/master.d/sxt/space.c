/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:sxt/space.c	1.3"

#include "sys/types.h"
#include "sys/tty.h"
#include "sys/sxt.h"

#define LINKSIZE  (sizeof(struct Link) + sizeof(struct Channel) * (MAXPCHAN-1))

#include "config.h"	/* for overriding above parameter */

struct  tty     sxt_tty[NUMSXT] ;
char    sxt_buf[NUMSXT * LINKSIZE] ;
int		sxt_cnt ={NUMSXT} ;
