/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/fmalloc.c	6.3"

/*
	The functions is this file replace xalloc-xfree-xfreeall from
	the PW library.

	Xalloc allocated words, not bytes, so this adjustment is made
	here.  This inconsistency should eventually be cleaned-up in the
	other source, i.e. each request for memory should be in bytes.

	These functions are complicated by the fact that libc has no
	equivalent to ffreeall.  This requires that pointers to allocated
	arrays be stored here.  If malloc ever has a freeall associated with
	it, most of this code can be discarded.
*/

#define LCHUNK	100
#define NULL	0

static unsigned	ptrcnt = 0;
static unsigned	listsize = 0;
static char	**ptrlist = NULL;
void	free();

char *
fmalloc(asize)
unsigned asize;
{
	char *ptr;

	int	fatal();

	if (listsize == 0) {
		listsize = LCHUNK;
		if ((ptrlist = (char **)malloc(sizeof(char *)*listsize)) == NULL)
			fatal("OUT OF SPACE (ut9)");
	}
	if (ptrcnt >= listsize) {
		listsize += LCHUNK;
		if ((ptrlist = (char **)realloc((char *)ptrlist,
					sizeof(char *)*listsize)) == NULL)
			fatal("OUT OF SPACE (ut9)");
	}

	if ((ptr = (char *) malloc(sizeof(int)*asize)) == NULL)
		fatal("OUT OF SPACE (ut9)");
	else
		ptrlist[ptrcnt++] = ptr;
	return(ptr);
}

void
ffree(aptr)
char *aptr;
{
	register unsigned cnt;

	cnt = ptrcnt;
	while (cnt)
		if (aptr == ptrlist[--cnt]) {
			free(aptr);
			if (cnt == ptrcnt - 1)
				--ptrcnt;
			else
				ptrlist[cnt] = NULL;
			return;
		}
	fatal("ffree: Pointer not pointing to allocated area");
	/*NOTREACHED*/
}

void
ffreeall()
{
	while(ptrcnt)
		if (ptrlist[--ptrcnt] != NULL)
			free(ptrlist[ptrcnt]);
	if (ptrlist != NULL)
		free((char *)ptrlist);
	ptrlist = NULL;
	listsize = 0;
}
