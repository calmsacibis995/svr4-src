/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsstatus.d/admin_auth.c	1.2.2.1"

#define TRUE	1
#define FALSE	0

/* Routine determines whether user is an authorized administrator.  */
/* It returns TRUE if user is an administrator, FALSE otherwise.    */
/* THIS ROUTINE MUST BE REPLACED WHEN THE AUTHORIZATION SOFTWARE IS */
/* AVAILABLE. */
admin_auth()
{
	unsigned short getuid();

	return ( getuid() == (unsigned short)0 );
}
