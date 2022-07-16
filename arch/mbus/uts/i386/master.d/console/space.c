/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/console/space.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"		/* definitins for console switch */
#include "config.h"		/* etc/conf/cf.d/config.h */
#include "sys/at_ansi.h"	/* hack to get AT sepcific structures */

/* XXX Fix these references and DELETE THIS FILE XXX */
struct attrmask kdstrmap;
struct attrmask kb_attrmask;
struct attrmask nattrmsks;
