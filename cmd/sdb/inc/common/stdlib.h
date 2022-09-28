/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/stdlib.h	1.1"
/*ident	"@(#)cfront:incl/stdlib.h	1.7"*/


#ifndef STDLIBH
#define STDLIBH

extern int     abort();
extern double  atof (const char*);
extern int     atoi (const char*);
extern long    atol (const char*);
extern char*   calloc (unsigned,unsigned);
extern void    exit (int);
extern void    free (char*);
extern char*   getenv (const char*);
extern char*   malloc (unsigned);
extern int     rand ();
extern char*   realloc (char*, unsigned);
extern void    srand  (unsigned);
extern double  strtod (const char*, char**);
extern long    strtol (const char*, char**, int);
extern unsigned long  strtoul(const char *, char **, int);
extern int     system (const char*);

#endif
