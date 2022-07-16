/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)userdefs.h:userdefs.h	1.7.1.1"

/* User/group default values */
#define	DEFGID		99	/* max reserved group id */
#define	DEFRID		99
#define	DEFGROUP	1
#define	DEFGNAME	"other"
#define	DEFPARENT	"/home"
#define	DEFSKL		"/etc/skel"
#define	DEFSHL		"/sbin/sh"
#define	DEFINACT	0
#define	DEFEXPIRE	""

/* Defaults file keywords */
#define	RIDSTR		"defrid="
#define	GIDSTR		"defgroup="
#define	GNAMSTR		"defgname="
#define	PARSTR		"defparent="
#define	SKLSTR		"defskel="
#define	SHELLSTR	"defshell="
#define	INACTSTR	"definact="
#define	EXPIRESTR	"defexpire="
#define	FHEADER		"#	Default values for useradd.  Changed "

/* Defaults file */
#define	DEFFILE		"/usr/sadm/defadduser"
#define	GROUP	"/etc/group"

/* various limits */
#define	MAXGLEN		9	/* max length of group name */
#define	MAXDLEN		80	/* max length of a date string */

/* defaults structure */
struct userdefs {
	int defrid;		/* highest reserved uid */
	int defgroup;		/* default group id */
	char *defgname;		/* default group name */
	char *defparent;	/* default base directory for new logins */
	char *defskel;		/* default skel directory */
	char *defshell;		/* default shell */
	int definact;		/* default inactive */
	char *defexpire;		/* default expire date */
};

/* exit() values for user/group commands */

/* Everything succeeded */
#define	EX_SUCCESS	0

/* No permission */
#define	EX_NO_PERM	1

/* Command syntax error */
#define	EX_SYNTAX	2

/* Invalid argument given */
#define	EX_BADARG	3

/* A gid or uid already exists */
#define	EX_ID_EXISTS	4

/* PASSWD and SHADOW are inconsistent with each other */
#define	EX_INCONSISTENT	5

/* A group or user name  doesn't exist */
#define	EX_NAME_NOT_EXIST	6

/* GROUP, PASSWD, or SHADOW file missing */
#define	EX_MISSING	7

/* GROUP, PASSWD, or SHAWOW file is busy */
#define	EX_BUSY	8

/* A group or user name already exists */
#define	EX_NAME_EXISTS	9

/* Unable to update GROUP, PASSWD, or SHADOW file */
#define	EX_UPDATE	10

/* Not enough space */
#define	EX_NOSPACE	11

/* Unable to create/remove/move home directory */
#define	EX_HOMEDIR	12

/* new login already in use */
#define	EX_NL_USED	13

/* Unexpected failure */
#define	EX_FAILURE	14
