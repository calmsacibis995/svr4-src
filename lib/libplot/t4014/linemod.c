/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libplot:t4014/linemod.c	1.3"
linemod(s)
char *s;
{
	char c;
	putch(033);
	switch(s[0]){
	case 'l':	
		c = 'd';
		break;
	case 'd':	
		if(s[3] != 'd')c='a';
		else c='b';
		break;
	case 's':
		if(s[5] != '\0')c='c';
		else c='`';
	}
	putch(c);
}
