/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SEARCH_H
#define _SEARCH_H

#ident	"@(#)head:search.h	1.3.1.11"

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned	size_t;
#endif

/* HSEARCH(3C) */
typedef enum { FIND, ENTER } ACTION;

struct qelem {
	struct qelem	*q_forw;
	struct qelem	*q_back;
};

#if defined(__STDC__)
typedef struct entry { char *key; void *data; } ENTRY;
int hcreate(size_t);
void hdestroy(void);
ENTRY *hsearch(ENTRY, ACTION);

void insque(struct qelem *, struct qelem *);
void remque(struct qelem *);
#else
typedef struct entry { char *key, *data; } ENTRY;
int hcreate();
void hdestroy();
ENTRY *hsearch();
void insque();
void remque();
#endif

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

#if defined(__STDC__)
void *tdelete(const void *, void **, int (*)(const void *, const void *)); 
void *tfind(const void *, void *const *, int (*)(const void *, const void *));
void *tsearch(const void *, void **, int (*)(const void *, const void *));
void twalk(void *, void (*)(void *, VISIT, int));
#else
char *tdelete();
char *tfind();
char *tsearch();
void twalk();
#endif

#if defined(__STDC__)

/* BSEARCH(3C) */
void *bsearch(const void *, const void *, size_t, size_t,
	    int (*)(const void *, const void *));

/* LSEARCH(3C) */
void *lfind(const void *, const void *, size_t *, size_t, 
	    int (*)(const void *, const void *));
void *lsearch(const void *, void *, size_t *, size_t,
	    int (*)(const void *, const void *));
#else
char *bsearch();
char *lfind();
char *lsearch();
#endif

#endif 	/* _SEARCH_H */
