/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/list_rept.c	1.15"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>

#include "symint.h"
#include "glob.h"
#include "env.h"
#include "retcode.h"
#include "funcdata.h"
#include "debug.h"

#define LINELEN	513


#define OKSLOP	(8)		/* based on assumption that an 8-col tab follows */
#define MARGIN	(10)		/* basic, well-behaved margin */
char linenum[MARGIN+OKSLOP+1] = "";	/* margin of listing - add for reasonable */
					/* Margin Release, and the NULL @the end. */
	/* note bene: %min.max where min=MARGIN and max=MARGIN+OKSLOP */
#define MARGINFMT	"%10.18s\t%s"


extern short lxprint;

char *fld_mark = "U";	/* used by xprof in margin of listing to indicate coverage */

char buffer[LINELEN];
unsigned short linecount;
/* linecount is absolute line number - needed if printing absoulte line numbers
   instead of relative line numbers */

CAlistrept(src_list, envir)
struct caSRC_FILE *src_list;
struct command *envir;
{
    FILE *srcptr;
    struct caFUNC_DATA *func;
    struct caFUNC_DATA *tmpf;
    struct caFUNC_DATA *firstfunc;
    short flag;


    /*  need to know if entire file is being printed, or just
	certain functions.  If just certain functions, then there
	may be unnecessary files in src_list, and we don't want
	to print out everything.  Otherwise, whatever
	is in src_list is to be printed. */

    if (envir->fnc_next == 0) {
	/* no functions wanted, just the source files in src_list */
	for ( ; src_list != NULL; src_list = src_list->next_file) {
	    if ((srcptr = fopen(src_list->file_name, "r")) != NULL) {
		/* Print name of source file */
		printf ("\nSOURCE FILE:  %s\n\n",src_list->file_name);
		/* print it */
		flag = OK;
		linecount = 0;
		firstfunc = src_list->func_list;
		while (flag == OK) {
		    func = NULL;
		    /*
		    *	Find the function that occurs next in the
		    *	source file (given that the current position
		    *	in the file is at line "linecount").  This is
		    *	a new addition to the code because the functions
		    *	are now (with ELF) sorted by name (they used to
		    *	be in order of appearance in the file).
		    *
		    *	This amounts to finding the least upper bound
		    *	of all the function's line_nums.
		    */
		    for (tmpf = firstfunc; tmpf; tmpf = tmpf->next_func) {
			if (tmpf->line_num > linecount) {
			    if (func) {
				if (tmpf->line_num < func->line_num) {
				    func = tmpf;
				}
			    } else {
				func = tmpf;
			    }
			}
		    }
		    if (!func) {
			break;
		    }

		    while (
			    ((uint) func->line_num > (uint) linecount + 1)
			    && (flag == OK)
		    ) {
			/* print non-executable lines (or funcs without
			   any coverage data) before func */
			if (fgets(buffer, LINELEN, srcptr) != NULL) {
			    ++linecount;
			    printf(MARGINFMT, linenum, buffer);
			} else {
			    DEBUG(printf("unexpected EOF #1\n"));
			    fprintf(stderr, "unexpected EOF\n");
			    flag = EOF_FAIL;
			}
		    }
		    if (flag == OK)
			flag = printfcn(srcptr, func);
		}
		/* no more funcs to correlate, print to EOF */
		while (fgets(buffer, LINELEN, srcptr) != NULL) {
		    ++linecount;
		    printf(MARGINFMT, linenum, buffer);
		}
		fclose(srcptr);
	    } else {
		fprintf(
			stderr,
			"*****error:  cannot open %s\n",
			src_list->file_name
		);
	    }

	}	/* for each source file in src_list */
    }
    else {
	/* only certain functions are to be printed */
    }

}


/* printfcn prints up to the last executable line of the function */

printfcn(src, func)
FILE *src;
struct caFUNC_DATA *func;
{

    unsigned short lnno;
    int cnt;
    short flag;
    struct caDATA_BLK	*blk;

    DEBUG(printf("top of printfcn\n"));

    flag = OK;
    cnt = 0;
    lnno = 1;		/* lnno is relative to function */
    lnno = linecount+1;	/* NO! now lnno is absolute */
    blk = func->data;

    /* right now, to get all line numbers in order, first to
	a linear scan to see if out of order.
	If out of order call sort.
	    Sort will realloc space if more than one blk.
	    Then call qsort(3) to sort by line number.
	This is not good, because qsort does not work well
	when list is mostly in order.  In this case, line
	numbers are mostly in order.  Insertion sort may
	be better here */

    /* first check to see that all line numbers are in order */
    if (ordered(blk) == 0) {
	sort(blk);
    }
    
    while ((blk != NULL) && (flag == OK)) {
	while (
		((unsigned int) blk->stats->data[cnt].line_num > lnno)
		&& (flag == OK)
	) {
	    if (fgets(buffer, LINELEN, src) != NULL) {
		lnno++;
		linecount++;
		printf(MARGINFMT, linenum, buffer);
	    } else {
	        DEBUG(printf("unexpected EOF #2\n"));
		fprintf(stderr, "unexpected EOF\n");
		flag = EOF_FAIL;
	    }
	}
	/* now line_num <= lnno */

	/*
	 *  if relative line numbers are desired, print lnno-1 
	 *  if absolute line numbers are desired, print linecount.
	 */
	linecount++;
	if (lxprint == XOPT) {
		if (blk->stats->data[cnt].count > 0) {
			sprintf(linenum, "[%d]", linecount);
		} else {
			sprintf(linenum, "[%s] [%d]", fld_mark, linecount);
		}
	} else {
		sprintf(
			linenum,
			"%ld [%d]", blk->stats->data[cnt].count,
			linecount
		);
	}
	DEBUG(printf("cnt = %d, blk->entry_cnt  = %d\n",cnt,blk->entry_cnt));
	if (blk->stats->data[cnt].line_num == lnno) {
		if (fgets(buffer, LINELEN, src) != NULL) {
			lnno++;
			printf(MARGINFMT, linenum, buffer);
		} else {
			DEBUG(printf("unexpected EOF #3\n"));
			fprintf(stderr, "unexpected EOF\n");
			flag = EOF_FAIL;
		}
	} else {
		printf(MARGINFMT, linenum,
			"WARNING: No matching source line.\n"
		);
	}
	linenum[0] = '\0';

	if (++cnt >= LINEMAX) {
	    /* get next block of line numbers */
	    DEBUG(printf("Advancing to next block\n"));
	    DEBUG(printf("Old block = 0x%x\n",blk));
	    blk = blk->next_blk;
	    DEBUG(printf("New block = 0x%x\n",blk));
	    cnt = 0;
	} else if (cnt == blk->entry_cnt) {
	    /* done with function */
	    DEBUG(printf("Done with function\n"));
	    break;
	}
    } /* end while blk != NULL */

    DEBUG(printf("bottom of printfcn\n"));
    return(flag);
}

ordered(blk)
struct caDATA_BLK *blk;
{
    int i, last_no;
    struct caDATA_BLK *tmp;

    last_no = 0;
    tmp = blk;
    while (tmp != NULL) {
	for (i = 0; i < (unsigned int)tmp->entry_cnt; i++) {
	    if ((unsigned int)tmp->stats->data[i].line_num < (unsigned int)last_no)
		return 0;
	    last_no = tmp->stats->data[i].line_num;
	}
	tmp = tmp->next_blk;
    }
    return 1;
}

sort(blk)
struct caDATA_BLK *blk;
{
    int no_blks, compare();
    struct caDATA_BLK *last;
    struct caSTAT_BLK *nblk;

    /* if more than one blk, first realloc stats so that we can call qsort */
    /* first find number of blks */
    no_blks = 1;
    last = blk;
    while (last->next_blk != NULL) {
	no_blks++;
	last = last->next_blk;
    }

    if (no_blks > 1) {	/* more than one blk */
	nblk = (struct caSTAT_BLK *)realloc((char *)blk->stats, (unsigned)(no_blks * sizeof(struct caSTAT_BLK)));
	if (nblk == NULL) {
	    fprintf(stderr, "***WARNING: out of space, printing errors likely in this function\n");
	    return;
	}
	blk->stats = nblk;
	/* now copy blks and reassign pointers in blks */
	last = blk;
	while (last->next_blk != NULL) {
	    nblk++;	/* pointer arith */
	    (void)memcpy((char *)nblk, (char *)last->next_blk->stats, sizeof(struct caSTAT_BLK));
	    last->next_blk->stats = nblk;
	    last = last->next_blk;
	}
    }

    /* now call qsort to sort line numbers */
    qsort((char *)blk->stats->data, (unsigned)((no_blks-1) * LINEMAX + last->entry_cnt),
	sizeof(struct caCOV_STAT), compare);
}

compare(a,b)
struct caCOV_STAT *a, *b;
{
    if (a->line_num == b->line_num)
	return(0);
    if ((unsigned int)a->line_num < (unsigned int)b->line_num)
	return(-1);
    return(1);
}

