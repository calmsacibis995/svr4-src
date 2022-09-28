/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/incfile.c	1.11.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<string.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>

extern int	backup_toc();
extern int	bkopt();
extern int	bld_except();
extern void	br_toc_close();
extern int	br_toc_open();
extern int	brestimate();
extern int	brlog();
extern int	brreturncode();
extern int	cmdargs();
extern int	do_comprest();
extern int	do_cpio();
extern int	do_filerest();
extern void	exit();
extern int	find();
extern int	fsarg_check();
extern long	last_full_date();
extern void	mi_init();
extern void	remount();
extern int	unlink();
extern int	TLsync();

m_info_t	mi;			/* method info structure */
m_info_t	*mp = &mi;		/* pointer to info structure */

char	tocname[PATH_MAX+1];		/* table of contents path */

main(argc, argv)
int		argc;
unsigned char *argv[];
{
	register	i;
	extern int	optind;
	int		toc = -1;	/* toc stuff */

	mp->method_name = (char *) argv[0];	/* full command name */

	mi_init(mp);				/* init method info */
	
	if (i = bkopt(argc, argv, mp)) {	/* do options */
		sprintf(ME(mp), "bad command line options" );
		brreturncode(BRBADOPTS, 0, ME(mp));
		exit(i);
	}
	mp->noptarg = argc - optind;		/* number of cmdargs */

	if (i = cmdargs(argc, argv, mp)) {	/* process cmdargs */
		brreturncode(BRBADCMDS, 0, ME(mp));
		exit(1);
	}
	if ((mp->br_type & IS_BTOC) == IS_BTOC) {	/* move toc to media */
		*tocname = '\0';
		toc = backup_toc(mp, tocname);
		if (toc < 0) {
			(void) unlink(tocname);
			(void) brreturncode(BRBADTOC, 0, ME(mp));
			exit(2);
		}
		if (i = do_cpio(mp, toc)) {
			(void) unlink(tocname);
			(void) brreturncode(BRUNSUCCESS, 0, ME(mp));
			exit(2);
		}
		(void) unlink(tocname);

		brreturncode(BRSUCCESS, mp->blk_count, "");
		exit (0);
	}
	else if (mp->br_type == IS_BACKUP) {		/* do the work */
		if (i = fsarg_check(mp)) {		 /* verify cmd args */
			brreturncode(BRBADFSCK, 0, ME(mp));
			remount(mp);
			exit(i);
		}
		if (mp->meth_type == IS_INC) {		/* incremental */
			mp->lastfull = last_full_date(mp);
			if (mp->lastfull <= 0) {
				brlog("no full backup for %s", mp->ofsname);
				sprintf(ME(mp), "Job ID %s: backup not found for %s", mp->jobid, mp->ofsname);
				brreturncode(BRBADFSCK, 0, ME(mp));
				remount(mp);
				exit(2);
			}
			if (!(mp->flags & xflag)) {	/* no ignore */
				if (i = bld_except(mp)) {
					sprintf(ME(mp), "Job ID %s: exception list error", mp->jobid);
					brreturncode(BRBADEXCEPT, 0, ME(mp));
					remount(mp);
					exit(i);
				}
			}
		}
		/* open table of contents */
		toc = br_toc_open(1, mp->jobid, tocname);

		if (toc >= 0) {
			if (i = find(mp, toc)) {	/* wander the fs */
				brreturncode(i, 0, ME(mp));
				(void) unlink(tocname);
				exit(2);
			}
			else if (mp->flags & Nflag) {
				brestimate(((mp->blks_per_vol) > 0) ?
						 NMEDIA(mp) : 0, mp->blk_count);
				brreturncode(BRSUCCESS, 0, ME(mp));
				(void) unlink(tocname);
				remount(mp);
				exit(0);
			}
			else {
				(void) TLsync( toc );
				if (mp->flags & Eflag) {
					brestimate(((mp->blks_per_vol) > 0) ?
						NMEDIA(mp) : 0, mp->blk_count);
				}
				mp->estimate = mp->blk_count;
				i = do_cpio(mp, toc);
				br_toc_close();
				if (i) {
					brreturncode(i, 0, ME(mp));
					(void) unlink(tocname);
					remount(mp);
					exit(2);
				}
			}
		}
		else {
			sprintf(ME(mp), "Job ID %s: cannot create table of contents", mp->jobid);
			brreturncode(BRBADTOC, 0, ME(mp));
			remount(mp);
			exit(1);
		}
	}
	else {
		if (ISFILE_REST(mp)) {		/* -RF for files */
			i = do_filerest(mp, argv);
			sprintf(ME(mp), "Job ID %s: error during incfile -RF", mp->jobid);
		}
		else {				/* -RC for complete */
			i = do_comprest(mp);
			sprintf(ME(mp), "Job ID %s: error during incfile -RC", mp->jobid);
		}
	}
	if (i) {
		brreturncode(BRUNSUCCESS, mp->blk_count, ME(mp));
	}
	else {
		brreturncode(BRSUCCESS, mp->blk_count, "");
		brlog("%s: successful blocks=%d",MN(mp),mp->blk_count);
	}
	brlog("exit code = %d",i);

	remount(mp);
	exit(i);
/*NOTREACHED*/
} /* main() */
