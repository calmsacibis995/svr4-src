/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:comops.c	1.2"
#include <stdio.h>
#include <string.h>

#include "retcode.h"
#include "lst_str.h"



_CAadd_flist(list_start,new_func,new_name)

	struct caFUNCLIST **list_start, *new_func;
	char   *new_name;

/* add new member to list of functions used.
   (used when there is a possibility of functions having been added to a
   particular object file entry.  */

{


	short  ret_code;
	char *malloc();


	ret_code = OK;

	new_func->name = malloc((unsigned)strlen(new_name)+1);
	strcpy(new_func->name,new_name);
	new_func->next_func = *list_start;
	*list_start = new_func;

	return(ret_code);

}




_CAf_search(name,list_start)

	char   *name;
	struct caFUNCLIST   *list_start;

/* search list of functions */

{
	struct caFUNCLIST *current;

	short  ret_code;

	ret_code = FALSE;

        current = list_start;
	while ((current != NULL) && (ret_code == FALSE)) {
	    if (strcmp(name,current->name) == 0)
		ret_code = TRUE;
	    current = current->next_func;
	}
	return(ret_code);

}
	



_CAfree_list(func_list)

	struct caFUNCLIST *func_list;

/* release the memory used by the function name list. */

{
	struct caFUNCLIST *temp_fcn;
        short  ret_code;
	void free();

	ret_code = OK;

	while (func_list != NULL) {
	    temp_fcn = func_list->next_func;
	    free(func_list->name);
	    free((char *) func_list);
	    func_list = temp_fcn;
	}


	return(ret_code);

}
