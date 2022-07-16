/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/ckperms/defs.h	1.3"

/*	return codes from scan	*/

#define		Error	99
#define		Token	0
#define		NewLine 1
#define		EndOfFile 2
#define		Comment	3

/*	Token definitions	*/

#define		NL	'\n'
#define		BACKSL	'\\'
#define		COMMNT	'#'

/* other defines	*/

#define		BUFSIZE	256
#define		CP_MAXLINK 32
#define		MAXID	20
#define		MAXTP	9
#define		MAXINOD	100
#define		ARGSIZ	20
#define		CP_PMASK	0007777
#define		SAMLN	0
#define		INCLN	1
#define		STRING	0
#define		RECORD	2

/*  errors */

#define		DERR	1
#define		FERR	2
#define		PERR	3
#define		WARN	0

/*	record for a single entry in file		*/

struct	perm {
	char	p_id[BUFSIZE];
	char	p_ftype;		/* file type 	*/
	int	p_perm;			/* permissions 	*/
	int	p_owner;		/* owner	*/
	int	p_group;		/* group	*/
	char	p_sflag;		/* flag to ignore size field */
	long	p_size;			/* size of file */
	char	p_cflag;		/* flag to ignore cksum field*/
	int	p_ck1_maj;		/* 1st check sum*/
					/* or major no	*/
	int	p_ck2_min;		/* 2nd check sum*/
					/* or minor no	*/
	int	p_nolink;		/* link count	*/
	char	*p_path[CP_MAXLINK];	/* pathname	*/
} ;

/*	field identifiers			*/

#define		TFIELDS 9
#define		ID	0
#define		FT	1
#define		PERM	2
#define		UID	3
#define		GID	4
#define		SIZE	5
#define		CSUM1	6
#define		CSUM2	7
#define		LINK	8
#define		PATH	9


/*	flags for perms rec 	*/

#define		F	0x01
#define		T	0x02
