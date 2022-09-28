/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/mi_init.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<method.h>

char	e[160];		/* error string for standard error */
			/* two 80 character lines          */

void
mi_init(mp)
register m_info_t	*mp;
{

	mp->jobid = NULL;		/* backup job id */
	mp->ofsname = NULL;		/* originating file system name */
	mp->ofsdev = NULL;		/* originating file system device */
	mp->ofslab = NULL;		/* originating file system label */
	mp->dgroup = NULL;		/* dest device dgroup description */
	mp->flags = 0;			/* set from command options */
	mp->br_type = 0;		/* backup or restore */
	mp->c_count = 0;		/* block count for image backup */
	mp->noptarg = 0;		/* number of cmd args */
	mp->n_names = 0;		/* num file names if -RF */
	mp->fnames = 0;			/* offset in argv to first name */
	mp->ofs_loc = 0;		/* is ofs local or remote */
	mp->ofs_mntopts = NULL;		/* ofs mount options */
	mp->mntinfo = DONOTHING;	/* no mount info yet */
	mp->meth_type = -1;		/* index of method type */
	mp->ex_count = 0;		/* index of method type */
	mp->err = e;			/* optimistic error string */
	(void) sprintf(mp->err, "success");
	mp->blk_count = 0;		/* blocks written */
	mp->nfsname = NULL;		/* -RC nfsdev */
	mp->nfsdev = NULL;		/* -RC nfsdev */
	mp->nfsdevmnt = NULL;		/* fs mounted on nfsdev */
	mp->ofsdevmnt = NULL;		/* fs mounted on ofsdev */
	mp->fstype = NULL;		/* fs type string */
} /* mi_init() */
