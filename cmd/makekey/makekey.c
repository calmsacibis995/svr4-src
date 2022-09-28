/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)makekey:makekey.c	1.3"

/*
 * You send it 10 bytes.
 * It sends you 13 bytes.
 * The transformation is expensive to perform
 * (a significant part of a second).
 */

char	*crypt();

main()
{
	char key[8];
	char salt[2];
	
	read(0, key, 8);
	read(0, salt, 2);
	write(1, crypt(key, salt), 13);
	return(0);
}
