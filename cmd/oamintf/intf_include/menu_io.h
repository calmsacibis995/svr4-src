/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)oamintf:intf_include/menu_io.h	1.1.3.1"

#define P_NONE		0	/* no placeholder */
#define P_INACTIVE	1	/* placeholder - inactive */
#define P_ACTIVE	2	/* placeholder - active */

/* pull in sizes from intf.h */
struct item_def {
	char mname[(NAMELEN+1)];	/* menu item name */
	char mdescr[(DESCLEN+1)];	/* menu item description */
	char help[HELPLEN];		/* menu item help file */
	int pholder;			/* placeholder status */
	char maction[ACTLEN];		/* menu item action */
	char pkginsts[PKGILEN];		/* pkg instance identifiers */
	char orig_name[(NAMELEN+1)];	/* original name if rename */
	struct item_def *next;		/* next menu item */
};

struct menu_file {
	struct menu_line *head;		/* menu file header lines */
	struct item_def *entries;	/* menu file entries */
};
