/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)cmd-streams:kmacct/kmaprint.c	1.1"

#include <stdio.h>

main()
{
unsigned int address;
char name[80];

/* text in first 2 lines */
while(scanf("%u %s",&address,&name[0]) != EOF)
	printf("0x%8x %s\n",address,name);
}
