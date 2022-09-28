/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/utility.c	1.10"

#include <stdio.h>

#include "symint.h"
#include "coredefs.h"
#include "cor_errs.h"
#include "retcode.h"
#include "funcdata.h"
#include "debug.h"

extern PROF_FILE *objfile;

static struct caDATA_BLK * new_data_block();

/*
*	Starting from where we were upon the last exit from this routine,
*	search the symbol table for the given function and read and store
*	some information about it.  If the search returns to the starting
*	point (oldsym_p), then the search failed.
*
*	- Find the correct function or return fail (if we reach the start).
*	- Get pointers into the line section for this function.
*	- Set caline_p to the coverage array lines for this function.
*	- If type is LIST, store index and line number.
*	- Allocate a datablock for the line number information.
*	(Allocate new datablocks as needed.)
*	- For each line, save number and whether it begins a basic block.
*	- Clean up counts and return.
*
*	The test for a given line number beginning a basic block is accurate
*	because the line numbers in the line number section and the line
*	numbers in the coverage data are in the same order.   This is so
*	because their origin common (the assembler source) and both
*	generation schemes (for the line number section and the coverage
*	data line numbers) preserve the line number order presented in
*	the assembler source.
*
*	The search for the function fails when the symbol pointer (sym_p)
*	returns to where the search began (at oldsym_p).  When the search
*	begins, sym_p and oldsym_p are the same; oldsym_p is the first
*	symbol we test.  If we find the function, then the next search
*	will begin with the symbol following the symbol found, thus we
*	set oldsym_p to the next symbol after sym_p.
*/
#define NEXT_SYMBOL(sym_p)	((sym_p) == lastsym_p ? firstsym_p : (sym_p)+1)

/*
*	oldsym_p is a pointer to the symbol of the previous search.
*	Because it's value is retained, it must be reset to 0 before
*	the symbol search begins.
*/
static PROF_SYMBOL *oldsym_p = 0;
CAutility_reset()
{
	oldsym_p = 0;
}

CAgline(bufptr, type, covdata_p)
struct caFUNC_DATA *bufptr;
short type;
struct caCOV_DATA *covdata_p;
{
	struct caDATA_BLK	*curr_blk;
	struct caCOV_STAT	*statdata_p;
	short			count;
	caCOVWORD		calines_left;	/* #of ``bblk-start line #s'' left to match */
	PROF_LINE		*line_p;
	PROF_LINE		*lastline_p;
	caCOVWORD		*caline_p;	/* coverage array lines */
	PROF_SYMBOL		*sym_p;		/* search symbol pointer */
	PROF_SYMBOL		*firstsym_p;	/* first symbol in array */
	PROF_SYMBOL		*lastsym_p;	/* last symbol in array */

	char *func_not_found = "lprof: function '%s' not found\n";

	DEBUG_LOC("top of CAgline");

	firstsym_p = objfile->pf_symarr_p;
	lastsym_p = &(firstsym_p[objfile->pf_nsyms-1]);

	sym_p = (oldsym_p ? oldsym_p : firstsym_p);
	oldsym_p  = sym_p;

	for (;;) {
		if (	(SYMBOL_IS_FUNC(sym_p))
		&&	(strcmp(bufptr->func_name, SYMBOL_NAME(sym_p)) == 0)
		) {
			break;
		}
		if ((sym_p = NEXT_SYMBOL(sym_p)) == oldsym_p) {
			fprintf(stderr, func_not_found, bufptr->func_name);
			return(FUNC_FAIL);
		}
	}

	oldsym_p = NEXT_SYMBOL(sym_p);
	line_p = SYMBOL_LINES_P(sym_p);
	lastline_p = SYMBOL_LASTLN_P(sym_p);
	caline_p = covdata_p->lca_lineos;

	DEBUG(printf("Processing symbol: %s\n",SYMBOL_NAME(sym_p)));
	DEBUG(printf("line_p = 0x%x, lastline_p = 0x%x\n",line_p,lastline_p));

	if ((type == LIST) || (type == LISTALL)) {
		bufptr->func_idx = sym_p - firstsym_p;
		bufptr->line_num = *line_p;
	}

	curr_blk = bufptr->data = new_data_block();
	bufptr->total_ent = 0;
	count = 0;
	/*
	*  Set (calines_left) number of caline entries against which to match.
	*/
	calines_left = covdata_p->lca_bblks;
	statdata_p = curr_blk->stats->data;
	DEBUG(printf("calines_left = %d\n",calines_left));
	DEBUG_LOC("CAgline: Just before do loop.");
	do {
		if (count >= LINEMAX) {
			curr_blk->entry_cnt = count;
			curr_blk->next_blk = new_data_block();
			curr_blk = curr_blk->next_blk;
			bufptr->total_ent += LINEMAX;;
			count = 0;
			statdata_p = curr_blk->stats->data;
		}

		DEBUG(printf("src line==%2d,\tln# for nxt blk==%2d",
			*line_p, (calines_left>0 ? *caline_p : 0)
		));
		DEBUG(printf("\t%s\n",
			((calines_left>0 && *caline_p == *line_p)
				? "<<New Block!!>>" : "<blk continued>"
			)
		));

		statdata_p[count].line_num = *line_p;
		statdata_p[count].block = (
			(calines_left>0 && *caline_p == *line_p)
			? (caline_p++, calines_left--, TRUE)
			: FALSE
		);
		count++;
	} while (line_p++ < lastline_p);

	curr_blk->entry_cnt = count;  
	bufptr->total_ent += count;

	return(OK);
}


/*
*	Allocate, initialize, and return a new data block.
*/
static struct caDATA_BLK *
new_data_block()
{
	char *malloc();
	struct caDATA_BLK *p;

	/* initialize data block list */
	p = (struct caDATA_BLK *) malloc(sizeof(struct caDATA_BLK));
	p->entry_cnt = 0;
	p->stats = (struct caSTAT_BLK *) malloc(sizeof(struct caSTAT_BLK));
	p->next_blk = NULL;
	return(p);
}



CAinterp_array(covdata,funcdata)
struct caCOV_DATA *covdata;
struct caFUNC_DATA *funcdata;
{
	caCOVWORD		count, block_cnt;
	caCOVWORD		word, *larrayptr;
	struct caDATA_BLK	*current;


	block_cnt = CAblock_cnt(funcdata);
	DEBUG(printf("blkcnt==%d, lca_bblks==%lu\n",
		block_cnt, covdata->lca_bblks ));

	if (covdata->lca_bblks != block_cnt) {
		fprintf(stderr,"lprof: %s\n",COR106);
		return(SIZE_FAIL);
	}

	count = 0;
	current = funcdata->data;

	/* initialize coverage array pointer and size */
	larrayptr = covdata->lca_counts;
	word = *larrayptr;
	while ((current != NULL) && (count < (unsigned int)current->entry_cnt))
	{
		if (current->stats->data[count].block == TRUE)
		{	
			/* This is the beginning of a logical block. */
			word = *(larrayptr++);
			current->stats->data[count++].count = word;
		} else {
			/* This is NOT the beginning of a logical block. */
			current->stats->data[count++].count = word;
		}

		if (count >= LINEMAX) {
			current = current->next_blk;
			count = 0;
		}
	}

	return(OK);
  }





CAblock_cnt(funcdata)
struct caFUNC_DATA *funcdata;
{
	struct	caDATA_BLK *datablk;
	short	index,count;

	index = 0;
	count = 0;
	datablk = funcdata->data;

	/* count the number of entries that are marked as logical blocks */
	while((datablk != NULL) && (index < (unsigned int)datablk->entry_cnt)) {
		if(datablk->stats->data[index++].block == TRUE)
			count++;
		if (index >= LINEMAX) {
			datablk = datablk->next_blk;
			index = 0;
		}
	}

	return(count);
}



CAchk_covf(code)
short code;
{

	if(code != OK) {
		if((code & EDIT_MASK) != 0)
			fprintf(stderr,"***WARNING:  PROFILING DATA HAS BEEN EDITED \n");
		if((code & VER_MASK) != 0)
			fprintf(stderr,"***WARNING:  PROFILING DATA WAS CREATED WITH DIFFERENT VERSION OF LPROF/XPROF\n");
		if((code & MACH_MASK) != 0) {
			fprintf(stderr,"***WARNING: CNTFILE MACHINE TYPE DOES NOT MATCH THIS MACHINE\n");
			fprintf(stderr,"\tDATA MAY BE UNINTELLIGIBLE, ERRORS ARE LIKELY\n");
		}
		if((code & COMP_MASK) != 0)
			fprintf(stderr,"***WARNING:  CNTFILE IS INCOMPLETE, POSSIBLY BAD DATA \n");
	  }

 }



CAchk_obj(objfile,objbufptr)
PROF_FILE *objfile;
struct caOBJ_ENTRY *objbufptr;
{
	short _CAobj_test();

	if(_CAobj_test(objfile,objbufptr) != OK)
	{
		fprintf(stderr,"***WARNING:  OBJECT FILE'S TIME STAMP IS DIFFERENT THAN IT WAS WHEN PROFILING\n");
		fprintf(stderr,"\tDATA WAS RECORDED. IF FILE HAS BEEN CHANGED, ERRORS ARE LIKELY\n");
	   }
  }
