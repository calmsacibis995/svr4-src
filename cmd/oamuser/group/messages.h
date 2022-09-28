/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/messages.h	1.1.9.1"



/* WARNING: gid %d is reserved.\n */
#define M_RESERVED		0

/* ERROR: invalid syntax.\nusage:  groupadd [-g gid [-o]] group\n */
#define M_AUSAGE		1

/* ERROR: invalid syntax.\nusage:  groupdel group\n */
#define M_DUSAGE		2

/* ERROR: invalid syntax.\nusage:  groupmod -g gid [-o] | -n name group\n */
#define M_MUSAGE		3

/* ERROR: Cannot update system files - group cannot be %s.\n */
#define M_UPDATE		4

/* ERROR: %s is not a valid group id.  Choose another.\n */
#define M_GID_INVALID	5

/* ERROR: %s is already in use.  Choose another.\n */
#define M_GRP_USED	6

/* ERROR: %s is not a valid group name.  Choose another.\n */
#define M_GRP_INVALID	7

/* ERROR: %s does not exist.\n */
#define M_NO_GROUP	8

/* ERROR: Group id %d is too big.  Choose another.\n */
#define M_TOOBIG	9
