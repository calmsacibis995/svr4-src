/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/src_list.c	1.11"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include	<stdio.h>
#include	<string.h>

#include	"symintHdr.h"
#include	"retcode.h"
#include	"funcdata.h"
#include	"filedata.h"
#include	"glob.h"
#include	"env.h"

#define	PATHLEN	250
#define	READ	04	/* access(2) pointer */


long	index;		/* used to pass index from findfile to CAsrc_list */
long	last_index;	/* index of last function for given file */
PROF_SYMBOL	symbol;		/* used to pass entry to CAsrc_list */

extern PROF_FILE *objfile;

struct caSRC_FILE *
CAsrc_list(envir,filedata)
struct command *envir;
struct caFILEDATA *filedata;
{
    struct caSRC_FILE *new_src, *head, *current;
    int i, new_file;
    long idx, cur_file_idx, last_idx, previdx;
    char cur_file_name[FILNMLEN];
    PROF_SYMBOL entry;
    char *tmpptr, *filename, *fileonly, *fname, *func;
    short flag;
    char *searchdir(), *malloc(), *strcpy(), *strncpy();
    short _CAtraverse();

    head = NULL;
    current = NULL;
    if (envir->sourc_next > 0) {
	/* for each source in envir */
	for (i = 0; i < envir->sourc_next; i++) {
	    /* get simple file name */
	    filename = envir->sourc_ptr[i];
	    tmpptr = filename;
	    for (fileonly = tmpptr; *tmpptr != '\0'; tmpptr++) {
		if (*tmpptr == '/')
		    fileonly = tmpptr + 1;
	    }
	    if (findfile(fileonly, envir->obj_ptr) == OK) {
		/* first check if source file is accessible */
		if ((fname = searchdir(filename, envir)) != NULL) {
		    new_src = (struct caSRC_FILE *) malloc(sizeof(struct caSRC_FILE));
		    /* keep track of complete path name */
		    new_src->file_name = fname;
		    new_src->sym_idx = index;
		    new_src->last_idx = last_index;
		    new_src->func_list = NULL;
		    new_src->next_file = NULL;

		    /* add to list */
		    if (head == NULL) {
			/* this is the first in list */
			head = new_src;
		    }
		    else {
		    /* insert in order of increasing sym_idx */
			if (head->sym_idx > new_src->sym_idx) {
			    /* insert at beginning */
			    new_src->next_file = head;
			    head = new_src;
			}
			else {
			    current = head;
			    while ((current->next_file != NULL) && 
			      (current->next_file->sym_idx <= new_src->sym_idx)) {
				current = current->next_file;
			    }
			    if (current->sym_idx == new_src->sym_idx) {
				fprintf(stderr,"****warning:  %s appears twice in list of files, second reference discarded\n", current->file_name);
			    }
			    else {
				new_src->next_file = current->next_file;
				current->next_file = new_src;
			    }
			}
		    }
		}
		else {
		    fprintf(stderr, "*****warning: cannot access %s\n", filename);
		    fprintf(stderr,"     discarding source file %s\n", filename);
		}
	    }
	    else {
		fprintf(stderr,"*****warning: file %s not found in object file\n",
		    fileonly);
		fprintf(stderr,"     discarding source file %s\n", filename);
	    }
	}
    }
    /* no source files given, look at COVFILE for list of file names */
    else {
        DBG_TAG dbgt;

	_CArewind(filedata);

	idx = 0;
	new_file = 0;

	flag = _CAtraverse(filedata, &func);
	/* flag == EOF_FAIL if this is the last one */
	/* func is name of next function in COVFILE */

	previdx =  0;	/* Initialize previous index into symbol table */
	do {
	/*
	    1.  Find next function or file entry in symbol table.
	    2.  If file, update cur_file_idx
	    3.  If function, is it func?  
		    yes - then we want cur_file_idx
			first check to see if we aleady have
			  this file.  (must continue to
			  iterate through because if we skip
			  to next file, the next function may
			  still be in this file)
			if file not yet there,
			  create caSRC_FILE struct
			call _CAtraverse again if flag != EOF_FAIL
			  to get next function to look for
		    no - go back to 1 until reach global symbols
			or no more functions.
			(we are in serious trouble if at end
			of symbol table and there are functions
			left!!!)
	*/

	    if (previdx > idx) {
		fprintf(
			stderr,
			"lprof:  symbol table indexing incorrect in executable for %s\n",
			objfile->pf_symstr_p[entry.ps_sym.st_name]
		);

		exit(1);	/* Exit indicating error in processing */
	    }

            entry = objfile->pf_symarr_p[idx];

	    /* idx should never go below previous idx, otherwise there is a */
	    /* problem in the symbol table. */
	    previdx = idx;	

	    if (entry.ps_dbg.pd_symtag == TAG_source_file) {
		cur_file_idx = idx;
		new_file = 1;
		strncpy(
		    cur_file_name,
		    entry.ps_dbg.pd_name,
		    FILNMLEN
		);
		last_idx = find_last_filidx(cur_file_idx+1);
	    } else if (
		(
			(dbgt = entry.ps_dbg.pd_symtag) == TAG_subroutine
			|| dbgt == TAG_global_subroutine
		) && (
			strcmp(func, entry.ps_dbg.pd_name) == 0
		)
	    ) {
		if (new_file) {
		    /* this is a new file, add to our list */
		    new_file = 0;

		    /* first check if source file is accessible */
		    if ((fname = searchdir(cur_file_name, envir)) == NULL) {
			fprintf(stderr,
			    "*****warning: cannot access %s\n",
			    cur_file_name
			);
			continue;
		    }

		    /* keep track of complete path name */
		    new_src = (struct caSRC_FILE *)
			    malloc(sizeof(struct caSRC_FILE));
		    new_src->file_name =
			    malloc((unsigned)strlen(fname) + 1);
		    (void) strcpy(new_src->file_name, fname);
		    new_src->sym_idx = cur_file_idx;
/* ??? */	    new_src->last_idx = last_idx;
		    new_src->func_list = NULL;
		    new_src->next_file = NULL;

		    /* add to list */
		    /* list is in order of increasing sym tbl index */
		    if (head == NULL) {
			/* this is the first in list */
			head = new_src;
		    } else {
			current->next_file = new_src;
		    }
		    current = new_src;
		}

		if (flag == EOF_FAIL) {
		    goto theend;
		} else {
		    flag = _CAtraverse(filedata, &func);  /* get next func */
		}
	    }

	} while (idx++ < objfile->pf_nsyms);
    }

theend:;

    return(head);
}

char *
searchdir(shortname, cmd)
char *shortname;
struct command *cmd;
{
    int i;
    char longname[PATHLEN + 1];
    char *retname;
    char *malloc(), *strcpy();

    if (access(shortname, READ) == 0) {
	retname = malloc((unsigned) strlen(shortname) + 1);
	(void) strcpy(retname, shortname);
	return(retname);
    }
    for (i = 0; i < cmd->inc_next; i++) {
	sprintf(longname, "%s/%s", cmd->incdir_ptr[i], shortname);
	if (access(longname, READ) == 0) {
	    retname = malloc((unsigned) strlen(longname) + 1);
	    (void) strcpy(retname, longname);
	    return(retname);
	}
    }
    return(NULL);
}



CAadd_src(head, new)
struct caSRC_FILE *head;
struct caFUNC_DATA *new;
{
    /* add a function to the corresponding file */

    struct caFUNC_DATA *tmp;

    /* we are not guarenteed that the file will be in the list
	this will happen when user specifies src files, and functions,
	and the functions are not in the src files given 
	Also, function may be duplicated */


    while ((head != NULL) && (head->last_idx < new->func_idx))
	head = head->next_file;

    if (head == NULL) {
	/* function is not in the set of source files given */
	return(SRC_FAIL);
    }

    /* check that lower boundary is right */
    if (head->sym_idx > new->func_idx) {
	/* file is not in src_list */
	/* if user specified src files, some funcs will not be in these files,
	   but that is OK */
	/* when fixing this, fix also CAget_fdata in core/com/CArept_utl.c */
	/*fprintf(stderr,"*****error:  function %s not found in source files.\n", new->func_name);
	fprintf(stderr,"      If you used -r option, the function was not found in those files\n");
	*/
	return(SRC_FAIL);
    }
    /* head is the file we want */
    if (head->func_list == NULL) {
	head->func_list = new;
	new->next_func = NULL;
    }
    else {
	tmp = head->func_list;
	if (tmp->func_idx > new->func_idx) {
	    /* insert at beginning */
	    new->next_func = tmp;
	    head->func_list = new;
	}
	else {
	    while ((tmp->next_func != NULL) && (tmp->next_func->func_idx <= new->func_idx)) {
		tmp = tmp->next_func;
	    }
	    if (tmp->func_idx == new->func_idx) {
		if (strcmp(tmp->func_name, new->func_name) == 0) {
		    /* function appeared twice in function list */
		    fprintf(stderr,"*****warning:  function %s listed twice, ignoring second reference\n");
		    return(OK);
		}
		else {
		    /* ERROR! */
		    fprintf(stderr,
			"lprof: two functions with same line numbers but different names found\n");
		    return(FUNC_FAIL);
		}
	    }
	    else {
		new->next_func = tmp->next_func;
		tmp->next_func = new;
	    }
	}
    }
    return(OK);
}


CAfree_fdata(fdata)
struct caFUNC_DATA *fdata;
{
    struct caDATA_BLK *data_list, *data_tmp;
    void free();

    if (fdata != NULL) {
	data_list = fdata->data;
	while (data_list != NULL) {
	    data_tmp = data_list->next_blk;
	    free(data_list->stats);
	    free(data_list);
	    data_list = data_tmp;
	}
	free(fdata->func_name);
	free(fdata);
    }
}



CAfree_src(src_list)
struct caSRC_FILE *src_list;
{
    struct caSRC_FILE *src_tmp;
    struct caFUNC_DATA *func_list, *func_tmp;
    void free();

    while (src_list != NULL) {
	func_list = src_list->func_list;
	while (func_list != NULL) {
	    func_tmp = func_list->next_func;
	    CAfree_fdata(func_list);
	    func_list = func_tmp;
	}
	src_tmp = src_list->next_file;
	free(src_list->file_name);
	free(src_list);
	src_list = src_tmp;
    }
}








/* STATIC VARIABLES */
static long	lastfilndx = 0;
static long	symndx = 0;

/*  findfile searchs the symbol table of the object file for source file name
 *
 *  the following static variables are used to localize the search
 *	    - symndx  is the symbol table index of the NEXT symbol table
 *		entry to be read.
 *	    - lastfilndx  is the symbol table index of the LAST FILE entry
 *		processed.
 *		If symndx ever grows (mod objfile->pf_nsyms) equal
 *		to lastfilndx:  the named file is not in the symbol table.
 *
 *	findfile(filename)  simply discovers whether or not the named file 
 *	is entered in the symbol table of the object file (objname).
 *
 *	The variable is used only to make
 *	searching the symbol table a little bit quicker.
 *
 *	The variable lastfilndx is more important if there happen
 *	to be two functions of the same name (one or more static)
 *	in the same object file.
 *
 *  returns:
 *      - OK when the appropriate filename is found
 *
 */


int
findfile(filename, objname)
char	*filename;
char	*objname;
{
    /*
    *	From the current position in the symbol table (given by static
    *	"symndx") search for the given file name (wrapping around the
    *	end of the symbol table, if necessary).  If the search reaches
    *	back to the starting point (lastfilndx), then the file is not
    *	to be found.
    */

    /* filename is already simple file name */
    do {
	register PROF_DEBUGE	*p;
	long 			cur_symndx;

        symbol = objfile->pf_symarr_p[symndx];
	p = &(symbol.ps_dbg);

	cur_symndx = symndx;
	if (++symndx >= objfile->pf_nsyms) {
	    symndx = 0;
	}

	if (
		(p->pd_symtag == TAG_source_file) 
	&&	(strncmp(filename, p->pd_name, FILNMLEN) == 0)
	) {
		lastfilndx = cur_symndx;
		index = cur_symndx;
		last_index = find_last_filidx(index+1);

		return(OK);
	}

    } while (symndx != lastfilndx);

    return(EOF_FAIL);
}


/*
*	Find the last index for the given file.
*/
find_last_filidx(tmp_index)
long tmp_index;
{
	long i;

	for (i = tmp_index; i < objfile->pf_nsyms; i++) {
	    if (objfile->pf_symarr_p[i].ps_dbg.pd_symtag == TAG_source_file)
		break;
	}
	return(i-1);
}

