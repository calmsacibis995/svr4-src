/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/values-Xt.c	1.3"

#include "synonyms.h"
#include <math.h>

/* variables which differ depending on the
 * compilation mode
 *
 * C Issue 4.2 compatibility mode
 * This file is linked into the a.out by default if
 * no compilation mode was specified or if the -Xt option
 * was specified - the linking occurs if there is an unresolved
 * reference to _lib_version
 */

 const enum version _lib_version = c_issue_4;
