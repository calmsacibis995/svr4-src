/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/fimage.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>

extern int	bkopt();
extern void	br_toc_close();
extern int	br_toc_open();
extern int	brlog();
extern int	brreturncode();
extern int	cmdargs();
extern void	do_history();
extern int	do_image_backup();
extern int	do_image_comprest();
extern int	do_image_filerest();
extern int	do_remount();
extern void	exit();
extern int	find();
extern int	fsarg_check();
extern void	mi_init();
extern void	remount();
extern int	rsresult();
extern int	unlink();

extern int		optind;
extern media_info_t	IMM;

char tocname[PATH_MAX];		/* table of contents path */

m_info_t	mi;		/* method info structure */
m_info_t	*mp = &mi;	/* pointer to info structure */

main(argc, argv)
int		argc;
unsigned char	*argv[];
{
	int	i;
	long	sav_blkcount;
	int	toc = -1;

	mp->method_name = (char *) argv[0];	/* full command name */
	mi_init(mp);				/* init method info */
	
	if ((i = bkopt(argc, argv, mp))) {	/* do options */
		sprintf(ME(mp), "bad command line options" );
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(1);
	}
	mp->noptarg = argc - optind;		/* number of cmdargs */

	if ((i = cmdargs(argc, argv, mp))) {	/* process cmdargs */
		brreturncode(BRBADCMDS, 0, ME(mp));
		exit(1);
	}
	if (mp->br_type == IS_BACKUP) {		/* fimage or fdp backup */

		if (mp->meth_type == IS_IMAGE) {
			if (i = fsarg_check(mp)) {	/* verify cmd args */
				brreturncode(BRBADFSCK, 0, ME(mp));
				remount(mp);
				exit(1);
			}
		}
		if ((mp->meth_type == IS_IMAGE) &&
			    ((mp->flags & tflag) || !(mp->flags & sflag))) {
			toc = br_toc_open(1, mp->jobid, tocname);

			if (toc < 0) {
				sprintf(ME(mp), "Job ID %s: cannot create table of contents", mp->jobid);
				brreturncode(BRBADTOC, 0, ME(mp));
				remount(mp);
				exit(2);
			}
		}
		if (i = do_image_backup(mp)) {
			brreturncode(i, 0, ME(mp));
			remount(mp);
			exit(2);
		}
		if (toc >= 0) {				/* write toc */
			if (!(mp->flags & nflag)) {	/* must mount to toc */
				if (i = do_remount(mp, "-r")) {	/* read only */
					brlog("table of contents mount failed");
					sprintf(ME(mp), "Job ID %s: toc mount failed", mp->jobid);
					brreturncode(i, 0, ME(mp));
					remount(mp);
					exit(2);
				}
			}
			sav_blkcount = mp->blk_count;
			i = find(mp, toc);		/* wander the fs */
			mp->blk_count = sav_blkcount;

			if (i) {
				brreturncode(i, 0, ME(mp));
				(void) unlink(tocname);
				remount(mp);
				exit(2);
			}
			else {
				br_toc_close();
			}
		}
		do_history(mp, &IMM, -1);
	}
	else {					/* is a restore */
		if (ISFILE_REST(mp)) {		/* -RF for files */
			i = do_image_filerest(mp, argv);
		}
		else {				/* -RC for complete */
			i = do_image_comprest(mp, 0);
			rsresult(mp->jobid, i ? BRUNSUCCESS : BRSUCCESS,
				    i ? ME(mp) : "successful");
		}
		brlog("finished blocks=%d i=%d",mp->blk_count,i);

		if (i) {
			brreturncode(BRUNSUCCESS, 0, ME(mp));
		}
		exit(i);
	}
	brreturncode(BRSUCCESS, mp->blk_count, "successful");

	remount(mp);
	exit(0);
/*NOTREACHED*/
} /* main() */
