/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/mountd/sharetab.h	1.2.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *	PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
struct share {
	char *sh_path;
	char *sh_res;
	char *sh_fstype;
	char *sh_opts;
	char *sh_descr;
};

#define SHARETAB  "/etc/dfs/sharetab"

/* generic options */
#define SHOPT_RO	"ro"
#define SHOPT_RW	"rw"

/* options for nfs */
#define SHOPT_ROOT	"root"
#define SHOPT_ANON	"anon"
#define SHOPT_SECURE	"secure"
#define SHOPT_WINDOW	"window"

int		getshare();
int		putshare();
int		remshare();
char *		getshareopt();
