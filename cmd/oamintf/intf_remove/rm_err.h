/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_remove/rm_err.h	1.1.1.2"

#define USAGE		0
#define INV_FILE	1
#define FILE_OPN 	2
#define FILE_RD		3
#define FILE_WR		4
#define D_CREAT		5
#define FILE_CLS	6
#define RM_ERR		7

#define NONE		10

#define HAS_STR		INV_FILE	/* string argument above this pos */
#define HAS_TWO		NONE	/* two string arguments */

#define WARN		0
#define ERR		1

#define ERR_RET		1
#define SUCCESS_RET	0
