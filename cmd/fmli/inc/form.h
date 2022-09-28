/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)fmli:inc/form.h	1.4"

typedef struct {
	char *name;		/* contents of "name" descriptor */
	char *value;		/* contents of "value" descriptor */
	int frow;		/* contents of "frow" descriptor */
	int fcol;		/* contents of "fcol" descriptor */
	int nrow;		/* contents of "nrow" descriptor */
	int ncol;		/* contents of "ncol" descriptor */
	int rows;		/* contents of "rows" descriptor */
	int cols;		/* contents of "cols" descriptor */
	int flags;		/* flags set according to the values of
				   "field related" boolean descriptors
				   (scroll, edit, etc. see winp.h) */
	char **ptr;		/* object dependent pointer to low 
				   level field structure (ifield) */ 
} formfield;

struct form {
	formfield (*display)();	/* display function of object */
	char *	  argptr;	/* (object dependent) arg passed "display" */
	vt_id	  vid;		/* virtual terminal number */
	int	  curfldnum;	/* current field num */
	int	  flags;	/* misc. flags (listed below) */
	int	  rows;		/* number of rows in form */
	int	  cols;		/* number of columns in form */
};

#define FORM_USED	1
#define FORM_DIRTY	2	/* contents of form changed */
#define FORM_ALLDIRTY	4	/* form has been reshaped or moved */

extern form_id		FORM_curid;
extern struct form	*FORM_array;
