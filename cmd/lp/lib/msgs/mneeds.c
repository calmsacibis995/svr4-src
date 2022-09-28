/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/msgs/mneeds.c	1.4.2.1"
/* LINTLIBRARY */

/**
 ** mneeds() -  RETURN NUMBER OF FILE DESCRIPTORS NEEDED BY mopen()
 **/

int mneeds ( )
{
    /*
     * This is the expected number of file descriptors needed.
     */
    return (4);
}
