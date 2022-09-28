/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/CONFIG/alphanum.c	1.1"

#define isalphanum(k) (((k >= 'a') && (k <= 'z')) || ((k >= 'A') && (k <= 'Z')) || ((k >= '0') && (k <= '9')) || (k == '_'))

main(argc,argv)
int argc;
char *argv[];
{
 char *p = 0, c;
 p = *(++argv);
 while (c = *p) {
      if (! (isalphanum(c))) exit(1); 
      p++;
 }
exit(0);
}
