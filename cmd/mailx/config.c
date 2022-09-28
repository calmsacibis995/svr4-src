/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:config.c	1.2.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 */

/*
 * This file contains definitions of network data used by mailx
 * when replying.  See also:  configdefs.h and optim.c
 */

/*
 * The subterfuge with CONFIGFILE is to keep cc from seeing the
 * external definitions in configdefs.h.
 */
#define	CONFIGFILE
#include "configdefs.h"

/*
 * Set of network separator characters.
 */
char	*metanet = "!^:%@.";

/*
 * Host table of "known" hosts.  See the comment in configdefs.h;
 * not all accessible hosts need be here (fortunately).
 */
struct netmach netmach[] = {
	EMPTY,		EMPTYID,	BN|AN,	/* Filled in dynamically */
	EMPTY,		EMPTYID,	BN|AN,	/* Filled in dynamically */
	0,		0,		0
};

/*
 * Table to convert from network separator code in address to network
 * bit map kind.  With this transformation, we can deal with more than
 * one character having the same meaning easily.
 */
struct ntypetab ntypetab[] = {
	'%',	AN,
	'@',	AN,
	':',	SN,
	'!',	BN,
	'^',	BN,
	0,	0
};
