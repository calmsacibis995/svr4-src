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
 */

#ident	"@(#)fmli:inc/optabdefs.h	1.1"

/* Note: this file created with tabstops set to 4.
 *
 * Definitions for the Object Operations Table (OOT).  One exists
 * per system, and it defines all the available object operations by
 * OEH number.
 */

#define OPERNAMESIZ	15			/* size of an operation's name */

#define MAX_TYPES	12		/* maximum number of object types in-core */
#define MAX_OPERS	22		/* maximum number of operations per object */

#define NOBUT		-1		/* function not on a label */

/* The following defines are for the func_type field */

#define F_NOP	0x01		/* no operation required */
#define F_ILL	0x02		/* illegal operation */
#define F_NULL	0x03		/* null operation, end of operations */
#define F_INT	0x04		/* internal operation */
#define F_SHELL	0x05		/* fork with shell */
#define F_EXEC  0x06		/* fork with no shell */
#define	F_PARTS	0x07		/* internal parts function (for heuristics) */
#define F_DPARTS	0x08	/* internal directory parts function (ditto) */
#define F_MAGIC	0x09		/* magic number detection (ditto) */

/* the following defines are for the op_type field */

#define OP_SNG	0x01		/* single argument */
#define OP_NEW	0x02		/* new object name */
#define OP_BUT  0x04		/* last label the user selected */
#define OP_DIR	0x08		/* existing directory name */
#define OP_OLD	0x10		/* existing file */
#define OP_CUR	0x20		/* existing item in CURRENT dir */

struct operation {
	char *opername;				/* operation name */
	int  but;					/* label it goes on */
	int  func_type;				/* kind of function */
	int	 intern_func;			/* internal function index */
	char *extern_func;			/* external function name */
	int  op_type;				/* operation type */
	bool multiple;				/* true/false value */
	long all_mask;				/* function available only if all present*/
	long none_mask;				/* function available only if none present*/
	char *perms;				/* permissions */
};
