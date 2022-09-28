/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/rept_utl.c	1.9"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>

#include "symint.h"
#include "filedata.h"
#include "retcode.h"
#include "covfile.h"
#include "funcdata.h"
#include "glob.h"
#include "env.h"
#include "coredefs.h"


struct caFILEDATA *filedata;
struct caCOV_DATA covdata;
struct caSRC_FILE *src_list;
PROF_FILE *objfile;


CAreport(type,envir)
short type;
struct command *envir;
{
	short ret_code;

	if ( envir == NULL) {
		fprintf(stderr,"\nCAreport: bad pointer passed\n");
		return(BUG_FAIL);
	}

	/*
	*	Reset static variables.  See also utility.c.
	*/
	CAutility_reset();

	/* is this 'if' needed?? */
	if ( envir->cov_next > 0 )
		ret_code = CAcovrept(type,envir);
	return(ret_code);
}

		

#define COV_DATA_LOST_STR \
"\n***coverage data for function '%s',object file '%s',\n\
\tnot found in covfile '%s'***\n"

CAcovrept(type,envir)
struct command *envir;
short type;
{
    char *obj_name;

    struct caOBJ_ENTRY obj_buf;

    struct caFILEDATA *_CAopen_covf();
    struct caSRC_FILE *CAsrc_list();
    short _CAvalid_test();
    short _CAfind_obj();
    short _CAfind();


    short ret_code,k,validcode;

    ret_code = OK;

    filedata = _CAopen_covf(envir->cov_ptr[0]);
    if (filedata == NULL) {
	fprintf(	
	    stderr,
	    "\n***unable to access CNTFILE '%s'***\n",
	    envir->cov_ptr[0]
	);
	return(COND_FAIL);
    }

    /* make COVFILE validity check */
    validcode = _CAvalid_test(filedata);

    /*
    *  The following statement will set obj_name to either NULL
    *  (if no object file is specified) or the name of the object file.
    */
    obj_name = envir->obj_ptr;

    if(_CAfind_obj(filedata, obj_name, &obj_buf) != OK) {
    	/* _CAfind_obj failed */
	fprintf(stderr,"\n***no coverage data ");
	if (obj_name != NULL)
	    fprintf(stderr,"for object file '%s' ",obj_name);
	fprintf(stderr,"in CNTFILE '%s'***\n",envir->cov_ptr[0]);
	ret_code = COND_FAIL;
	goto theend;
    }

    if (obj_name == NULL)
	obj_name = obj_buf.name;
    /* print header information */
    CArepthdr(type,envir,envir->cov_ptr[0],obj_name);
    /* report on COVFILE validity */
    CAchk_covf(validcode);
    /* read the objectfile */
    if ((objfile = _symintOpen(obj_name)) == NULL) {
	/* try name in COVFILE */
	obj_name = obj_buf.name;
	if ((objfile = _symintOpen(obj_name)) == NULL) {
	    fprintf(stderr,"\n***cannot access object file '%s'***\n",obj_name);
	    ret_code = COND_FAIL;
	    goto theend;
	}
    }
    if (elf_kind(objfile->pf_elf_p) == ELF_K_COFF) {
	    fprintf(stderr,
	    "lprof: %s: Warning - internal conversion of COFF file to ELF\n",
	    obj_name
	    );
    }

    /*
    *  Make object file validity check, and print warning message
    *  if there is a problem.
    */
    CAchk_obj(objfile,&obj_buf);
    if((type == LIST)||(type == LISTALL))
	if ((src_list = CAsrc_list(envir, filedata)) == NULL) {
	    fprintf(stderr,"\nNo source files found.\n\n");
	    ret_code = SRC_FAIL;
	}
    /* set up line number table pointers, and read in
	section header to find out how many line numbers in file */
    /* CAget_fdata calls CAgline */
    if((envir->fnc_next > 0) && (ret_code != SRC_FAIL)) {
	    /* for each function specified */
	    for(k = 0; k < envir->fnc_next; ++k) {
		    if(_CAfind(filedata, envir->fnc_ptr[k]) != OK) {
			    fprintf(
				    stderr,
				    COV_DATA_LOST_STR,
				    envir->fnc_ptr[k],
				    obj_name,
				    envir->cov_ptr[0]
			    );
			    ret_code = COND_FAIL;
		    } else {
			    CAget_fdata(type, envir);
		    }
	    }
    } else if (ret_code != SRC_FAIL) {
	    /* for all functions in this object file */
	    _CArewind(filedata);
	    while(CAget_fdata(type,envir) == OK)
		    ;
    }
    /* if this is a marked listing type
       report, generate the listing */
    if (((type == LIST)||(type == LISTALL)) && (ret_code != SRC_FAIL))
	CAlist_gen(envir);
    if (type == SUM)
	CAsum_tot();

    _symintClose(objfile);

theend:;

    /* close COVFILE */
    _CAclose_covf(filedata);

    return(ret_code);
}



CAget_fdata(type,envir)
short type;
struct command *envir;
{
    short ret_code, flag;
    struct caFUNC_DATA *node;
    struct caFUNC_DATA funcdata;
    short _CAread();
    char *malloc();
    char *strcpy();
    void free();


    if ( (ret_code = _CAread(filedata,&covdata)) == OK)
    {
	if (type == LINE || type == SUM)
	{
	    /* this is a 'summary' or 'line itemized' type of report */
	    funcdata.func_name = malloc(covdata.fname_size+1);
	    (void)strcpy(funcdata.func_name,(char *)covdata.func_name);
	    /* get the line numbers */
	    flag = CAgline(&funcdata, type, &covdata);
	    if (flag == OK)
	    {
		/* interpret the coverage array */
		if (CAinterp_array(&covdata,&funcdata) == OK)
		{
		    /* generate the report */
		    if (type == LINE)
			CAlinerept(&funcdata,envir);
		    else
			CAsumrept(&funcdata);
		}
		else
		    ret_code = COND_FAIL;
	    }
	    else
		ret_code = COND_FAIL;
	    free(funcdata.func_name);
	}
	else  /* this is a 'marked listing' type of report, so accumulate
		    the function data */
	{
	    node = (struct caFUNC_DATA *) malloc(sizeof(struct caFUNC_DATA));
	    node->func_name = malloc(covdata.fname_size+1);
	    (void)strcpy(node->func_name,(char *)covdata.func_name);
	    node->next_func = NULL;
	    flag = CAgline(node, type, &covdata);
	    if (flag == OK)
	    {
		if (CAinterp_array(&covdata,node) == OK)
		{
		    if ((flag = CAadd_src(src_list, node)) != OK)
		    {
			if (flag == SRC_FAIL)
			    /* the function is not in given source list */
			    ret_code = OK;
			else
			    ret_code = COND_FAIL;
			CAfree_fdata(node);
		    }
		}
		else
		{
		    ret_code = COND_FAIL;
		    free(node->func_name);
		    free(node);
		}
	    }
	    else
	    {
		ret_code = COND_FAIL;
		free(node->func_name);
		free(node);
	    }
	}
    }
	
    return(ret_code);
}






CAlist_gen(envir)
struct command *envir;
{
	CAlistrept(src_list,envir);
	CAfree_src(src_list);
}




CArepthdr(type,envir,src,obj)
short type;
struct command *envir;
char *src, *obj;
{
	switch (type)
	{
		case LINE:
		case SUM:
			CAsumhdr(type,src,obj);
			break;
		case LIST:
		case LISTALL:
		case BLANK:
			break;	/* no header */
		default:
			fprintf(stderr,"\nCArepthdr:  unrecognized report type\n");
			break;
	   }
  }

