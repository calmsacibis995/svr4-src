/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)shl:defs.c	1.5.4.1"

#include	"defs.h"

struct layer 	*layers[MAX_LAYERS];

int max_index = 0;
int chan = 0;
int fildes[2];
int real_tty_fd;
	
int stream=1;
char *cntlf;
char ocntl_fl[]	= "/dev/sxto/000";
char cntl_fl[]	= "/dev/sxt/000";
char vcntl_fl[]	= "/dev/vsxt/000";

int	cntl_chan_fd;
	
struct utmp	  *u_entry;
struct termio ttysave;

uid_t uid;
gid_t gid;
