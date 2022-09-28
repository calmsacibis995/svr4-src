/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:inc/users.h	1.4.4.1"



#define GROUP		"/etc/group"

/* validation returns */
#define NOTUNIQUE	0	/* not unique */
#define RESERVED	1	/* reserved */
#define UNIQUE		2	/* is unique */
#define	TOOBIG	3
#define	INVALID	4

/* Exit codes from passmgmt(1) */
#define	PEX_SUCCESS	0
#define	PEX_NO_PERM	1
#define	PEX_SYNTAX	2
#define	PEX_BADARG	3
#define	PEX_BADUID	4
#define	PEX_HOSED_FILES	5
#define	PEX_FAILED	6
#define	PEX_MISSING	7
#define	PEX_BUSY	8
#define	PEX_BADNAME	9

#define	REL_PATH(x)	(x && *x != '/')
