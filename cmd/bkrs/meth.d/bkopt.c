/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/bkopt.c	1.12.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<string.h>
#include	<bkrs.h>
#include	<method.h>

extern int	brinit();
extern int	brlog();
extern int	brsndfname();
extern int	chdir();
extern int	getopt();
extern int	rstocname();
extern long	strtol();

extern int	brstate;			/* libmeth state */
#ifdef DEBUG
extern char	broambase[];
#endif
static char	*oam_methods[] = {
			"ffile",
			"incfile",
			"fdp",
			"fdisk",
			"fimage",
			"migration"
};    /* names we go by */

static int	nmethod = sizeof( oam_methods )/sizeof( char * );

static char	*opt_list[] = {
			"BRFCde:lmorstvAENSV",
			"BRFCde:ilmop:rstvxAENSVT",
	    		"BRCc:doqvAENSV",
			"BRCdovAENV",
			"BRFCdlmnoqstvAENSV",
			"BNEt:V"
};

int
bkopt(argc, argv, mp)
int			argc;
unsigned char		*argv[];
register m_info_t	*mp;
{
	char		*opt;
	char		*c_arg;
	register int	c;
	extern char	*optarg;
	extern int	opterr;
	int		idx;
	int		error=0;
	int		n_unknown=0;

	opterr = 0;				/* no msg from getopt */

	if ((opt = strrchr(mp->method_name, '/')))
		mp->method_name = ++opt;	/* file only */

	for (c = 0; c < nmethod; c++) {	/* see if name we know */
		if (!strcmp(oam_methods[c], mp->method_name))
			break;
	}
	if (c == nmethod) {
		(void) brinit(mp->method_name,
			(int) BACKUP_T ); /* lie about type */
		brlog("bkopt: %s unknown name ",MN(mp));
		return(1);		/* unknown name */
	}
	idx = c;
	mp->meth_type = (short) c;

	while ((c = getopt( argc, (char **) argv, opt_list[idx])) != -1) {

		switch (c) {
		case 'B':		/* is a backup */
			mp->br_type |= IS_BACKUP;
			break;
		case 'R':		/* is restore */
			mp->br_type |= IS_RESTORE;
			break;
		case 'F':		/* is file restore */
			mp->br_type |= IS_RFILE;
			break;
		case 'C':		/* complete restore */
			mp->br_type |= IS_RCOMP;
			break;
		case 'A':		/* automated operation */
			mp->flags |= Aflag;
			break;
		case 'c':		/* blk count for data partition */
			mp->flags |= cflag;
			c_arg = optarg;
			break;
		case 'd':		/* no history log */
			mp->flags |= dflag;
			break;
		case 'e':		/* exception list file */
			mp->flags |= eflag;
			mp->ex_tab = (char **)optarg;
			break;
		case 'i':		/* exclude inode changes */
			mp->flags |= iflag;
			break;
		case 'l':		/* long history log */
			mp->flags |= lflag;
			break;
		case 'm':		/* mount read only */
			mp->flags |= mflag;
			break;
		case 'o':		/* permit override */
			mp->flags |= oflag;
			break;
		case 'p':		/* days prior for backups */
			mp->flags |= pflag;

			if (atoi(optarg) < 0) {
				brlog(" argument to -p cannot be < 0");
				return (1);
			}
			if (atoi(optarg) == 0)
				mp->lastfull = 0;
			else
				mp->lastfull = time((long *) 0) - 60*60*24*atoi(optarg);
			break;
		case 'q':		/* quick fdp/fimage method use g_copy */
			mp->flags |= qflag;
			break;
		case 'r':		/* include remote files */
			mp->flags |= rflag;
			break;
		case 's':		/* no table of contents online */
			mp->flags |= sflag;
			break;
		case 't':		/* table of contents on media */
			if (mp->meth_type == IS_MIGRATION) {
				mp->tocfname = optarg;
			}
			mp->flags |= tflag;
			break;
		case 'n':		/* don't umount fs during backup */
			mp->flags |= nflag;
			break;
		case 'v':		/* validate */
			mp->flags |= vflag;
			break;
		case 'x':		/* ignore exception list */
			mp->flags |= xflag;
			break;
		case 'E':		/* estimate, proceed */
			mp->flags |= Eflag;
			break;
		case 'N':		/* estimate, stop */
			mp->flags |= Nflag;
			break;
		case 'S':		/* put out dots */
			mp->flags |= Sflag;
			break;
		case 'V':		/* generate names */
			mp->flags |= Vflag;
			mp->sndfp = brsndfname;
			break;
		case 'T':		/* table of contents */
			mp->br_type |= IS_TOC;
			break;
		case '?':		/* unknown option letter */
#ifdef TRACE
			brlog(" Unknown option = %c ", c);
#endif
			n_unknown++;
			break;
		default:		/* unexpected return */
			;
		} /* switch */
	}
	(void) brinit(mp->method_name, (int)(IS_R(mp->br_type) ? RESTORE_T : BACKUP_T));

	if (!(mp->br_type & IS_BOTH)) {
		brlog(" neither B nor R specified ");
		return (1);
	}
	if ((mp->br_type & IS_BOTH) == IS_BOTH) {
		brlog(" both B and R specified ");
		return (1);
	}
	if (IS_R(mp->br_type)) {		/* if its a restore */
		mp->flags |= REST_IGNORE;	/* be sure all on */
		mp->flags ^= REST_IGNORE;	/* be sure all off */

		if (!(mp->br_type & IS_FC)) {
			brlog(" neither F or C for restore ");
			error++;
		}
		if ((mp->br_type & IS_FC) == IS_FC) {
			brlog(" both F and C for restore ");
			error++;
		}
		if (mp->br_type & IS_TOC) {
			mp->flags |= Vflag;
			mp->sndfp = rstocname;
		}
	}
	else {		/* check for bad combinations (backup) */
		if (mp->flags&tflag) {
			if (mp->flags&Aflag) {
			 	brlog(" -t used with -A ");
				error++;
			}
		}
		if ((mp->flags&(lflag|dflag)) == (lflag|dflag)) {
			brlog(" both -l and -d specified ");
			error++;
		}
	}		/* end backup conflict checking */
	if ((mp->flags&(Aflag|oflag)) == (Aflag|oflag)) {
		brlog(" both -A and -o specified ");
		error++;
	}
	if (n_unknown) {
		brlog(" unknown option specified ");
	}
	if (mp->flags & cflag) {
		mp->c_count = (int)strtol(c_arg, &opt, 0);
		if (c_arg == opt) {
	               brlog(" -c %s not numeric ", c_arg);
		       error++;
		}
		else if (mp->c_count <= 0) {
			brlog(" count <= 0 specified ");
			error++;
		}
	}
	if (error || n_unknown) {
		return(1);
	}
#ifdef DEBUG
	brlog("broambase = %s",broambase);
	(void) chdir(broambase);
#endif
	return (0);
} /* bkopt() */
