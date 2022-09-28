/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/fdisk.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<string.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>
#include	<errno.h>

extern int	bkopt();
extern int	brestimate();
extern int	brlog ();
extern int	brreturncode();
extern int	cmdargs();
extern int	do_bkfdisk();
extern int	do_rsfdisk();
extern void	exit();
extern void	mi_init();
extern int	rsresult();

extern int	optind;

m_info_t	mi;				/* method info structure */
m_info_t	*mp = &mi;			/* pointer to info structure */

main( argc, argv )
unsigned char *argv[];
{
	int	i;
	struct stat	st;

	mp->method_name = (char *) argv[0];	/* full command name */
	mi_init(mp);				/* init method info */
	
	if ((i = bkopt(argc, argv, mp))) {	/* do options */
		sprintf(ME(mp), "bad command line options" );
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(1);
	}
	mp->noptarg = argc - optind;		/* number of cmdargs */

	if (mp->br_type & IS_RFILE) {		/* only -RC , no -RF */
		sprintf(ME(mp), "Job ID %s: -RF not allowed", mp->jobid);
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(1);
	}
	if ((i = cmdargs(argc, argv, mp))) {	/* process cmdargs */
		brreturncode(BRBADCMDS, 0, ME(mp));
		exit(1);
	}
	i = stat(mp->ofsdev, &st);	/* see if char special */

	if (i) {
		brlog("stat of partdev %s failed %s",mp->ofsdev,SE);
		sprintf(ME(mp), "Job ID %s: stat failed for %s: %s", mp->jobid, mp->ofsdev, SE);
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFCHR) {
		brlog("partdev %s not character special", mp->ofsdev);
		sprintf(ME(mp), "Job ID %s: %s not character special", mp->jobid, mp->ofsdev);
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(1);
	}
	if (mp->br_type == IS_BACKUP) {		/* fdisk backup */

		if (mp->flags & (Eflag | Nflag)) {
			brestimate(1, 1);	/* small media if more */

			if (mp->flags & Nflag) {
				sprintf(ME(mp), "Job ID %s: estimate completed", mp->jobid);
				brreturncode(BRSUCCESS, 0, ME(mp));
				exit(0);
			}
		}
		mp->estimate = (long) 1;
		i = do_bkfdisk(mp);		/* drub, drub, drub */

		if (i) {
			brlog("fdisk -B failed %d", i);
			sprintf(ME(mp), "Job ID %s: -B failed", mp->jobid);
			brreturncode(BRUNSUCCESS, 0, ME(mp));
			exit(2);
		}
	}
	else {					/* is a restore */
		i = do_rsfdisk(mp);		/* burd, burd, burd */

		brlog("finished blocks=%d i=%d id=%s",
				mp->blk_count,i,mp->jobid);
		rsresult(mp->jobid, i ? BRUNSUCCESS : BRSUCCESS,
				    i ? "unsuccessful" : "successful");
		if (i) {
			sprintf(ME(mp), "Job ID %s: failed", mp->jobid);
			brreturncode(BRUNSUCCESS, 0, ME(mp));
			exit(1);
		}
	}
	brreturncode(BRSUCCESS, mp->blk_count, ME(mp));

	exit(0);
/*NOTREACHED*/
} /* main() */
