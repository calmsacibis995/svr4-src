/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/messages.h	1.4.4.1"



/* WARNING: uid %d is reserved. */
#define M_RESERVED		0

/* WARNING: more than NGROUPS_MAX(%d) groups specified. */
#define M_MAXGROUPS	1

/* ERROR: invalid syntax.\nusage:  useradd [-u uid [-o] | -g group | -G group[[,group]...] | -d dir |\n                -s shell | -c comment | -m [-k skel_dir]] login\n        useradd -D [-g group | -b base_dir | -r rid] */
#define M_AUSAGE		2

/* ERROR: Invalid syntax.\nusage:  userdel [-r] login */
#define M_DUSAGE		3

/* ERROR: Invalid syntax.\nusage:  usermod -u uid [-o] | -g group | -G group[[,group]...] | -d dir [-m] |\n                -s shell | -c comment | -l new_logname login */
#define M_MUSAGE		4

/* ERROR: Unexpected failure.  Defaults unchanged. */
#define M_FAILED	5

/* ERROR: Unable to remove files from home directory. */
#define M_RMFILES	6

/* ERROR: Unable to remove home directory. */
#define M_RMHOME		7

/* ERROR: Cannot update system files - login cannot be %s. */
#define M_UPDATE		8

/* ERROR: uid %d is already in use.  Choose another. */
#define M_UID_USED	9

/* ERROR: %s is already in use.  Choose another. */
#define M_USED	10

/* ERROR: %s does not exist. */
#define M_EXIST	11

/* ERROR: %s is not a valid %s.  Choose another. */
#define M_INVALID		12

/* ERROR: %s is in use.  Cannot %s it. */
#define M_BUSY	13

/* WARNING: %s has no permissions to use %s. */
#define M_NO_PERM	14

/* ERROR: There is not sufficient space to move %s home directory to %s */
#define M_NOSPACE		15

/* ERROR: %s %d is too big.  Choose another. */
#define	M_TOOBIG	16

/* ERROR: group %s does not exist.  Choose another. */
#define	M_GRP_NOTUSED	17

/* ERROR: Unable to %s: %s */
#define	M_OOPS	18

/* ERROR: %s is not a full path name.  Choose another. */
#define	M_RELPATH	19

/* ERROR: %s is the primary group name.  Choose another. */
#define	M_SAME_GRP	20

/* ERROR: Inconsistent password files.  See pwconv(1M). */
#define	M_HOSED_FILES	21
