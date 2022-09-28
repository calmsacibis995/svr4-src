/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_get_err.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_get_err.c	3.3	LCC);	/* Modified: 16:29:34 2/26/90 */

/****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

#include	"pci_types.h"
#include	<sys/stat.h>
#include	<time.h>
#include	<errno.h>
#include	<string.h>


/*				External Routines			*/


extern  void ftslash();         /* Translates frontslash to UNIX backslash */

extern  int bdate();            /* Converts date to MS-DOS format */
extern  int btime();            /* Converts time into MS-DOS format */
extern  int swap_in();          /* Causes a virtual descriptor to be paged in */
extern  int attribute();        /* Set attribute bits in output frame */

extern	char *memory();         /* Allocate a dynamic buffer */

#ifndef	SYS5
extern char *mktemp();          /* Create a unique temporary filename */
#else	SYS5
extern char *tempnam();		/* Create a unique temporary filename */
#endif	SYS5

extern  struct tm *localtime(); /* Load file date into tm structure */



/*                      Imported Variables and Structures               */


extern  int errno;              /* Contains error code from system calls */

extern char err_class, err_action, err_locus;
extern int  err_code;


void
pci_get_ext_err(addr)
struct output *addr;
{
	addr->hdr.res = err_code;
	addr->text[0] = err_class;
	addr->text[1] = err_action;
	addr->text[2] = err_locus;
/*
	This has been "fixed" on the DOS side. You do not to need this 
	statement.  It should be fixed better later.
	*((int*)&addr->text[3]) = err_code;
*/

	addr->hdr.t_cnt = 3;
}
