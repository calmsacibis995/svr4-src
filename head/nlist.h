/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:nlist.h	1.8.2.4"

#ifndef _NLIST_H
#define _NLIST_H

struct nlist
{
        char            *n_name;        /* symbol name */
        long            n_value;        /* value of symbol */
        short           n_scnum;        /* section number */
        unsigned short  n_type;         /* type and derived type */
        char            n_sclass;       /* storage class */
        char            n_numaux;       /* number of aux. entries */
};

#if defined(__STDC__)
extern int nlist(const char *, struct nlist *);
#endif  /* __STDC__ */

#endif 	/* _NLIST_H */
