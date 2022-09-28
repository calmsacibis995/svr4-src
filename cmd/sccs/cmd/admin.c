/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:cmd/admin.c	6.14"
# include	"../hdr/defines.h"
# include	"../hdr/had.h"
# include	<sys/dir.h>
# include       <sys/utsname.h>
# include	<string.h>

/*
	Program to create new SCCS files and change parameters
	of existing ones. Arguments to the program may appear in
	any order and consist of keyletters, which begin with '-',
	and named files. Named files which do not exist are created
	and their parameters are initialized according to the given
	keyletter arguments, or are given default values if the
	corresponding keyletters were not supplied. Named files which
	do exist have those parameters corresponding to given key-letter
	arguments changed and other parameters are left as is.

	If a directory is given as an argument, each SCCS file within
	the directory is processed as if it had been specifically named.
	If a name of '-' is given, the standard input is read for a list
	of names of SCCS files to be processed.
	Non-SCCS files are ignored.

	Files created are given mode 444.
*/

# define MAXNAMES 9
# define COPY 0
# define NOCOPY 1

struct stat Statbuf;

char	Null[1];
char	Error[128];
char	*Comments, *Mrs;

int	Did_id;
int 	Encoded = 0;
long	Encodeflag_offset;	/* offset in file where encoded flag is stored */
int	irel;
int	Domrs;

extern FILE *Xiop;
extern char *Sflags[];

static char	*ifile, *tfile;
static char	*CMFAPPL;	/* CMF MODS */
static char	*z;			/* for validation program name */
static char	had_flag[NFLAGS], rm_flag[NFLAGS];
static char	Valpgm[]	=	"val";
static int	fexists, num_files;
static int	VFLAG  =  0;
static struct sid	newsid;
static char	*anames[MAXNAMES], *enames[MAXNAMES];
static char	*unlock;
static char	*locks;
static char	*flag_p[NFLAGS];
static int	asub, esub;
static int	check_id;
static struct utsname	un;
static char	*uuname;
static char b_dir[]  =  "directory `%s' specified as `%c' keyletter value (ad29)";
static	void admin(), cmt_ba(), putmrs();
static	char *adjust(), *getval();
static	int val_list(), pos_ser(), range(), fgetchk();

void	sinit(), xrm();
void	chksid(), setsig(), do_file(), exit(), putline();
void	stats_ab(), newstats(), mrfixup(), ffree(), flushto(), flushline();

char	*repl(), *logname(), *sid_ab(), *gf(), *savecmt(), *getline();
char	*date_ba(), *del_ba(), *fmalloc();
char	*auxf();

int 	unlink(), chkid();
int	stat(), fatal();
int	patoi(), setjmp();
int	wait(), uname(), lockit();
int	xcreat(), cmrcheck(), valmrs(), xopen();
int	chmod(), chown(), unlockit();

long	time();


main(argc,argv)
int argc;
char *argv[];
{
	register int j;
	register char *p;
	char c, f;
	int i, testklt;
	extern int Fcnt;
	struct sid sid;

	/*
	Set flags for 'fatal' to issue message, call clean-up
	routine and terminate processing.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

	testklt = 1;

	/*
	The following loop processes keyletters and arguments.
	Note that these are processed only once for each
	invocation of 'main'.
	*/
	for(j=1; j<argc; j++)
		if(argv[j][0] == '-' && (c = argv[j][1])) {
			p = &argv[j][2];
			switch (c) {

			case 'i':	/* name of file of body */
				ifile = p;
				if (*ifile && exists(ifile))
					if ((Statbuf.st_mode & S_IFMT) ==
						 S_IFDIR) {
					sprintf(Error,b_dir,ifile,c);
					fatal(Error);
				}
				break;

			case 't':	/* name of file of descriptive text */
				tfile = p;
				if (*tfile && exists(tfile))
					if ((Statbuf.st_mode & S_IFMT) ==
						S_IFDIR) {
					sprintf(Error,b_dir,tfile,c);
					fatal(Error);
				}
				break;
			case 'm':	/* mr flag */
				Mrs = p;
				repl(Mrs,'\n',' ');
				break;
			case 'y':	/* comments flag for entry */
				Comments = p;
				break;

			case 'd':	/* flags to be deleted */
				testklt = 0;
				if (!(f = *p))
					fatal("d has no argument (ad1)");
				p = &argv[j][3];

				switch (f) {

				case IDFLAG:	/* see 'f' keyletter */
				case BRCHFLAG:	/* for meanings of flags */
				case VALFLAG:
				case TYPEFLAG:
				case MODFLAG:
				case QSECTFLAG:
				case NULLFLAG:
				case FLORFLAG:
				case CEILFLAG:
				case DEFTFLAG:
				case JOINTFLAG:
					if (*p) {
						sprintf(Error,
						"value after %c flag (ad12)",f);
						fatal(Error);
					}
					break;
				case CMFFLAG:	/* option installed by CMF */
					if (*p) {
						sprintf(Error,
						"value after %c flag (ad12)",f);
						fatal(Error);
					}
					break;
				case LOCKFLAG:
					if (*p) {
						/*
						set pointer to releases
						to be unlocked
						*/
						repl(p,',',' ');
						if (!val_list(p))
							fatal("bad list format (ad27)");
						if (!range(p))
							fatal("element in list out of range (ad28)");
						if (*p != 'a')
							unlock = p;
					}
					break;

				default:
					fatal("unknown flag (ad3)");
				}

				if (rm_flag[f - 'a']++)
					fatal("flag twice (ad4)");
				break;

			case 'f':	/* flags to be added */
				testklt = 0;
				if (!(f = *p))
					fatal("f has no argument (ad5)");
				p = &argv[j][3];

				switch (f) {

				case BRCHFLAG:	/* branch */
				case NULLFLAG:	/* null deltas */
				case JOINTFLAG:	/* joint edit flag */
					if (*p) {
						sprintf(Error,
						"value after %c flag (ad13)",f);
						fatal(Error);
					}
					break;

				case IDFLAG:	/* id-kwd message (err/warn) */
					break;

				case VALFLAG:	/* mr validation */
					VFLAG++;
					if (*p)
						z = p;
					break;

				case CMFFLAG:			/* CMFNET MODS */
					if (*p)				/* CMFNET MODS */
						CMFAPPL = p;	/* CMFNET MODS */
					else				/* CMFNET MODS */
						fatal ("No application with application flag.");
					if (gf (CMFAPPL) == (char*) NULL)
						fatal ("No .FRED file exists for this application.");
					break;				/* END CMFNET MODS */

				case FLORFLAG:	/* floor */
					if ((i = patoi(p)) == -1)
						fatal("floor not numeric (ad22)");
					if (((int) size(p) > 5) || (i < MINR) ||
							(i > MAXR))
						fatal("floor out of range (ad23)");
					break;

				case CEILFLAG:	/* ceiling */
					if ((i = patoi(p)) == -1)
						fatal("ceiling not numeric (ad24)");
					if (((int) size(p) > 5) || (i < MINR) ||
							(i > MAXR))
						fatal("ceiling out of range (ad25)");
					break;

				case DEFTFLAG:	/* default sid */
					if (!(*p))
						fatal("no default sid (ad14)");
					chksid(sid_ab(p,&sid),&sid);
					break;

				case TYPEFLAG:	/* type */
				case MODFLAG:	/* module name */
				case QSECTFLAG:	/* csect name */
					if (!(*p)) {
						sprintf(Error,
						"flag %c has no value (ad2)",f);
						fatal(Error);
					}
					break;
				case LOCKFLAG:	/* release lock */
					if (!(*p))
						/*
						lock all releases
						*/
						p = "a";
					/*
					replace all commas with
					blanks in SCCS file
					*/
					repl(p,',',' ');
					if (!val_list(p))
						fatal("bad list format (ad27)");
					if (!range(p))
						fatal("element in list out of range (ad28)");
					break;

				default:
					fatal("unknown flag (ad3)");
				}

				if (had_flag[f - 'a']++)
					fatal("flag twice (ad4)");
				flag_p[f - 'a'] = p;
				break;

			case 'r':	/* initial release number supplied */
				 chksid(sid_ab(p,&newsid),&newsid);
				 if ((newsid.s_rel < MINR) ||
				     (newsid.s_rel > MAXR))
					fatal("r out of range (ad7)");
				 break;

			case 'n':	/* creating new SCCS file */
			case 'h':	/* only check hash of file */
			case 'z':	/* zero the input hash */
			case 'b':	/* force file to be encoded (binary) */
				break;

			case 'a':	/* user-name allowed to make deltas */
				testklt = 0;
				if (!(*p))
					fatal("bad a argument (ad8)");
				if (asub > MAXNAMES)
					fatal("too many 'a' keyletters (ad9)");
				anames[asub++] = p;
				break;

			case 'e':	/* user-name to be removed */
				testklt = 0;
				if (!(*p))
					fatal("bad e argument (ad10)");
				if (esub > MAXNAMES)
					fatal("too many 'e' keyletters (ad11)");
				enames[esub++] = p;
				break;

			default:
				fatal("unknown key letter (cm1)");
			}

			if (had[c - 'a']++ && testklt++)
				fatal("key letter twice (cm2)");
			argv[j] = 0;
		}
		else
			num_files++;

	if (num_files == 0)
		fatal("missing file arg (cm3)");

	if ((HADY || HADM) && ! (HADI || HADN))
		fatal("illegal use of 'y' or 'm' keyletter (ad30)");
	if (HADI && num_files > 1) /* only one file allowed with `i' */
		fatal("more than one file (ad15)");
	if ((HADI || HADN) && ! logname())
		fatal("USER ID not in password file (cm9)");

	setsig();

	/*
	Change flags for 'fatal' so that it will return to this
	routine (main) instead of terminating processing.
	*/
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	/*
	Call 'admin' routine for each file argument.
	*/
	for (j=1; j<argc; j++)
		if (p = argv[j])
			do_file(p,admin);

	exit(Fcnt ? 1 : 0);
}


/*
	Routine that actually does admin's work on SCCS files.
	Existing s-files are copied, with changes being made, to a
	temporary file (x-file). The name of the x-file is the same as the
	name of the s-file, with the 's.' replaced by 'x.'.
	s-files which are to be created are processed in a similar
	manner, except that a dummy s-file is first created with
	mode 444.
	At end of processing, the x-file is renamed with the name of s-file
	and the old s-file is removed.
*/

static struct packet gpkt;	/* see file defines.h */
static char	Zhold[BUFSIZ];	/* temporary z-file name */

static void
admin(afile)
char	*afile;
{
	struct deltab dt;	/* see file defines.h */
	struct stats stats;	/* see file defines.h */
	struct stat sbuf;
	FILE	*iptr, *fdfopen();
	register int k;
	register char *cp, *q;
	char	*in_f;		/* ptr holder for lockflag vals in SCCS file */
	char	nline[BUFSIZ];
	char	*p_lval, *tval, *lval;
	char	f;		/* character holder for flag character */
	char	line[1024];
	int	i;		/* used in forking procedure */
	int	ck_it  =  0;	/* used for lockflag checking */
	int     offset;
	int     thash;
	char    nl;             /* used for checking for newline at EOF */
	int	status;		/* used for status return from fork */
	char	lastch;		/* last char of the file - should be newline */
	extern	nfiles;
	extern	char had_dir;

	if (setjmp(Fjmp))	/* set up to return here from 'fatal' */
		return;		/* and return to caller of admin */

	zero((char *) &stats,sizeof(stats));

	if (HADI && had_dir) /* directory not allowed with `i' keyletter */
		fatal("directory named with `i' keyletter (ad26)");

	fexists = exists(afile);

	if (HADI)
		HADN = 1;
	if (HADI || HADN) {
			if (VFLAG && had_flag[CMFFLAG - 'a'])
			fatal ("Can't have two verification routines.");

		if (HADM && !VFLAG && !had_flag[CMFFLAG - 'a'])
			fatal("MRs not allowed (de8)");

		if (VFLAG && !HADM)
			fatal("MRs required (de10)");

	}

	if (!HADI && HADR)
		fatal("r only allowed with i (ad16)");

	if (HADN && HADT && !(*tfile))
		fatal("t has no argument (ad17)");

	if (HADN && HADD)
		fatal("d not allowed with n (ad18)");

	if (HADN && fexists) {
		sprintf(Error,"file %s exists (ad19)",afile);
		fatal(Error);
	}

	if (!HADN && !fexists) {
		sprintf(Error,"file %s does not exist (ad20)",afile);
		fatal(Error);
	}
	if (HADH) {
		/*
		   fork here so 'admin' can execute 'val' to
		   check for a corrupted file.
		*/
		if ((i = fork()) < 0)
			fatal("cannot fork, try again");
		if (i == 0) {		/* child */
			/*
			   perform 'val' with appropriate keyletters
			*/
			execlp(Valpgm, Valpgm, "-s", afile, 0);
			sprintf(Error,"cannot execute '%s'",Valpgm);
			fatal(Error);
		}
		else {
			wait(&status);	   /* wait on status from 'execlp' */
			if (status)
				fatal("corrupted file (co6)");
			return;		/* return to caller of 'admin' */
		}
	}

	/*
	Lock out any other user who may be trying to process
	the same file.
	*/
	uname(&un);
	uuname = un.nodename;
	if (!HADH && lockit(copy(auxf(afile,'z'),Zhold),(unsigned)2,getpid(),uuname))
		fatal("cannot create lock file (cm4)");

	if (fexists) { /* modifying */

		sinit(&gpkt,afile,1);	/* init pkt & open s-file */
	
		/* Modify checksum if corrupted */

	        if ((int) strlen(gpkt.p_line) > 8 && gpkt.p_line[0] == '\001'
		         && gpkt.p_line[1] == '\150' ) 
		{
			gpkt.p_line[7] = '\012';
		   	gpkt.p_line[8] = '\000';
	
		}

	}


	

	else {
		xfcreat(afile,0444);	/* create dummy s-file */
		sinit(&gpkt,afile,0);	/* and init pkt */
	}

	if (!HADH)
		/*
		   set the flag for 'putline' routine to open
		   the 'x-file' and allow writing on it.
		*/
		gpkt.p_upd = 1;

	if (HADZ) {
		gpkt.do_chksum = 0;	/* ignore checksum processing */
		gpkt.p_ihash = 0;
	}

	/*
	Get statistics of latest delta in old file.
	*/
	if (!HADN) {
		stats_ab(&gpkt,&stats);
		gpkt.p_wrttn++;
		newstats(&gpkt,line,"0");
	}

	if (HADN) {		/*   N E W   F I L E   */

		/*
		Beginning of SCCS file.
		*/
		sprintf(line,"%c%c%s\n",CTLCHAR,HEAD,"00000");
		putline(&gpkt,line);

		/*
		Statistics.
		*/
		newstats(&gpkt,line,"0");

		dt.d_type = 'D';	/* type of delta */

		/*
		Set initial release, level, branch and
		sequence values.
		*/
		if (HADR)
			{
			 dt.d_sid.s_rel = newsid.s_rel;
			 dt.d_sid.s_lev = newsid.s_lev;
			 dt.d_sid.s_br  = newsid.s_br ;
			 dt.d_sid.s_seq = newsid.s_seq;
			 if (dt.d_sid.s_lev == 0) dt.d_sid.s_lev = 1;
			 if ((dt.d_sid.s_br) && ( ! dt.d_sid.s_seq))
				dt.d_sid.s_seq = 1;
			}
		else
			{
			 dt.d_sid.s_rel = 1;
			 dt.d_sid.s_lev = 1;
			 dt.d_sid.s_br = dt.d_sid.s_seq = 0;
			}
		time(&dt.d_datetime);		/* get time and date */

		copy(logname(),dt.d_pgmr);	/* get user's name */

		dt.d_serial = 1;
		dt.d_pred = 0;

		del_ba(&dt,line);	/* form and write */
		putline(&gpkt,line);	/* delta-table entry */

		/*
		If -m flag, enter MR numbers
		*/

		if (Mrs) {
			if (had_flag[CMFFLAG - 'a']) {	/* CMF check routine */
				if (cmrcheck (Mrs, CMFAPPL) != 0) {	/* check them */
					fatal ("Bad CMR number(s).");
					}
				}
			mrfixup();
			if (z && valmrs(&gpkt,z))
				fatal("invalid MRs (de9)");
			putmrs(&gpkt);
		}

		/*
		Enter comment line for `chghist'
		*/

		if (HADY) {
			sprintf(line,"%c%c ",CTLCHAR,COMMENTS);
			putline(&gpkt,line);
			sprintf(line,"%s",savecmt(Comments));
			putline(&gpkt,line);
			putline(&gpkt,"\n");
		}
		else {
			/*
			insert date/time and pgmr into comment line
			*/
			cmt_ba(&dt,line);
			putline(&gpkt,line);
		}
		/*
		End of delta-table.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EDELTAB);
		putline(&gpkt,line);

		/*
		Beginning of user-name section.
		*/
		sprintf(line,CTLSTR,CTLCHAR,BUSERNAM);
		putline(&gpkt,line);
	}
	else
		/*
		For old file, copy to x-file until user-name section
		is found.
		*/
		flushto(&gpkt,BUSERNAM,COPY);

	/*
	Write user-names to be added to list of those
	allowed to make deltas.
	*/
	if (HADA)
		for (k = 0; k < asub; k++) {
			sprintf(line,"%s\n",anames[k]);
			putline(&gpkt,line);
		}

	/*
	Do not copy those user-names which are to be erased.
	*/
	if (HADE && !HADN)
		while ((cp = getline(&gpkt)) &&
				!(*cp++ == CTLCHAR && *cp == EUSERNAM)) {
			for (k = 0; k < esub; k++) {
				cp = gpkt.p_line;
				while (*cp)	/* find and */
					cp++;	/* zero newline */
				*--cp = '\0';	/* character */

				if (equal(enames[k],gpkt.p_line)) {
					/*
					Tell getline not to output
					previously read line.
					*/
					gpkt.p_wrttn = 1;
					break;
				}
				else
					*cp = '\n';	/* restore newline */
			}
		}

	if (HADN) {		/*   N E W  F I L E   */

		/*
		End of user-name section.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EUSERNAM);
		putline(&gpkt,line);
	}
	else
		/*
		For old file, copy to x-file until end of
		user-names section is found.
		*/
		if (!HADE)
			flushto(&gpkt,EUSERNAM,COPY);

	/*
	For old file, read flags and their values (if any), and
	store them. Check to see if the flag read is one that
	should be deleted.
	*/
	if (!HADN)
		while ((cp = getline(&gpkt)) &&
				(*cp++ == CTLCHAR && *cp++ == FLAG)) {

			gpkt.p_wrttn = 1;	/* don't write previous line */

			NONBLANK(cp);	/* point to flag character */
			k = *cp - 'a';
			f = *cp++;
			NONBLANK(cp);
			if (f == LOCKFLAG) {
				p_lval = cp;
				tval = fmalloc(size(gpkt.p_line)- (unsigned)5);
				copy(++p_lval,tval);
				lval = tval;
				while(*tval)
					++tval;
				*--tval = '\0';
			}

			if (!had_flag[k] && !rm_flag[k]) {
				had_flag[k] = 2;	/* indicate flag is */
							/* from file, not */
							/* from arg list */

				if (*cp != '\n') {	/* get flag value */
					q = fmalloc(size(gpkt.p_line) - (unsigned)5);
					copy(cp,q);
					flag_p[k] = q;
					while (*q)	/* find and */
						q++;	/* zero newline */
					*--q = '\0';	/* character */
				}
			}
			if (rm_flag[k])
				if (f == LOCKFLAG) {
					if (unlock) {
						in_f = lval;
						if ((lval = adjust(in_f)) &&
							!had_flag[k])
							ck_it = had_flag[k] = 1;
					}
					else had_flag[k] = 0;
				}
				else had_flag[k] = 0;
		}


	/*
	Write out flags.
	*/
	/* test to see if the CMFFLAG is safe */
	if (had_flag[CMFFLAG - 'a']) {
		if (had_flag[VALFLAG - 'a'] && !rm_flag[VALFLAG - 'a'])
			fatal ("Can't use -fz with -fv.");
		}
	for (k = 0; k < NFLAGS; k++)
		if (had_flag[k]) {
			if (flag_p[k] || lval) {
				if (('a' + k) == LOCKFLAG && had_flag[k] == 1) {
					if (*flag_p[k] == 'a' || *lval == 'a')
						locks = "a";
					else if (lval && flag_p[k])
						locks =
						cat(nline,lval," ",flag_p[k],0);
					else if (lval)
						locks = lval;
					else locks = flag_p[k];
					sprintf(line,"%c%c %c %s\n",
						CTLCHAR,FLAG,'a' + k,locks);
					locks = 0;
					if (lval) {
						ffree(lval);
						tval = lval = 0;
					}
					if (ck_it)
						had_flag[k] = ck_it = 0;
				}
				else sprintf(line,"%c%c %c %s\n",
					CTLCHAR,FLAG,'a'+k,flag_p[k]);
			}
			else
				sprintf(line,"%c%c %c\n",
					CTLCHAR,FLAG,'a'+k);

			/* flush imbeded newlines from flag value */
			i = 4;
			if (line[i] == ' ')
				for (i++; line[i+1]; i++)
					if (line[i] == '\n')
						line[i] = ' ';
			putline(&gpkt,line);

			if (had_flag[k] == 2) {	/* flag was taken from file */
				had_flag[k] = 0;
				if (flag_p[k]) {
					ffree(flag_p[k]);
					flag_p[k] = 0;
				}
			}
		}

	if (HADN) {
		if (HADI) {
			/*
			If the "encoded" flag was not present, put it in
			with a value of 0; this acts as a place holder,
			so that if we later discover that the file contains
			non-ASCII characters we can flag it as encoded
			by setting the value to 1.
		 	*/
			Encodeflag_offset = ftell(Xiop);
			sprintf(line,"%c%c %c 0\n",
				CTLCHAR,FLAG,ENCODEFLAG);
			putline(&gpkt,line);
		}
		/*
		Beginning of descriptive (user) text.
		*/
		sprintf(line,CTLSTR,CTLCHAR,BUSERTXT);
		putline(&gpkt,line);
	}
	else
		/*
		Write out BUSERTXT record which was read in
		above loop that processes flags.
		*/
		gpkt.p_wrttn = 0;
		putline(&gpkt,(char *) 0);

	/*
	Get user description, copy to x-file.
	*/
	if (HADT) {
		if (*tfile) {
			iptr = xfopen(tfile,0);
			fgetchk(line,1024,iptr,tfile,&gpkt,0,NULL);
			fclose(iptr);
		}

		/*
		If old file, ignore any previously supplied
		commentary. (i.e., don't copy it to x-file.)
		*/
		if (!HADN)
			flushto(&gpkt,EUSERTXT,NOCOPY);
	}

	if (HADN) {		/*   N E W  F I L E   */

		/*
		End of user description.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EUSERTXT);
		putline(&gpkt,line);

		/*
		Beginning of body (text) of first delta.
		*/
		sprintf(line,"%c%c %d\n",CTLCHAR,INS,1);
		putline(&gpkt,line);

		if (HADI) {		/* get body */

			/*
			Set indicator to check lines of body of file for
			keyword definitions.
			If no keywords are found, a warning
			will be produced.
			*/
			check_id = 1;
			/*
			Set indicator that tells whether there
			were any keywords to 'no'.
			*/
			Did_id = 0;
			if (*ifile)
				iptr = xfopen(ifile,0);	/* from a file */
			else
				iptr = stdin;	/* from standard input */

			/* save an offset to x-file in case need to encode
                           file.  Then won't have to start all over.  Also
                           save the hash value up to this point.
			 */
			offset = ftell(Xiop);
			thash = gpkt.p_nhash;

			/*
			If we haven't already been told that the file
			should be encoded, read and copy to x-file,
			while checking for control characters (octal 1),
			and also check if file ends in newline.  If
			control char or no newline, the file needs to
			be encoded.
			Also, count lines read, and set statistics'
			structure appropriately.
			The 'fgetchk' routine will check for keywords.
			*/
			if (!HADB) {
				stats.s_ins = fgetchk(line,512,iptr,ifile,
				    &gpkt, 1, &lastch);
				if (stats.s_ins == -1 || lastch != '\n')
					Encoded = 1;
			} else
				Encoded = 1;
			if (Encoded) {
				/* non-ascii characters in file, encode them */
				code(iptr,afile,offset,thash,&gpkt);
				stats.s_ins = fgetchk(line,512,iptr,ifile,
				   &gpkt,0,NULL);
			}
			stats.s_del = stats.s_unc = 0;

			/*
			If no keywords were found, issue warning.
			*/
			if (!Did_id) {
				if (had_flag[IDFLAG - 'a'])
					if(!(flag_p[IDFLAG -'a']))
						fatal("no id keywords (cm6)");
					else
						fatal("invalid id keywords (cm10)");
				else
					fprintf(stderr,"%s\n","No id keywords (cm7)");
			}

			check_id = 0;
			Did_id = 0;
		}

		/*
		End of body of first delta.
		*/
		sprintf(line,"%c%c %d\n",CTLCHAR,END,1);
		putline(&gpkt,line);
	}
	else {
		/*
		Indicate that EOF at this point is ok, and
		flush rest of (old) s-file to x-file.
		*/
		gpkt.p_chkeof = 1;
		while (getline(&gpkt)) ;
	}

	/* If encoded file, put change "fe" flag and recalculate
	   the hash value
	 */
	
	if (Encoded)
	{
		strcpy(line,"0");
		q = line;
		while (*q)
			gpkt.p_nhash -= *q++;
		strcpy(line,"1");
		q = line;
		while (*q)
			gpkt.p_nhash += *q++;
		fseek(Xiop,Encodeflag_offset,0);
		fprintf(Xiop,"%c%c %c 1\n",
			CTLCHAR, FLAG, ENCODEFLAG);
	}

	/*
	Flush the buffer, take care of rewinding to insert
	checksum and statistics in file, and close.
	*/
	flushline(&gpkt,&stats);

	/*
	Change x-file name to s-file, and delete old file.
	Unlock file before returning.
	*/
	if (!HADH) {
		if (!HADN) stat(gpkt.p_file,&sbuf);
		rename(auxf(gpkt.p_file,'x'),(char *)&gpkt);
		if (!HADN) {
			chmod(gpkt.p_file, sbuf.st_mode);
			chown(gpkt.p_file,sbuf.st_uid, sbuf.st_gid);
		}
		xrm();
		uname(&un);
		uuname = un.nodename;
		unlockit(auxf(afile,'z'),getpid(),uuname);
	}

	if (HADI)
		unlink(auxf(gpkt.p_file,'e'));
}

static int
fgetchk(strp,len,inptr,file,pkt,fflag,lastchp)
register char *strp;
register int len;
FILE *inptr;
register char *file;
register struct packet *pkt;
int fflag;
char *lastchp;
{
	register int k;
	int error_line;
	char lastch;

	/*
	 * This gives the illusion that a zero-length file ends
	 * in a newline so that it won't be mistaken for a 
	 * binary file.
	 */
	if (lastchp)
		*lastchp = '\n';

	error_line = 0;
	for (k = 1; fgets(strp,len,inptr); k++)
	{
		lastch = strp[strlen(strp)-1];
		if (lastchp)
			*lastchp = lastch;
		if (lastch != '\n') 
		{
			error_line = k;
			if (!fflag) 
			{
				break;
			}
		}
		if (*strp == CTLCHAR)
		{
			if (fflag)
				return(-1);
			else
			{
				sprintf(Error,"%s illegal data on line %d (ad21)",
					file,k);
				fatal(Error);
			}
		}

		if (check_id)
			(void) chkid(strp,flag_p['i'-'a']);

		putline(pkt,strp);
	}
	if (error_line != 0) 
	{
		sprintf(Error,"%s line %d too long (ad32)",
			file, error_line);
		fatal(Error);
	}
	return(k - 1);
}

void
clean_up()
{
	xrm();
	if (Xiop)
		fclose(Xiop);
	if(gpkt.p_file[0])
		unlink(auxf(gpkt.p_file,'x'));
	if (HADI)
		unlink(auxf(gpkt.p_file,'e'));
	if (HADN)
		unlink(gpkt.p_file);
	if (!HADH) {
		uname(&un);
		uuname = un.nodename;
		unlockit(Zhold, getpid(),uuname);
	}
}

static void
cmt_ba(dt,str)
register struct deltab *dt;
char *str;
{
	register char *p;

	p = str;
	*p++ = CTLCHAR;
	*p++ = COMMENTS;
	*p++ = ' ';
	copy("date and time created",p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	date_ba(&dt->d_datetime,p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	copy("by",p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	copy(dt->d_pgmr,p);
	while (*p++)
		;
	--p;
	*p++ = '\n';
	*p = 0;
}

static void
putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char str[64];
	extern char **Varg;

	for (argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


static char*
adjust(line)
char	*line;
{
	register int k;
	register int i;
	char	*t_unlock;
	char	t_line[BUFSIZ];
	char	rel[5];

	t_unlock = unlock;
	while(*t_unlock) {
		NONBLANK(t_unlock);
		t_unlock = getval(t_unlock,rel);
		while ((k = pos_ser(line,rel)) != -1) {
			for(i = k; i < ((int) size(rel) + k); i++) {
				line[i] = '+';
				if (line[i++] == ' ')
					line[i] = '+';
				else if (line[i] == '\0')
					break;
				else --i;
			}
			k = 0;
			for(i = 0; i < (int) length(line); i++)
				if (line[i] == '+')
					continue;
				else if (k == 0 && line[i] == ' ')
					continue;
				else t_line[k++] = line[i];
			if (t_line[(int) strlen(t_line) - 1] == ' ')
				t_line[(int) strlen(t_line) - 1] = '\0';
			t_line[k] = '\0';
			line = t_line;
		}
	}
	if (length(line))
		return(line);
	else return(0);
}

static char*
getval(sourcep,destp)
register char	*sourcep;
register char	*destp;
{
	while (*sourcep != ' ' && *sourcep != '\t' && *sourcep != '\0')
		*destp++ = *sourcep++;
	*destp = 0;
	return(sourcep);
}

static int
val_list(list)
register char *list;
{
	register int i;

	if (list[0] == 'a')
		return(1);
	else for(i = 0; list[i] != '\0'; i++)
		if (list[i] == ' ' || numeric(list[i]))
			continue;
		else if (list[i] == 'a') {
			list[0] = 'a';
			list[1] = '\0';
			return(1);
		}
		else return(0);
	return(1);
}

static int
pos_ser(s1,s2)
char	*s1;
char	*s2;
{
	register int offset;
	register char *p;
	char	num[5];

	p = s1;
	offset = 0;

	while(*p) {
		NONBLANK(p);
		p = getval(p,num);
		if (equal(num,s2)) {
			return(offset);
		}
		offset = offset + (int) size(num);
	}
	return(-1);
}

static int
range(line)
register char *line;
{
	register char *p;
	char	rel[BUFSIZ];

	p = line;
	while(*p) {
		NONBLANK(p);
		p = getval(p,rel);
		if ((int) size(rel) > 5)
			return(0);
	}
	return(1);
}

code(iptr,afile,offset,thash,gpkt)
FILE *iptr;
char *afile;
int offset;
int thash;
struct packet *gpkt;
{
	FILE *eptr;


	/* issue a warning that file is non-text */
	if (!HADB)
		fprintf(stderr,"%s\n","Not a text file (ad31)");
	rewind(iptr);
	eptr = fopen(auxf(afile,'e'),"w");

	encode(iptr,eptr);
	fclose(eptr);
	fclose(iptr);
	iptr = fopen(auxf(afile,'e'),"r");
	/* close the stream to xfile and reopen it at offset.  Offset is
	 * the end of sccs header info and before gfile contents
	 */
	putline(&gpkt,0);
	fseek(Xiop,offset,0);
	gpkt->p_nhash = thash;
}
	

	
