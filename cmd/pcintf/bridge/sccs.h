/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/sccs.h	1.1"
/* SCCSID(@(#)sccs.h	3.8	LCC);	/* Modified: 16:30:15 11/17/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifndef _SCCS_
#define _SCCS_
#define H_SCCSID(arg)
#define LCC_SCCSID(arg)

#ifdef JANUS
#       include "system.h"
#endif /* JANUS */

#if	defined(lint) || defined(NOSCCSIDS)
#	define	SCCSID(arg)
#	define	SCCSIDPUFF(arg)
#elif	defined(__STDC__)
#	define SCCSID(arg)		static char Sccsid[] = #arg
#	define SCCSIDPUFF(arg)		static char Sccsidpuff[] = #arg
#else
#	define SCCSID(arg)		static char Sccsid[] = "arg"
#	define SCCSIDPUFF(arg)		static char Sccsidpuff[] = "arg"
#endif

#endif /* _SCCS_ */
