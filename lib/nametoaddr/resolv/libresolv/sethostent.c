/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libresolv/sethostent.c	1.1.1.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#include <sys/types.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include "res.h"

/* TWG name of same function */
sethostent(stayopen)
{
	return(_rs_sethostent(stayopen));
}

_rs_sethostent(stayopen)
{
	if (stayopen)
		_res.options |= RES_STAYOPEN | RES_USEVC;
}

/* TWG name of same function */
endhostent()
{
	return(_rs_endhostent());
}

_rs_endhostent()
{
	_res.options &= ~(RES_STAYOPEN | RES_USEVC);
	_rs__res_close();
}

/*  TWG name of same function */
sethostfile(name)
char *name;
{
	return(_rs_sethostfile(name));
}

_rs_sethostfile(name)
char *name;
{
#ifdef lint
name = name;
#endif
}
