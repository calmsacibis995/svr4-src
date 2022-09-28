/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:doname.c	1.19.1.1"
/*	@(#)make:doname.c	1.9 of 5/1/89	*/

#include "defs"
#include <time.h>
#include <errno.h>
#include <sys/stat.h>	/* struct stat */
#include <malloc.h>
#include <ccstypes.h>

#define	MAX(a, b)	( (a) > (b) ? (a) : (b) )
#define MIN(a, b)	( (a) < (b) ? (a) : (b) )
#define PRESTIME()	time( (long) NULL )

extern int	errno,
		lib_cpd;	/* flags library copied in */
extern char	archmem[];	/* archive member name / files.c */
extern CHARSTAR directs[];	/* array of directories for viewpath */
extern LINEBLOCK sufflist;	/* suffix list / main.c */
extern NAMEBLOCK curpname;	/* the name at level 1 of recursion */

extern CHARSTAR	mkqlist(), addstars(), trysccs();	/* misc.c */
extern DEPBLOCK	srchdir();	/* files.c */
extern time_t	exists(),	/* misc.c */
		time();

static CHARSTAR	prompt = "\t";	/* other systems -- pick what you want */
static int 	ndocoms = 0;
char	*touch_t;
size_t touch_t_size;

/*
**	Declare local functions and make LINT happy.
*/

static void	touch();
static int	dobcom();
static int	link_it();
static int	cp();
static void	lnk();
static void	addimpdep();
static void	ballbat();
static void	dbreplace();
static void	expand();
static int	docom1();
static int	docom();

#ifdef MKDEBUG
static void	blprt();
#endif

/*  doname() is a recursive function that traverses the dependency graph
**
**	p->done = 0   don't know what to do yet
**	p->done = 1   file in process of being updated
**	p->done = 2   file already exists in current state
**	p->done = 3   file make failed
*/
int
doname(p, reclevel, tval, d_level)
register NAMEBLOCK p;		/* current target name to be resolved */
int	reclevel,		/* recursion depth level */
	*d_level;		/* directory level in the viewpath */
time_t	*tval;
{
	register DEPBLOCK q,	/* dependency chain ptr, for each line	*
				 *  that name is a target 		*/
			suffp, suffp1;	/* loop variables */
	register LINEBLOCK lp,	/* line chain ptr, for every line	*
				 *  p is a target of (usually only 1) 	*/
			lp1, lp2;	/* loop variables */
	int	access(),
		suffix(),
		is_sccs(),
		errstat,	/* cumulative error counter */
		okdel1,
		didwork;

	void	setvar(),
		dyndep();

	/* the following keep track of minimum viewpath levels of names */

	int	min_level,	/* min. level all dependents, all lines*/
		m1_level,	/* minimum level of depend.s, one line */
		cur_level,	/* level at which target (p) was found */
		dir_level,	/* minimum level of individual dependent */
		rebuild;	/* flag indicating target to be built */

	/* the following keep track of latest (max) of	*
	*	modification times of dependents	*/

	time_t	td,		/* max time for all dependents on a line */
		td1,		/* max time for an individual dependent */
		tdep,		/* max time for all dependents, all lines*/
		ptime,		/* time for target */
		ptime1,		/* temp space */
		exists();
	NAMEBLOCK p1, p2, p3,lookup_name();
	SHBLOCK implcom, 	/* implicit command */
		explcom;	/* explicit command */

	char	sourcename[MAXPATHLEN],	/* area to expand file name & path */
		prefix[MAXPATHLEN], 
		temp[MAXPATHLEN],
		concsuff[ARY20];	/* suffix rule string goes here */
	CHARSTAR pnamep, p1namep;
	CHAIN 	qchain;
	void	appendq();
	int	found, onetime;

	BLDBLOCK bldcom;	/* build command */

	if ( !p ) {	/* if no name, time=0 and done */
		*tval = 0;
		return(0);
	}

#ifdef MKDEBUG
	if (IS_ON(DBUG)) {
		blprt(reclevel);
		printf("doname(%s,%d)\n", p->namep, reclevel);
		(void)fflush(stdout);
	}
#endif
	/* if file is: a) in process of determining status b) exists or
	 *	c) build failed (probably a looping definition)
	 */
	if ( p->done ) {
		*tval = p->modtime;	/* last modified for this name */
		*d_level = p->flevel;	/* set directory to former level*/
		return(p->done == 3);	/* if make failed rtn 1 else 0 */
	}

	min_level = LRGINT;	/* init min level ridiculously high */
	errstat = 0;		/* no errors (yet) */
	tdep = 0;		/* no dependency times yet */
	implcom = NULL;		/* no implicit command */
	explcom = NULL;		/* no explicit command */
	bldcom = 0;		/* no build command (yet) */
	rebuild = 0;		/* nothing found out of date (yet) */

	ptime = exists(p, &cur_level);	/* check file time this level */

#ifdef MKDEBUG
	if (IS_ON(DBUG)) {
		blprt(reclevel);
		printf("TIME(%s)=%ld\n", p->namep, ptime);
	}
#endif
	ptime1 = -1L;
	didwork = NO;
	p->done = 1;	/* avoid infinite loops, set to in-process */
	qchain = NULL;

	/*	Perform runtime dependency translations.	 */

	if ( !(p->rundep) ) {	/* if runtime translation not done */
		okdel1 = okdel;
		okdel = NO;
		setvar("@", p->namep);	/* set name of file to build */
		dyndep(p);		/* substitute dynamic dependencies */
		setvar("@", Nullstr);
		okdel = okdel1;
	}

	/*
	 *	Expand any names that have embedded metacharacters.
	 *	Must be done after dynamic dependencies because the
	 *	dyndep symbols ($(*D)) may contain shell meta
	 *	characters. 
	 */
	expand(p);

	if (p->alias) {
		if (archmem[0] != CNULL) {
			p->namep = copys(archmem);	/*replace with member name */
			archmem[0] = CNULL;
		}
#ifdef MKDEBUG
		if (IS_ON(DBUG)) {
			blprt(reclevel);
			printf("archmem = %s\n", p->namep);
			blprt(reclevel);
			printf("archive alias = %s\n", p->alias);
		}
#endif
	}

	/**		FIRST SECTION		***
	***	   GO THROUGH DEPENDENCIES	**/

#ifdef MKDEBUG
	if (IS_ON(DBUG)) {
		blprt(reclevel);
		printf("look for explicit deps. %d\n", reclevel);
	}
#endif

	/* for each line on which this name is a target	*
	*	(usually only 1 line)			*/
	for (lp = p->linep; lp; lp = lp->nextline) {
		td = 0;
		m1_level = LRGINT;	/* set level real high */

		/* for each dependent on the line */
		for (q = lp->depp; q; q = q->nextdep) {

			/* set predecessor to dependent = target */
			q->depname->backname = p;

			/* do the dependency, to get it's time & level*/

			errstat += doname(q->depname, reclevel + 1, &td1, &dir_level);
			curpname = p;	/* set current name as the target */
#ifdef MKDEBUG
			if (IS_ON(DBUG)) {	/* print out dependency */
				blprt(reclevel);
				printf("TIME(%s)=%ld\n", q->depname->namep, td1);
			}
#endif

			/* set new max time for all dependents on line */
			td = MAX(td1, td);

			/* set new min viewpath direct level for line*/
			m1_level = MIN(dir_level, m1_level);

			/* if out of date, add to out-of-date list */
			if (IS_ON(UCBLD) || ptime < td1) {
				appendq((CHAIN) &qchain, q->depname->namep);
			}
		}

		/* if line is a double-colon one */
		if (p->septype == SOMEDEPS) {

			/* if a shell command exists for line */

			if ( lp->shp )
/*
** note: tests formerly here were deleted to force a rebuild on all 
** double-colon lines.  bl87-34119
*/
				if ((m1_level < cur_level) ||
				    (ptime == -1) ||
				    (ptime < td) ||
				    (!lp->depp) ||
				    (IS_ON(UCBLD) && 
				     (explcom || implcom )))
					rebuild = YES;

/*
				rebuild = YES;
*/

			if ( rebuild ) {
				okdel1 = okdel;			/* save */
				okdel = NO;
				setvar("@", p->namep);
				if (p->alias) {
					setvar("@", p->alias);
					setvar("%", p->namep);
				}
				setvar("?", mkqlist(qchain) );

				/* ? is the list of names out of date
				 * for the target 
			 	 */
				qchain = NULL;
				if ( IS_OFF(QUEST) ) {
					/* link file in */
					lnk(lp->depp);

					/* set the predecessor chain */
					ballbat(p);

					/* do the shell cmd */
					errstat += docom(p, cur_level, lp->shp);
				}
				setvar("@", Nullstr);
				setvar("%", Nullstr);
				okdel = okdel1;

				/* is target there yet ? */
				if ((ptime1 = exists(p, &cur_level)) == -1)
					/* no, set to present */
					ptime1 = PRESTIME();

				didwork = YES;
				rebuild = NO;		/* reset flag*/
			}
		} else {
			/* single colon or implicit */

			if (lp->shp) { /* shell line specified? */
				if (explcom)	/*  specified before? */
					fprintf(stderr, "too many command lines for `%s' (bu10)\n",
					    p->namep);
				else
					explcom = lp->shp;
			}

			/* if arbitrary build criteria specified */
			if (lp->bldecn)
				if (bldcom)	/* specified before? */
					fprintf(stderr, "too many criteria for `%s' (bu11)\n", p->namep);
				else
					bldcom = lp->bldecn;

			/* max time, all lines */
			tdep = MAX(tdep, td);

			/* min level, all lines */
			min_level = MIN(m1_level, min_level);
		}
	}

	/**		SECOND SECTION		***
	***	LOOK FOR IMPLICIT DEPENDENTS	**/

#ifdef MKDEBUG
	if (IS_ON(DBUG)) {
		blprt(reclevel);
		printf("look for implicit rules. %d\n", reclevel);
	}
#endif
	found = 0;	onetime = 0;

	/* for each double suffix rule (default rules in rules.c) */
	for (lp = sufflist; lp; lp = lp->nextline)

		/* for each dependent suffix */
		for (suffp = lp->depp ; suffp ; suffp = suffp->nextdep) {
			/* get suffix string */
			pnamep = suffp->depname->namep;

			/* if it matches suffix of target name */
			if (suffix(p->namep , pnamep , prefix)) {
				CHKARY(doname, prefix, MAXPATHLEN)  /*macro defined in defs*/
#ifdef MKDEBUG
				if (IS_ON(DBUG)) {
					blprt(reclevel);
					printf("right match = %s\n", p->namep);
				}
#endif
				found = 1;

				/* if archive member is target 	*
				 * 	set target suffix	*/
				if (p->alias)
					pnamep = ".a";

searchdir:			
				(void) compath(prefix);
				(void)copstr(temp, prefix);
				CHKARY(doname, temp, MAXPATHLEN)  /*macro defined in defs*/
				(void)addstars(temp);	/* find all files w/ same root*/
				CHKARY(doname, temp, MAXPATHLEN)  /*macro defined in defs*/
				/* (eg.  "*file.*" )  */
				(void)srchdir( temp , NO, (DEPBLOCK) NULL);
				for (lp1 = sufflist; lp1; lp1 = lp1->nextline)
					/* do again for all suffixes & */
					/*  all dependencies of suffixes */
					for (suffp1 = lp1->depp; suffp1;
					     suffp1 = suffp1->nextdep) {

	/* get suffix name */			p1namep = suffp1->depname->namep;
	/* concatenate suffixes */		(void)concat(p1namep, pnamep, concsuff);
						CHKARY(doname, concsuff, ARY20)  /*macro defined in defs*/
						if (!(p1 = SRCHNAME(concsuff)))
							/*check if double suffix rule */
							continue;	/* if not double rule */
						if ( !(p1->linep) )
							continue;	/*no rule line for pair*/
						(void)concat(prefix, p1namep, sourcename);
						CHKARY(doname, sourcename, MAXPATHLEN)  /*macro defined in defs*/
						/*try target with other suffix*/

					        /* sccs file */
						if (ANY(p1namep, WIGGLE)) {
							sourcename[strlen(sourcename) - 1] = CNULL;
							if (!is_sccs(sourcename))

	/* put "s." in front of name */				(void)trysccs(sourcename);
							CHKARY(doname, sourcename, MAXPATHLEN)  /*macro defined in defs*/
						}
						if (!(p2 = SRCHNAME(sourcename)))
							/* find the name with the different
							 * suffix (and/or "s." prefix)
							 */
							continue;	/* if not found */
						if (STREQ(sourcename, p->namep))
							continue;	/* back to same name */

					/*		FOUND		**
					**	left and right match	*/

						found = 2;
#ifdef MKDEBUG
						if (IS_ON(DBUG)) {
							blprt(reclevel);
							/* prints out :
							 *  right name,
							 *  suffix rule,
							 *  and target name
	 					         */
							printf("%s-%s-%s\n",
								sourcename,
								concsuff,
								p->namep);
						}
#endif
						/* this is a dependent of parent */
						p2->backname = p;

						addimpdep(p, p2);
						errstat += doname(p2, reclevel + 1, &td, &dir_level);


						/* do the implied dependent */
						/* if dependent earlier than others */
						/* 	add to the out of date list*/

						if (IS_ON(UCBLD) || ptime < td)
							appendq((CHAIN) &qchain, p2->namep);
						tdep = MAX(tdep, td);	/* max of all lines */
						min_level = MIN(min_level, dir_level);

#ifdef MKDEBUG
						if (IS_ON(DBUG)) {
							blprt(reclevel);
							/* time of implicit dependent */
							printf("TIME(%s)=%ld\n", p2->namep, td);
						}
#endif

						p3=lookup_name(concsuff);
						if(p3 ){
							register DEPBLOCK dp;
							register LINEBLOCK lp;
							for (lp = p3->linep; lp; lp = lp->nextline)
								if ( dp = lp->depp )
									for (; dp; dp = dp->nextdep)
										if ( dp->depname) {
											dp->depname->backname = p;
											addimpdep(p,dp->depname);
											errstat += doname(dp->depname, reclevel + 1, &td, &dir_level);
											/* do the implied dependent */
											/* if dependent earlier than others */
											/* 	add to the out of date list*/
											if (IS_ON(UCBLD) || ptime < td)
												appendq((CHAIN) &qchain, dp->depname->namep);
											tdep = MAX(tdep, td);	/* max of all lines */
											min_level = MIN(min_level, dir_level);
#ifdef MKDEBUG
						if (IS_ON(DBUG)) {
							blprt(reclevel);
							/* time of implicit dependent */
							printf("TIME(%s)=%ld\n", dp->depname->namep, td);
						}
#endif
										}
						}



						curpname = p;	/* set current name as target */
						/* min of all lines */
						setvar("*", prefix);	/* set name root */
						setvar("<", sourcename);/* full name */
						for (lp2 = p1->linep;
						     lp2;
						     lp2 = lp2->nextline)
							/* find the first shell cmd
							 * of all lines for implicit
							 * suffix rule
							 */
							if (implcom = lp2->shp)
								break;
						goto endloop;		/*found the rule, so done here*/
					} /* repeat for next suffix rule
 					   * if doing the single suffix
					   * type rule (see below), you
					   * only loop through this stuff
					   * once, using the root of the
					   * target name as the prefix, and
					   * the 1st suffix null.  
 					   */
				if ( onetime )
					goto endloop;
			}	/* get a new first suffix */
		}

	/*
	 * look for a single suffix type rule.
	 * only possible if no explicit dependents and no shell rules
	 * are found, and nothing has been done so far. (previously, `make'
	 * would exit with 'Don't know how to make ...' message.
	 */
endloop:
	if ( !found &&			/* if not found AND */
		!onetime ){     /*    not the 2nd time thru AND */
		if(!p->linep)     /* no dep action */
			onetime=1;  /* try single suffix rule */
		else
			if(!p->linep->shp){ /* no shell action */
				LINEBLOCK lp;
				DEPBLOCK depp;
				for(lp = p->linep ; lp ; lp = lp->nextline)
				for(depp = lp->depp ; depp ; depp = depp->nextdep)
					if(depp->depname->linep) /* Has dep of dep */
						break;
				if(!depp) /* Not found ; Apply single suffix rule */
	  			onetime=1;
			}

		if(onetime){
#ifdef MKDEBUG
			if (IS_ON(DBUG)) {
				blprt(reclevel);
				printf("Looking for Single suffix rule.\n");
			}
#endif
			(void)concat(p->namep, "", prefix);		/*target name to target prefix*/
			CHKARY(doname, prefix, MAXPATHLEN)  /*macro defined in defs*/
			pnamep = "";
			goto searchdir;
		}
	}


	/**		THIRD SECTION				***
	***	LOOK FOR DEFAULT CONDITION OR DO COMMAND	***
	***     after trying double				***
	***		(and maybe single) suffix rules		**/

	if ((min_level < cur_level) ||	/* dependency on lower level */
	    ((ptime == -1) &&		/* target does not exist and */
	     (tdep == 0)) ||		/*	no dependencies there */
	    (ptime < tdep) ||		/* out of date target */
	    (IS_ON(UCBLD) &&		/* unconditional build  and */
	     (explcom || implcom)))	/*	a command to do */
		rebuild = YES;		

	else if ( bldcom )		/* if arbitrary build criteria chosen */
		rebuild = dobcom(bldcom);	/* check if the arbitrary build
		                                 * criterion calls for rebuilding
		                                 */

	/**	is the rebuilding necessary ?	**/

	if ( rebuild && !errstat ) {

		/* for each line for the target (usually only 1 line)*/
		for (lp = p->linep; lp; lp = lp->nextline)

			lnk(lp->depp);	/* link in dependents for the line */

		/* If the target does not exist in the current directory
		 * and the target is an element of a library, then copy
	 	 * library from the upper node if it exists.
	 	 */
#ifdef MKDEBUG
		if (IS_ON(DBUG))
			printf("library: p->alias, access(p->alias,0), ptime: %s %d %ld\n",
				((p->alias == NULL)? " ": p->alias), access(p->alias, 0), ptime);
#endif
		if (IS_OFF(NOEX) && p->alias &&
	            access(p->alias, 0) && (ptime != -1)) {
#ifdef MKDEBUG
			if (IS_ON(DBUG))
				printf("library-needs copy\n");
#endif
			(void) cat(temp, directs[cur_level], "/", p->alias, 0);
			CHKARY(doname, temp, MAXPATHLEN)  /*macro defined in defs*/
			if (cp(temp, p->alias))
				fprintf(stderr, "warning: cannot copy  %s (bu12)\n", temp);
			else {
#ifdef MKDEBUG
				if (IS_ON(DBUG))
					printf("library copy successful\n");
#endif
				lib_cpd = YES;
			}
		}
		if (p->alias) {
			setvar("@", p->alias);	/* archive file target */
			setvar("%", p->namep);	/* archive member name */
		} else
			setvar("@", p->namep);	/* regular old file target */
		setvar("?", mkqlist(qchain) );	/* string of all out of date things */
		ballbat(p);		/* predecessor chain */

		if (explcom)
			errstat += docom(p, cur_level, explcom);
		else if (implcom)
			errstat += docom(p, cur_level, implcom);
		else if ((p->septype != SOMEDEPS && IS_OFF(MH_DEP)) ||
		         ( !(p->septype)         && IS_ON(MH_DEP)))

	 		/*      OLD WAY OF DOING TEST is
	 		 *              else if(p->septype == 0)
			 *
	 		 *      the flag is "-b".
	 		 */
			/* if there is a .DEFAULT rule, do it */
			if (p1 = SRCHNAME(".DEFAULT")) {
#ifdef MKDEBUG
				if (IS_ON(DBUG)) {
					blprt(reclevel);
					printf("look for DEFAULT rule. %d\n", reclevel);
				}
#endif
				setvar("<", p->namep);
				for (lp2 = p1->linep; lp2; lp2 = lp2->nextline)
					if (implcom = lp2->shp)
						errstat += docom(p, cur_level, implcom);
			} else if ( !(IS_ON(GET) && vpget(p->namep, CD, 0)) )
				fatal1("don't know how to make %s (bu42)", p->namep);

		setvar("@", Nullstr);
		setvar("%", Nullstr);

/*
	By omitting this test and always setting ptime to the present time, we 
	resolve MR# we85-18429.  This doesn't seem to have any bad side 
	effects, but needs serious testing.
		if (IS_ON(NOEX) || ((ptime = exists(p, &cur_level)) == -1))
*/

			ptime = PRESTIME();
		lib_cpd = NO;

	} else if (errstat && !reclevel)
		printf("`%s' not remade because of errors (bu14)\n", p->namep);

	else if ( !(IS_ON(QUEST) || reclevel || didwork) )
		printf("`%s' is up to date.\n", p->namep);

	if (IS_ON(QUEST) && !reclevel)
		mkexit( -(ndocoms > 0) );

	p->done = (errstat ? 3 : 2);
	ptime = MAX(ptime1, ptime);
	p->modtime = ptime;
	*tval = ptime;
	setvar("<", Nullstr);
	setvar("*", Nullstr);
	p->flevel = cur_level;          /* reset the directory levels */
	*d_level = cur_level;
	return(errstat);
}


static int
docom(p, cur_level, q)
int	cur_level;		/* current recursion level */
NAMEBLOCK p;			/* info about this file */
register SHBLOCK q;		/* list of shell commands */
{
	register CHARSTAR s;
	int	sindex();

	++ndocoms;
	if (IS_ON(QUEST))
		return(0);

	if (IS_ON(TOUCH)) {

		s = varptr("@")->varval.charstar;
		if (IS_OFF(SIL))
			printf("touch(%s)\n", s);
		if (IS_OFF(NOEX))
			touch(p, cur_level, s);
	} else {
		char	string[OUTMAX];
		CHARSTAR subst();
		int	Makecall,	/* flags whether to exec $(MAKE) */
			ign, nopr;

		for ( ; q ; q = q->nextsh ) {

		/* Allow recursive makes only if NOEX flag is set */

			if ( !(sindex(q->shbp, "$(BM)")    == -1 &&
			       sindex(q->shbp, "$(BUILD)") == -1 &&
			       sindex(q->shbp, "$(MAKE)")  == -1) &&
 			    IS_ON(NOEX))
				Makecall = YES;
			else
				Makecall = NO;
			(void)subst(q->shbp, string);
			CHKARY(docom, string, OUTMAX)  /*macro defined in defs*/

			ign = IS_ON(IGNERR);
			nopr = NO;
			for (s = string ; *s == MINUS || *s == AT ; ++s)
				if (*s == MINUS)  
					ign = YES;
				else 
					nopr = YES;
			if ( docom1(s, ign, nopr, Makecall) && !ign)
				if (IS_ON(KEEPGO))
					return(1);
				else 
					fatal(0);
		}
	}
	return(0);
}


static int
docom1(comstring, nohalt, noprint, Makecall)
register CHARSTAR comstring;
int	nohalt, noprint, Makecall;
{
	register int	status;
	int	dosys();

	if (comstring[0] == '\0') 
		return(0);

	if (IS_OFF(SIL) && (!noprint || IS_ON(NOEX)) ) {
		register CHARSTAR p1 = comstring;
		CHARSTAR ps = p1;

		for (;;) {
			while (*p1 && *p1 != NEWLINE) 
				p1++;

			if (*p1) {
				*p1 = 0;
				printf("%s%s\n", prompt, ps);
				*p1 = NEWLINE;
				ps = p1 + 1;
				p1 = ps;
			} else {
				printf("%s%s\n", prompt, ps);
				break;
			}
		}
		(void)fflush(stdout);
	}

	if ( status = dosys(comstring, nohalt, Makecall) ) {
		if ( status >> 8 )
			printf("*** Error code %d", status >> 8 );
		else
			printf("*** Termination code %d", status );
		printf(" (bu21)");
		if (nohalt)
			printf(" (ignored)");
		printf("\n");
		(void)fflush(stdout);
	}
	return(status);
}



/* expand()
 *      If there are any Shell meta characters in the name, search the
 *	directory, and if the search finds something replace the
 *	dependency in "p"'s dependency chain.  srchdir() produces a
 *	DEPBLOCK chain whose last member has a null nextdep pointer or
 *	the NULL pointer if it finds nothing.  The loops below do the
 *	following:
 *	for each dep in each line 
 *		if the dep->depname has a shell metacharacter in it and
 *		if srchdir succeeds,
 *			replace the dep with the new one created by
 *				srchdir. 
 *	The Nextdep variable is to skip over the new stuff inserted into
 *	the chain. 
 */
static void
expand(p)
register NAMEBLOCK p;
{
	register DEPBLOCK db, srchdb;
	register LINEBLOCK lp;
	register CHARSTAR s;

	for (lp = p->linep ; lp ; lp = lp->nextline)
		for (db = lp->depp ; db ; db = db->nextdep )
			if (((ANY((s = db->depname->namep), STAR)) ||
			     (ANY(s, QUESTN) || ANY(s, LSQUAR))) &&
			    (srchdb = srchdir(s , YES, (DEPBLOCK) NULL)))
				dbreplace(p, db, srchdb);
}



/*
 *      Replace the odb depblock in np's dependency list with the
 *      dependency chain defined by ndb.  dbreplace() assumes the last
 *	"nextdep" pointer in "ndb" is null.
 */
static void
dbreplace(np, odb, ndb)
NAMEBLOCK np;
register DEPBLOCK odb, ndb;
{
	register LINEBLOCK lp;
	register DEPBLOCK  db, enddb;

	for (enddb = ndb; enddb->nextdep; enddb = enddb->nextdep)
		;
	for (lp = np->linep; lp; lp = lp->nextline)
		if (lp->depp == odb) {
			enddb->nextdep  = lp->depp->nextdep;
			lp->depp        = ndb;
			return;
		} else
			for (db = lp->depp; db; db = db->nextdep)
				if (db->nextdep == odb) {
					enddb->nextdep  = odb->nextdep;
					db->nextdep     = ndb;
					return;
				}
}



#define NPREDS 50

static void
ballbat(np)
NAMEBLOCK np;
{
	static char *ballb;
	static int ballb_size;

	register CHARSTAR p;
	register NAMEBLOCK npp;
	register int	i;

	VARBLOCK vp;
	int	npreds = 0;
	NAMEBLOCK circles[NPREDS];
	int update_vp;
	int temp_size;
	char *temp_string;

	if ( ballb == NULL ) {
		ballb_size = 200;
		if ( ( ballb = malloc(ballb_size) ) == 0 )
			fatal("malloc failed");
	}

	if ( !((vp = varptr("!"))->varval.charstar) ) {
		update_vp = 1;
		vp->varval.charstar = ballb;
	} else
		update_vp = 0;

	temp_string = varptr("<")->varval.charstar;
	temp_size = (temp_string == NULL)?0:strlen(temp_string) + 2;

	for (npp = np; npp; npp = npp->backname) {
		for (i = 0; i < npreds; i++)
			if (npp == circles[i]) {
p_error:			fprintf(stderr, "$! nulled, predecessor cycle\n");
				ballb[0] = CNULL;
				return;
			}
		circles[npreds++] = npp;
		if (npreds >= NPREDS)
			goto p_error;

		temp_size += strlen(npp->namep) + 2;
	}

	if ( temp_size > ballb_size ) {
		ballb_size = temp_size;
		if ( (ballb = realloc(ballb,ballb_size)) == 0 )
			fatal("realloc failed");
		if ( update_vp )
			vp->varval.charstar = ballb;
	}
	p = ballb;
	p = copstr(p, temp_string);
	p = copstr(p, " ");

	for (npp = np; npp; npp = npp->backname) {
		p = copstr(p, npp->namep);
		p = copstr(p, " ");
	}
}


#ifdef MKDEBUG
static void
blprt(n)	/* PRINT n BLANKS WHERE n = CURRENT RECURSION LEVEL. */
register int	n;
{
	while (n--)
		printf("  ");
}
#endif


static void
addimpdep(fname, dep)
register NAMEBLOCK dep;
NAMEBLOCK fname;
{
	register DEPBLOCK dpnom;
	register LINEBLOCK lpnom;

	for (lpnom = fname->linep; lpnom; lpnom = lpnom->nextline)
		for (dpnom = lpnom->depp; dpnom; dpnom = dpnom->nextdep)

			/* if file is already listed as explicit
			 * dependent don't add it to the list. */

			if (STREQ(dpnom->depname->namep, dep->namep)) 
				return;

	dpnom = ALLOC(depblock);
	if (!fname->linep)
		fname->linep = ALLOC(lineblock);
	else
		dpnom->nextdep = fname->linep->depp;
	fname->linep->depp = dpnom;
	dpnom->depname = dep;

}


static void
lnk(fname)
DEPBLOCK fname;
{
	register DEPBLOCK fnomad;
	register NAMEBLOCK depp;
	register CHARSTAR name;
	char	pname[MAXPATHLEN], temp[MAXPATHLEN];
	int	access();

	for (fnomad = fname; fnomad; fnomad = fnomad->nextdep) {
		depp = fnomad->depname;
#ifdef MKDEBUG
		if ( IS_ON(DBUG) && depp != NULL )
			fprintf(stdout, "level = %d %s\n", depp->flevel, depp->namep);
#endif
		if(depp == NULL)exit(255);
		if (depp->flevel > 1) {
			if (!(name = depp->alias))
				name = depp->namep;
			if ((*name == '/') ||
			    (depp->lnkdfl) || (depp->cpydfl))
				continue;

			(void) cat(temp, name, 0);
			CHKARY(lnk, temp, MAXPATHLEN)  /*macro defined in defs*/
			if (access(compath(temp), 04)) {
				(void) cat(pname, directs[depp->flevel], "/",
				    temp, 0);
				CHKARY(lnk, pname, MAXPATHLEN)  /*macro defined in defs*/
				if (is_sccs(name)) {
cp_it:					depp->cpydfl = 1;

					if (cp(pname, temp)) {
						fprintf(stderr, "warning: cannot copy %s (bu15)\n", pname);
						depp->cpydfl = 0;
					}
				} else {
					depp->lnkdfl = 1;
					if (link_it(pname, temp)) {
						depp->lnkdfl = 0;
						if (errno != EEXIST)
							goto cp_it;
						else 
							fatal1("cannot link %s (bu55)\n", pname);
					}
				}
			}
		}
	}
}


static int
cp(oldfile, newfile)
char	*oldfile, *newfile;
{
	register int	numchar;
	register FILE	*fold, *fnew;
	int	stat(), utime(), chmod();
	char	buf[BUFSIZ], nfile[MAXPATHLEN], ofile[MAXPATHLEN];
	struct stat statbuf;

	if (IS_ON(TRACK))
		printf("%scopy %s\n", prompt, oldfile);

	(void)cat(ofile, oldfile, 0);
	CHKARY(cp, ofile, MAXPATHLEN)  /*macro defined in defs*/
	(void)compath(ofile);
	(void)cat(nfile, newfile, 0);
	CHKARY(cp, nfile, MAXPATHLEN)  /*macro defined in defs*/
	(void)compath(nfile);
#ifdef MKDEBUG
	if (IS_ON(DBUG)) {
		printf("cp: oldfile, ofile: %s %s\n", oldfile, ofile);
		printf("cp: newfile, nfile: %s %s\n", newfile, nfile);
	}
#endif
	if ( !(fold = fopen(ofile, "r")) ) {
#ifdef MKDEBUG
		if (IS_ON(DBUG))
			printf("cp: errno: %d\n", errno);
#endif
		fprintf(stderr, "cannot open %s (bu17)\n", oldfile);
		return(-1);
	}
	if ( !(fnew = fopen(nfile, "w")) ) {
		fprintf(stderr, "cannot open %s (bu18)\n", newfile);
		return(-1);
	}
	while (numchar = fread(buf, 1, BUFSIZ, fold))
		if (numchar < 0) {
			fprintf(stderr, "cp: fread error (bu19)\n");
f_error:		(void)fclose(fold);
			(void)fclose(fnew);
			return(-1);
		} else if (fwrite(buf, 1, numchar, fnew) != numchar) {
			fprintf(stderr, "cp: fwrite error (bu20)\n");
			goto f_error;
		}
	(void)stat(ofile, &statbuf);
	(void)chmod(nfile, statbuf.st_mode);
	(void)fclose(fold);
	(void)fclose(fnew);

	return(0);
}


static int
link_it(oldfile, newfile)	/* ln newfile oldfile */
char *oldfile, *newfile;
{
	int link();

	if (link(oldfile, newfile))
		return(-1);		/* couldn't link */

	if (IS_ON(TRACK))
		printf("%slink %s\n", prompt, oldfile);

	return(0);
}


static int
dobcom(q)
register BLDBLOCK q;
{
	register int	status;
	char	string[ARY400];
	CHARSTAR	subst();

	while (q) {
		(void)subst(q->bldecp, string);
		CHKARY(dobcom, string, ARY400)  /*macro defined in defs*/
		status = dosys(string, 0, 0);
		q = q->nxtbldblock;
	}
	return(status);
}


static void
touch(p, cur_level, s)
int	cur_level;
register NAMEBLOCK p;
register CHARSTAR s;
{
	struct utimbuf {
		time_t T_actime;	/* access time */
		time_t T_modtime;	/* modification time */
	};
	extern CHARSTAR	directs[];
	unsigned sleep();
	CHARSTAR mktemp(), dname(), sname();
	int	unlink(), creat(), close(), fd;
	char	s1[MAXPATHLEN], s2[MAXPATHLEN];

	if (p->lnkdfl) {
		(void) cat(touch_t, s, 0);
		(void)dname(touch_t);
		(void) cat(touch_t, "BtXXXXXX", 0);
		CHKARY(touch, touch_t, MAXPATHLEN)	/*macro defined in defs*/
		(void)mktemp(touch_t);
		if (link(s, touch_t)) {
bad:			fprintf(stderr, "cannot touch %s (bu25)", s);
			return;
		}
		(void)unlink(s);
		p->lnkdfl = 0;
		p->cpydfl = 1;
		if (cp(touch_t, s)) {
			p->cpydfl = 0;
			(void)unlink(touch_t);
			goto bad;
		}
		(void)unlink(touch_t);
		return;
	} else if (cur_level > 1) {
		(void) cat(s2, s, 0);
		(void)sname(s2);
		(void) cat(s1, directs[cur_level], "/", s2, 0);
		CHKARY(touch, s1, MAXPATHLEN)	/* macro defined in defs */
		p->cpydfl = 1;
		if (cp(s1, s)) {
			p->cpydfl = 0;
			goto bad;
		}
		return;
	} else {
		if (utime(s, NULL) ) {
			if ((fd = creat(s, (mode_t)0666)) < 0)
				goto bad;
			(void)close(fd);
		}
		(void)sleep(1);
	}
}
