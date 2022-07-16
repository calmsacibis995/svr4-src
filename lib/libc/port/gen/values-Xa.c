/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/values-Xa.c	1.3"

#include "synonyms.h"
#include <math.h>

/* variables which differ depending on the
 * compilation mode
 *
 * ANSI conforming mode
 * This file is linked into the a.out immediately following
 * the startup routine if the -Xa compilation mode is selected
 */

 const enum version _lib_version = ansi_1;
