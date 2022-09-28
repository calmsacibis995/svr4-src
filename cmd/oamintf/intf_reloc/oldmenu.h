/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_reloc/oldmenu.h	1.5.1.2"

/*
#define NAMELEN 16
#define DESCLEN 58
*/

#define DESC		"DESC"
#define OLD_PKG		"_PRE4.0"
#define OLD_SYSADM	"/usr/admin/r3_sysadm"
#define OLD_SYS		"old_sysadm"

/* values for 'turn_pholder' flag in reloc.c */
#define NEUTRAL		0
#define ON		1
#define OFF		2

struct old_item {
	char o_name[NAMELEN+1];		/* name of menu item */
	char o_desc[DESCLEN+1];		/* menu item descr */
 	struct old_item *o_next;	/* next menu item */
};
