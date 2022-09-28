/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/messages.c	1.6.4.1"



char *errmsgs[] = {
	"WARNING: uid %ld is reserved.\n",
	"WARNING: more than NGROUPS_MAX(%d) groups specified.\n",
	"ERROR: invalid syntax.\nusage:  useradd [-u uid [-o] | -g group | -G group[[,group]...] | -d dir |\n                -s shell | -c comment | -m [-k skel_dir] | -f inactive | -e expire ] login\n        useradd -D [-g group | -b base_dir | -f inactive | -e expire ]\n",
	"ERROR: Invalid syntax.\nusage:  userdel [-r] login\n",
	"ERROR: Invalid syntax.\nusage:  usermod -u uid [-o] | -g group | -G group[[,group]...] |\n                -d dir [-m] | -s shell | -c comment |\n                -l new_logname | -f inactive | -e expire login\n",
	"ERROR: Unexpected failure.  Defaults unchanged.\n",
	"ERROR: Unable to remove files from home directory.\n",
	"ERROR: Unable to remove home directory.\n",
	"ERROR: Cannot update system files - login cannot be %s.\n",
	"ERROR: uid %ld is already in use.  Choose another.\n",
	"ERROR: %s is already in use.  Choose another.\n",
	"ERROR: %s does not exist.\n",
	"ERROR: %s is not a valid %s.  Choose another.\n",
	"ERROR: %s is in use.  Cannot %s it.\n",
	"WARNING: %s has no permissions to use %s.\n",
	"ERROR: There is not sufficient space to move %s home directory to %s\n",
	"ERROR: %s %ld is too big.  Choose another.\n",
	"ERROR: group %s does not exist.  Choose another.\n",
	"ERROR: Unable to %s: %s.\n",
	"ERROR: %s is not a full path name.  Choose another.\n",
	"ERROR: %s is the primary group name.  Choose another.\n",
	"ERROR: Inconsistent password files.  See pwconv(1M).\n"
};

int lasterrmsg = sizeof( errmsgs ) / sizeof( char * );
