/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/migration.c	1.6.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<fcntl.h>
#include	<table.h>
#include	<stdio.h>
#include	<string.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>
#include	<errno.h>

extern int	bkopt();
extern void	br_toc_close();
extern int	br_toc_open();
extern int	brlog();
extern int	brreturncode();
extern int	cmdargs();
extern void	do_history();
extern int	do_migration();
extern int	do_remount();
extern void	exit();
extern int	find();
extern void	mi_init();
extern void	remount();
extern int	rsresult();
extern int	unlink();

extern int		optind;
extern media_info_t	IMM;

char	tocname[PATH_MAX];		/* table of contents path */

m_info_t	mi;				/* method info structure */
m_info_t	*mp = &mi;			/* pointer to info structure */

static void	yo_history();

main(argc, argv)
unsigned char	*argv[];
{
	int	i;
	long	sav_blkcount;
	int	tid;
	char	*tmp;

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
	if ((mp->br_type != IS_BACKUP) || (mp->meth_type != IS_MIGRATION)) {
		brlog("Migration: Illegal call br_type=0X%x, meth_type=0X%x",mp->br_type, mp->meth_type);
		exit(1);
	}
	if (mp->flags & tflag) {
		brlog("Migration: update TOC %s",mp->tocfname);
		if (TLopen(&tid, mp->tocfname, NULL, O_RDWR) != TLOK) {
			brlog("Migration: Cannot open %s",mp->tocfname);
			sprintf(ME(mp), "Job ID %s: cannot open %s: %s", mp->jobid, mp->tocfname, SE);
			brreturncode(BRBADTOC, 0, ME(mp));
			exit(2);
		}
	}
	i = do_migration(mp, 0);
#ifdef TRACE
	brlog("Migration: Complete (i=%d)",i);
#endif
	if (i) {
		brreturncode(i, 0, ME(mp));
		exit(2);
	}
	yo_history(NULL);

	if (mp->flags & tflag) {				/* write toc */
		int	entry = TLBEGIN;
		ENTRY	eptr;

		brlog("Migration: Do the TOC");

		if ((eptr = TLgetentry(tid)) == NULL) {
			sprintf(ME(mp), "Job ID %s: cannot allocate toc entry", mp->jobid);
			brreturncode(i, 0, ME(mp));
			exit(2);
		}
		while (TLread(tid, entry++, eptr) == TLOK) {
			if (TLgetfield(tid, eptr, "fname") == NULL) {
				continue;
			}
			if (TLassign(tid, eptr, "vol", "") != TLOK) {
				brlog("Migration: TOC entry assignment failed");
			}
			if (TLwrite(tid, (entry-1), eptr) != TLOK) {
				brlog("Migration: TOC entry write failed");
			}
		}
		if (TLfreeentry(tid, eptr) != TLOK) {
			brlog("Migration: Cannot free TOC entry");
		}
		if (TLsync(tid) != TLOK) {
			brlog("Migration: Cannot sync TOC");
		}
		if (TLclose(tid) != TLOK) {
			brlog("Migration: Cannot close TOC");
		}
		tmp = mp->ofsdev;
		mp->ofsdev = mp->tocfname;
		i = do_migration(mp, 1);
		mp->ofsdev = tmp;

		if (i) {
			brlog("do_migration returned %d, exit(2)", i);
			brreturncode(i, 0, ME(mp));
			exit(2);
		}
	}
	yo_history(1);
	brreturncode(BRSUCCESS, mp->blk_count, "successful");
	remount(mp);
	exit(0);
/*NOTREACHED*/
} /* main() */

static void
yo_history(toc)
int	toc;
{
	media_list_t	*ml;
	int		nv = 0;
	char		**lab;

#ifdef TRACE
	brlog("Migration: YO_HISTORY!");
#endif
	ml = IMM.first;

	while (ml != NULL) {
		nv++;
		ml = ml->next;
	}
	if (nv) {
		lab = (char **)malloc( nv * sizeof(char *));
		if (lab == NULL) {
			brlog("malloc failed for labels history: %s", SE);
		}
		else {
			char	**l = lab;

			ml = IMM.first;

			while (ml != NULL) {
				*l++ = ml->label;
				ml = ml->next;
			}
		}
	}
	brlog("brhistory call: nv=%d, lab=0x%x, *lab=%s", nv, lab, *lab);
	(void) brhistory(mp->ofsname, mp->ofsdev, mp->bkdate,
		(int)(mp->blk_count), lab, nv,
		BR_IS_OLD_ENTRY|(toc ? BR_IS_TMNAMES : NULL), NULL);
	IMM.first = IMM.last = IMM.cur = NULL;
	IMM.bytes_left = 0;
} /* yo_history() */
