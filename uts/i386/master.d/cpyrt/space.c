/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:cpyrt/space.c	1.3"

/*
 * OEM COPYRIGHTS "DRIVER"
 */

/*
 * The following array is to be used by any drivers which need to put
 * OEM copyright notices out on boot up.  It is initialized to all 0's
 * (NULL).  A driver which needs to have a copyright string written to
 * the console during bootup should look (in the init routine) for the
 * first empty slot and put in a pointer at a static string which 'main'
 * will write out (using printf "%s\n\n") after the AT&T copyright.  If
 * there is more than one driver from a given OEM, the init code should
 * check the filled slots and compare the copyright strings so a given
 * message only appears once.  If somebody overflows the array, chaos
 * will probably reign.  Caveat coder.
 */

#include "config.h"


int     max_copyrights = NCPYRIGHT;
char    *oem_copyrights[NCPYRIGHT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*      This is an example of how to use the cpyrt driver to insert
/*      your copyright notice:
/*
/*      int index;
/*      char *msgptr = "YOUR MESSAGE HERE";
/*
/*        for (index = 0; index < max_copyrights; index++) {
/*                if (!oem_copyrights[index]) { /* if open slot put in msg */
/*                        oem_copyrights[index] = msgptr;
/*                        break;
/*                }
/*                if (!strcmp(msgptr, oem_copyrights[index]))
/*                      break;  /*    msg already there  */
/*        }
*/


