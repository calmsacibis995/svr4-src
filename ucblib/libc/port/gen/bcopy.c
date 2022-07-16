/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/bcopy.c	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#if !defined(lint) && defined(SCCSIDS)
static	char sccsid[] = "@(#)bcopy.c 1.5 88/02/08 SMI";
#endif

/*
 * Copy s1 to s2, always copy n bytes.
 * For overlapped copies it does the right thing.
 */
void
bcopy(s1, s2, len)
	register char *s1, *s2;
	int len;
{
	register int n;

	if ((n = len) <= 0)
		return;

	if ((s1 < s2) && (n > abs(s1 - s2))) {		/* overlapped */
		s1 += (n - 1);
		s2 += (n - 1);
		do
			*s2-- = *s1--;
		while (--n);
	} else {					/* normal */
		 do
			*s2++ = *s1++;
		while (--n);
	}
}
