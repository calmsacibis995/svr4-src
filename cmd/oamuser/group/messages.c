/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/messages.c	1.2.7.1"



char *errmsgs[] = {
	"WARNING: gid %ld is reserved.\n",
	"ERROR: invalid syntax.\nusage:  groupadd [-g gid [-o]] group\n",
	"ERROR: invalid syntax.\nusage:  groupdel group\n",
	"ERROR: invalid syntax.\nusage:  groupmod -g gid [-o] | -n name group\n",
	"ERROR: Cannot update system files - group cannot be %s.\n",
	"ERROR: %s is not a valid group id.  Choose another.\n",
	"ERROR: %s is already in use.  Choose another.\n",
	"ERROR: %s is not a valid group name.  Choose another.\n",
	"ERROR: %s does not exist.\n",
	"ERROR: Group id %ld is too big.  Choose another.\n"
};

int lasterrmsg = sizeof( errmsgs ) / sizeof( char * );
