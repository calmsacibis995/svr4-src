/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/_locale.h	1.1.3.1"

#define LC_NAMELEN	15		/* maximum part name length (inc. \0) */
#define SZ_CTYPE	(257 + 257)	/* is* and to{upp,low}er tables */
#define SZ_CODESET	7		/* bytes for codeset information */
#define SZ_NUMERIC	2		/* bytes for numeric editing */
#define SZ_TOTAL	(SZ_CTYPE + SZ_CODESET)
#define NM_UNITS	0		/* index of decimal point character */
#define NM_THOUS	1		/* index of thousand's sep. character */

extern char _cur_locale[LC_ALL][LC_NAMELEN];
extern unsigned char __ctype[SZ_TOTAL];
extern unsigned char _numeric[SZ_NUMERIC];

char *_nativeloc(int);		/* trunc. name for category's "" locale */
char *_fullocale(const char *, const char *);	/* complete path */
int _set_tab(const char *, int);		/* fill __ctype[]  or _numeric[] */
