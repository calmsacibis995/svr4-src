/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/env.h	1.2"
/* This global "command" structure will contain items input on the command line */
struct command {

	float tsh;		/* threshold value */
	char *tsh_level;	/* "a" or "b" for above and below respectively*/
	short cov_reqst;	/* the type of reporting requested */
				/* 1 = covered    		   */
				/* 0 = not covered 		   */

	char *obj_ptr;		/*pointer to object file */

	char **cov_ptr;		/* pointer to coverage file list */
	short cov_next;		/* index to next avail pointer */

	char *dest_ptr;		/* pointer to name of destination file for merging */

	char **sourc_ptr;  	/* pointer to sourcefile name structure */
	short sourc_next;	/* index to next avail pointer */

	char **fnc_ptr;  	/* pointer to function name list*/
	short fnc_next;		/* index to next avail pointer */

	char **incdir_ptr;	/* pointer to include directories */
	short inc_next;		/* index to next avail pointer */

};
