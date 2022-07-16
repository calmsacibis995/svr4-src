/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:cpyrt.c	1.3"

/*
 * OEM COPYRIGHTS "DRIVER"
 */

/*
 * The following array is to be used by any drivers which need to put
 * OEM copyright notices out on boot up.  It is initialized to all 0's
 * (NULL).  A driver which needs to have a copyright string  written to
 * the console during bootup should look (in the init routine) for the
 * first empty slot and put in a pointer at a static string which
 * cpyrtstart will write out (using printf "%s\n\n"). If
 * there is more than one driver from a given OEM, the init code should
 * check the filled slots and compare the copyright strings so a given
 * message only appears once.  If somebody overflows the array, chaos
 * will probably reign.  Caveat coder.
 */

extern int     max_copyrights;
extern char    *oem_copyrights[];

/* write out any OEM driver copyright notices */

cpyrtstart()
{
int     i;
for (i=0; i<max_copyrights; i++)
	{
	if (!oem_copyrights[i]) break;
	printf("%s\n\n",oem_copyrights[i]);
	}
}

