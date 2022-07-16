/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:malloc.h	1.7"

#ifndef _MALLOC_H
#define _MALLOC_H

/*
	Constants defining mallopt operations
*/
#define M_MXFAST	1	/* set size of blocks to be fast */
#define M_NLBLKS	2	/* set number of block in a holding block */
#define M_GRAIN		3	/* set number of sizes mapped to one, for
				   small blocks */
#define M_KEEP		4	/* retain contents of block after a free until
				   another allocation */
/*
	structure filled by 
*/
struct mallinfo  {
	int arena;	/* total space in arena */
	int ordblks;	/* number of ordinary blocks */
	int smblks;	/* number of small blocks */
	int hblks;	/* number of holding blocks */
	int hblkhd;	/* space in holding block headers */
	int usmblks;	/* space in small blocks in use */
	int fsmblks;	/* space in free small blocks */
	int uordblks;	/* space in ordinary blocks in use */
	int fordblks;	/* space in free ordinary blocks */
	int keepcost;	/* cost of enabling keep option */
};	

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned        size_t;
#endif

#if defined(__STDC__)

void *malloc(size_t);
void free(void *);
void *realloc(void *, size_t);
int mallopt(int, int);
struct mallinfo mallinfo(void);
void *calloc(size_t, size_t);

#else

char *malloc();
void free();
char *realloc();
int mallopt();
struct mallinfo mallinfo();
char *calloc();

#endif	/* __STDC__ */

#endif 	/* _MALLOC_H */
