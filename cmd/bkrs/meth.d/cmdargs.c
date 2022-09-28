/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/cmdargs.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stddef.h>
#include	<string.h>
#include	<bkrs.h>
#include	<method.h>

DTYPES;

extern long	atol();
extern int	brlog();
extern char	*bkstrtok();
extern char	*devattr();
extern int	strfind();
extern int	strfind();

extern char	*optarg;
extern int	optind;

static long	get_dcap();
static int	get_dtype();
static char *mystrtok();

static char *cont_ptr;

int
cmdargs(argc , argv, mp)
int	argc;
unsigned char	*argv[];
register m_info_t	*mp;
{
	register	wk;
	char		*c;

#ifdef TRACE
	{
		int	i;

		for (i=0; i<argc; i++) {
			brlog("cmdarg: argv[%d] = %s  ",i,argv[i]);
		}
	}
#endif
	if ((mp->br_type) & IS_BACKUP) {
		if ((mp->br_type) & IS_TOC) {
			if (mp->noptarg < 6) {
				brlog("-BT specified, only %d optargs",
							mp->noptarg);
				sprintf(ME(mp), "Job ID %s: -BT specified with only %d arguments", mp->jobid, mp->noptarg);
				return(1);
			}
		}
		else if (mp->noptarg < 5) {
			brlog(" -B only %d optargs ", mp->noptarg);
			sprintf(ME(mp), "Job ID %s: -B specified with only %d arguments", mp->jobid, mp->noptarg);
			return(1);
		}
		wk = optind;
		mp->jobid = (char *)argv[wk++];
		mp->ofsname = (char *)argv[wk++];
		mp->ofsdev = (char *)argv[wk++];
		mp->ofslab = (char *)argv[wk++];
		mp->dgroup = mystrtok(argv[wk++], ":");
		mp->dname = mystrtok(NULL, ":");
		mp->dchar = mystrtok(NULL, ":");
		mp->dmnames = mystrtok(NULL, ":");

		if (mp->br_type & IS_TOC) {
			mp->tocfname = (char *)argv[wk++];
		}
#ifdef TRACE
		brlog(
		" jobid=%s ofsname=%s ofsdev=%s ofslab=%s ",
			mp->jobid,mp->ofsname, mp->ofsdev,mp->ofslab);
		brlog(
		" dgroup=%s dname=%s dchar=%s dmnames=%s ",
		 ST(mp,dgroup), ST(mp,dname), ST(mp,dchar), ST(mp,dmnames));
#endif
	}
	else {
		if (mp->noptarg < (ISFILE_REST(mp) ? 3 : 6)) {
			brlog(" -R%c only %d optargs ", RCHAR(mp), mp->noptarg);
			sprintf(ME(mp), "Job ID %s: -R%c specified with only %d arguments", mp->jobid, RCHAR(mp), mp->noptarg);
			return(1);
		}
		else {
			wk = optind;
			mp->ofsname = (char *)argv[wk++];
			mp->ofsdev = (char *)argv[wk++];
			mp->dgroup = mystrtok(argv[wk++], ":");
			mp->dname = mystrtok(NULL, ":");
			mp->dchar = mystrtok(NULL, ":");
			mp->dmnames = mystrtok(NULL, ":");
		}
#ifdef TRACE
		brlog(" ofsname=%s ofsdev=%s dname=%s",
			mp->ofsname, mp->ofsdev, mp->dname);
		brlog(
		" dgroup=%s dname=%s dchar=%s dmnames=%s ",
		 ST(mp,dgroup), ST(mp,dname), ST(mp,dchar), ST(mp,dmnames));
#endif
		if (mp->br_type & IS_RFILE) {
			if (mp->noptarg > 3) {
				mp->n_names = mp->noptarg - 3;
				mp->fnames = wk;
#ifdef TRACE
				brlog(" n_names=%d fnames=%d ",
				  mp->n_names,mp->fnames);
#endif
			}
			else {
				brlog(" -RF no files specified ");
				sprintf(ME(mp), "Job ID %s: -RF specified with no files", mp->jobid);
				return(1);
			}
		}
		else {			/* is complete, get new names */
			mp->nfsname = (char *)argv[wk++];
			mp->nfsdev = (char *)argv[wk++];
			mp->jobid = (char *)argv[wk++];
#ifdef TRACE
			brlog(" RC nfsname = %s nfsdev=%s ",
				mp->nfsname, mp->nfsdev);
#endif
		}
	}
	mp->blks_per_vol = get_dcap(mp);	/* perhaps admin specified
						   so don't call devattr */
	mp->dtype = get_dtype(mp->dchar, 1);

	mp->volpromt = devattr(mp->dname, "volume");

	if ((mp->volpromt == NULL) && (mp->flags & tflag)) {
		if (mp->dtype != IS_DIR) {
			brlog("-t specified but %s not removable", mp->dname);
			sprintf(ME(mp), "Job ID %s: -t specified with destination not removable", mp->jobid);
			return(1);
		}
	}
	c = devattr(mp->dname, "cdevice");

	if ((!c) || (!(*c))) {
		c = devattr(mp->dname, "bdevice");
	}
	if ((!c) || (!(*c))) {
		c = devattr(mp->dname, "pathname");
	}
	if (c && (*c)) {	
		mp->dname = c;
	}
	else if (*(mp->dname) != '/') {
		brlog(
		"devmgmt alias %s has no cdevice, no device, and no pathname",
			mp->dname);
		sprintf(ME(mp), "Job ID %s: alias given without cdevice, bdevice and pathname", mp->jobid);
		return(1);
	}
	return(0);
} /* cmdargs() */

static long
get_dcap(mp)
register m_info_t	*mp;
{
	int	i;
	long	cap;

	if (mp->dchar == NULL)
		return((long)(-1));

	i = strfind(mp->dchar, "capacity=");

	if (i < 0)
		return((long)(-1));

	i += 9;				/* skip capacity= */
	cap = atol((char *) ((mp->dchar) + i));

	if (cap > 0)
		return(cap);
	else
		return((long)(-1));
} /* get_dcap() */

static int
get_dtype(strng, search)
register char	*strng;
int		search;
{
	int		i;
	int		k;
	register char	*typ;

	if (strng == NULL)
		return(NDEV);

	if (search) {
		i = strfind(strng, "type=");

		if (i < 0)
			return(NDEV);

		i += 5;				/* skip type= */
		typ = (strng) + i;
	}
	else {
		typ = strng;
	}
	for (k = 0; k < NDEV; k++) {
		if (!strncmp(typ,dtypes[k],strlen(dtypes[k]))) {
			return(k);
		}
	}
	return(NDEV);
} /* get_dtype() */

static
char *
mystrtok(st, dlm)
char *st;
char *dlm;
{
	char *ret, *p;

	if(st)			/* new string */
		cont_ptr = st;

	if(!cont_ptr)		/* NULL pointer */
		return(NULL);

	for( ret = p = cont_ptr; *cont_ptr; ) {

		if( *cont_ptr == '\\' ) {
			cont_ptr++;
			*p++ = *cont_ptr++;

		} else if( *cont_ptr == *dlm ) {
			cont_ptr++;
			*p = '\0';
			break;

		} else *p++ = *cont_ptr++;

	}

	return(ret);			/* was dgroup */

}
