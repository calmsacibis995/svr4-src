/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/search.h	1.1"
/*ident	"@(#)cfront:incl/search.h	1.5"*/

#ifndef SEARCHH
#define SEARCHH

/* BSEARCH(3C) */
extern char *bsearch(const char*, char*, unsigned, int, int);

/* HSEARCH(3C) */
typedef struct entry { char *key, *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

extern ENTRY *hsearch (ENTRY,ACTION);
extern int hcreate(unsigned);
extern void hdestroy ();

/* LSEARCH(3C) */                       
#ifndef PFSEEN
#define PFSEEN
typedef int (*PF) ();
#endif

extern char *lsearch (const char*,  char*, unsigned*, unsigned, PF);
extern char *lfind (const char*, char*, unsigned*, unsigned, PF);

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

extern char *tsearch (const char*, char**, int);
extern char *tfind (const char*, char**, int);
extern char *tdelete (const char*, char**, int);
extern void twalk(char*,void (*)());

#endif
