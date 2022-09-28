/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:cmd/get.c	6.16"
# include	"../hdr/defines.h"
# include	"../hdr/had.h"
# include       "sys/utsname.h"
# include       <ccstypes.h>

#if	defined (__STDC__)
#include	<time.h>
#endif	

#define	DATELEN	12

/*
 * Get is now much more careful than before about checking
 * for write errors when creating the g- l- p- and z-files.
 * However, it remains deliberately unconcerned about failures
 * in writing statistical information (e.g., number of lines
 * retrieved).
 */

struct stat Statbuf;
char Null[1];
char Error[128];

int	Debug = 0;
int	had_pfile;
struct packet gpkt;
struct sid sid;
unsigned	Ser;
int	num_files;
char	Whatstr[BUFSIZ];
char	Pfilename[FILESIZE];
char	*ilist, *elist, *lfile;
char	*sid_ab(), *auxf(), *logname();
char	*sid_ba(), *date_ba();
long	cutoff = 0X7FFFFFFFL;	/* max positive long */
int verbosity;
char	Gfile[FILESIZE];
char	*Type;
int	Did_id;
FILE *fdfopen();
struct utsname un;
char *uuname;

/* Beginning of modifications made for CMF system. */
#define CMRLIMIT 128
char	cmr[CMRLIMIT];
int		cmri = 0;
/* End of insertion */

int	fatal(), patoi(), date_ab(), setjmp();
int	uname(), lockit(), getser();
int	stat(), setuid(), setgid(), xcreat();
int	readmod(), unlink(), unlockit(), sidtoser();
int	xopen(), any(), strncmp(), chdir();
int	strcmp(), dup(), cmrinsert(), isatty(), in_pfile();

void    chksid(), setsig(), do_file(), exit(), sinit(), permiss();
void	flushto(), fmterr(), finduser(), doflags(), doie(), setup();
void	ffreeall(), condset(), pf_ab(), fredck(), escdodelt();
void	enter(), dolist(), newsid(), gen_lfile(), idsetup(), prfx();
void	wrtpfile(), mk_qfile(), get();
char	*strcpy(), *strcat();
char	del_ab();

unsigned int	strlen();

main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char c;
	int testmore;
	extern int Fcnt;

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	for(i=1; i<argc; i++)
		if(argv[i][0] == '-' && (c=argv[i][1])) {
			p = &argv[i][2];
			testmore = 0;
			switch (c) {

    			case CMFFLAG:
				/* Concatenate the rest of this argument with the
				 * existing CMR list. */
				while (*p) {
					if (cmri == CMRLIMIT)
						fatal ("CMR list is too long.");
					cmr[cmri++] = *p++;
					}
				cmr[cmri] = NULL;
				break;

			case 'a':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				Ser = patoi(p);
				break;
			case 'r':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				chksid(sid_ab(p,&sid),&sid);
				if ((sid.s_rel < MINR) ||
					(sid.s_rel > MAXR))
					fatal("r out of range (ge22)");
				break;
			case 'w':
				if(*p)
					strcpy(Whatstr,p);
				break;
			case 'c':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				if (date_ab(p,&cutoff))
					fatal("bad date/time (cm5)");
				break;
			case 'l':
				lfile = p;
				break;
			case 'i':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				ilist = p;
				break;
			case 'x':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				elist = p;
				break;
			case 'G':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				copy(p, Gfile);
				break;
			case 'b':
			case 'g':
			case 'e':
			case 'p':
			case 'k':
			case 'm':
			case 'n':
			case 's':
			case 't':
				testmore++;
				break;
			default:
				fatal("unknown key letter (cm1)");
			}

			if (testmore) {
				testmore = 0;
				if (*p) {
					sprintf(Error,
						"value after %c arg (cm8)",c);
					fatal(Error);
				}
			}
			if (had[c - 'a']++)
				fatal("key letter twice (cm2)");
			argv[i] = 0;
		}
		else num_files++;

	if(num_files == 0)
		fatal("missing file arg (cm3)");
	if (HADE && HADM)
		fatal("e not allowed with m (ge3)");
	if (HADE)
		HADK = 1;
	if (!HADS)
		verbosity = -1;
	if (HADE && ! logname())
		fatal("User ID not in password file (cm9)");
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=1; i<argc; i++)
		if (p=argv[i])
			do_file(p,get);
	exit(Fcnt ? 1 : 0);
}


extern char *Sflags[];

void
get(file)
char *file;
{
	register char *p;
	register unsigned ser;
	extern char had_dir, had_standinp;
	struct stats stats;
	struct idel *dodelt();
	char	str[32];
	char *idsubst(), *auxf();
	uid_t holduid;
	gid_t holdgid;
	static first = 1;

	Fflags |= FTLMSG;
	if (setjmp(Fjmp))
		return;
	if (HADE) {
		had_pfile = 1;
		/*
		call `sinit' to check if file is an SCCS file
		but don't open the SCCS file.
		If it is, then create lock file.
		  pass both the PID and machine name to lockit
		*/
		sinit(&gpkt,file,0);
		uname(&un);
		uuname = un.nodename;
		if (lockit(auxf(file,'z'),10, getpid(),uuname))
			fatal("cannot create lock file (cm4)");
	}
	/*
	Open SCCS file and initialize packet
	*/
	sinit(&gpkt,file,1);
	gpkt.p_ixuser = (HADI | HADX);
	gpkt.p_reqsid.s_rel = sid.s_rel;
	gpkt.p_reqsid.s_lev = sid.s_lev;
	gpkt.p_reqsid.s_br = sid.s_br;
	gpkt.p_reqsid.s_seq = sid.s_seq;
	gpkt.p_verbose = verbosity;
	gpkt.p_stdout = (HADP ? stderr : stdout);
	gpkt.p_cutoff = cutoff;
	gpkt.p_lfile = lfile;
	if (Gfile[0] == 0 || !first)
		copy(auxf(gpkt.p_file,'g'),Gfile);
	first = 0;

	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);
	finduser(&gpkt);
	doflags(&gpkt);
	if (!HADA)
		ser = getser(&gpkt);
	else {
		if ((ser = Ser) > maxser(&gpkt))
			fatal("serial number too large (ge19)");
		gpkt.p_gotsid = gpkt.p_idel[ser].i_sid;
		if (HADR && sid.s_rel != gpkt.p_gotsid.s_rel) {
			zero(&gpkt.p_reqsid, sizeof(gpkt.p_reqsid));
			gpkt.p_reqsid.s_rel = sid.s_rel;
		}
		else
			gpkt.p_reqsid = gpkt.p_gotsid;
	}
	doie(&gpkt,ilist,elist,(char *) 0);
	setup(&gpkt,(int) ser);
	if (!(Type = Sflags[TYPEFLAG - 'a']))
		Type = Null;
	if (!(HADP || HADG))
		if (exists(Gfile) && (S_IWRITE & Statbuf.st_mode)) {
			Fflags = FTLEXIT | FTLMSG | FTLCLN;
			sprintf(Error,"writable `%s' exists (ge4)",Gfile);
			fatal(Error);
		}
	if (gpkt.p_verbose) {
		sid_ba(&gpkt.p_gotsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}
	if (HADE) {
		if (HADC || gpkt.p_reqsid.s_rel == 0)
			gpkt.p_reqsid = gpkt.p_gotsid;
		newsid(&gpkt,Sflags[BRCHFLAG - 'a'] && HADB);
		permiss(&gpkt);
		if (exists(auxf(gpkt.p_file,'p')))
			mk_qfile(&gpkt);
		else had_pfile = 0;
		wrtpfile(&gpkt,ilist,elist);
	}
	if (!HADG || HADL) {
		if (gpkt.p_stdout) {
			fflush(gpkt.p_stdout);
			fflush (stderr);
		}
		holduid=geteuid();
		holdgid=getegid();
		setuid(getuid());
		setgid(getgid());
		if (HADL)
			gen_lfile(&gpkt);
		if (HADG)
			return;
		flushto(&gpkt,EUSERTXT,1);
		idsetup(&gpkt);
		gpkt.p_chkeof = 1;
		Did_id = 0;
		/*
		call `xcreate' which deletes the old `g-file' and
		creates a new one except if the `p' keyletter is set in
		which case any old `g-file' is not disturbed.
		The mod of the file depends on whether or not the `k'
		keyletter has been set.
		*/

		if (gpkt.p_gout == 0) {
			if (HADP)
				gpkt.p_gout = stdout;
			else
				gpkt.p_gout = xfcreat(Gfile,HADK ? 
					((mode_t)0644) : ((mode_t)0444) );
		}
		if (Sflags[ENCODEFLAG - 'a'] && (strcmp(Sflags[ENCODEFLAG - 'a'],"1") == 0)) {
			while(readmod(&gpkt)) {
				decode(gpkt.p_line,gpkt.p_gout);
			}
		}
		else {
			while(readmod(&gpkt)) {
				prfx(&gpkt);
				p = idsubst(&gpkt,gpkt.p_line);
				if(fputs(p,gpkt.p_gout)==EOF)
					xmsg(Gfile, "get");
			}
		}
		if (gpkt.p_gout) {
			if (fflush(gpkt.p_gout) == EOF)
				xmsg(Gfile, "get");
			fflush (stderr);
		}
		if (gpkt.p_gout && gpkt.p_gout != stdout) {
			/*
			 * Force g-file to disk and verify
			 * that it actually got there.
			 */
#ifdef NFS_OK
			if (fsync(fileno(gpkt.p_gout)) < 0)
				xmsg(Gfile, "get");
#endif
			if (fclose(gpkt.p_gout) == EOF)
				xmsg(Gfile, "get");
		}
		if (gpkt.p_verbose)
			fprintf(gpkt.p_stdout,"%d lines\n",gpkt.p_glnno);
		if (!Did_id && !HADK)
			if (Sflags[IDFLAG - 'a'])
				if(!(*Sflags[IDFLAG - 'a']))
					fatal("no id keywords (cm6)");
				else
					fatal("invalid id keywords (cm10)");
			else if (gpkt.p_verbose)
				fprintf(stderr,"No id keywords (cm7)\n");
		setuid(holduid);
		setgid(holdgid);
	}
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);

	/*
	remove 'q'-file because it is no longer needed
	*/
	unlink(auxf(gpkt.p_file,'q'));
	ffreeall();
	uname(&un);
	uuname = un.nodename;
	unlockit(auxf(gpkt.p_file,'z'), getpid(),uuname);
}

void
newsid(pkt,branch)
register struct packet *pkt;
int branch;
{
	int chkbr;

	chkbr = 0;
	/* if branch value is 0 set newsid level to 1 */
	if (pkt->p_reqsid.s_br == 0) {
		pkt->p_reqsid.s_lev += 1;
		/*
		if the sid requested has been deltaed or the branch
		flag was set or the requested SID exists in the p-file
		then create a branch delta off of the gotten SID
		*/
		if (sidtoser(&pkt->p_reqsid,pkt) ||
			pkt->p_maxr > pkt->p_reqsid.s_rel || branch ||
			in_pfile(&pkt->p_reqsid,pkt)) {
				pkt->p_reqsid.s_rel = pkt->p_gotsid.s_rel;
				pkt->p_reqsid.s_lev = pkt->p_gotsid.s_lev;
				pkt->p_reqsid.s_br = pkt->p_gotsid.s_br + 1;
				pkt->p_reqsid.s_seq = 1;
				chkbr++;
		}
	}
	/*
	if a three component SID was given as the -r argument value
	and the b flag is not set then up the gotten SID sequence
	number by 1
	*/
	else if (pkt->p_reqsid.s_seq == 0 && !branch)
		pkt->p_reqsid.s_seq = pkt->p_gotsid.s_seq + 1;
	else {
		/*
		if sequence number is non-zero then increment the
		requested SID sequence number by 1
		*/
		pkt->p_reqsid.s_seq += 1;
		if (branch || sidtoser(&pkt->p_reqsid,pkt) ||
			in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			pkt->p_reqsid.s_seq = 1;
			chkbr++;
		}
	}
	/*
	keep checking the requested SID until a good SID to be
	made is calculated or all possibilities have been tried
	*/
	while (chkbr) {
		--chkbr;
		while (in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
		while (sidtoser(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
	}
	if (sidtoser(&pkt->p_reqsid,pkt) || in_pfile(&pkt->p_reqsid,pkt))
		fatal("bad SID calculated in newsid()");
}


void
enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	char str[32];
	register struct apply *ap;

	sid_ba(sidp,str);
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"%s\n",str);
	ap = &pkt->p_apply[n];
	switch(ap->a_code) {

	case SX_EMPTY:
		if (ch == INCLUDE)
			condset(ap,APPLY,INCLUSER);
		else
			condset(ap,NOAPPLY,EXCLUSER);
		break;
	case APPLY:
		sid_ba(sidp,str);
		sprintf(Error,"%s already included (ge9)",str);
		fatal(Error);
		break;
	case NOAPPLY:
		sid_ba(sidp,str);
		sprintf(Error,"%s already excluded (ge10)",str);
		fatal(Error);
		break;
	default:
		fatal("internal error in get/enter() (ge11)");
		break;
	}
}

void
gen_lfile(pkt)
register struct packet *pkt;
{
	char *n;
	int reason;
	char str[32];
	char line[BUFSIZ];
	struct deltab dt;
	FILE *in;
	FILE *out;
	char *outname = "stdout";

#define	OUTPUTC(c) \
	if (putc((c), out) == EOF) \
		xmsg(outname, "gen_lfile");

	in = xfopen(pkt->p_file,0);
	if (*pkt->p_lfile)
		out = stdout;
	else {
		outname = auxf(pkt->p_file, 'l');
		out = xfcreat(outname,(mode_t)0444);
	}
	fgets(line,sizeof(line),in);
	while (fgets(line,sizeof(line),in) != NULL && line[0] == CTLCHAR && line[1] == STATS) {
		fgets(line,sizeof(line),in);
		del_ab(line,&dt,pkt);
		if (dt.d_type == 'D') {
			reason = pkt->p_apply[dt.d_serial].a_reason;
			if (pkt->p_apply[dt.d_serial].a_code == APPLY) {
				OUTPUTC(' ');
				OUTPUTC(' ');
			}
			else {
				OUTPUTC('*');
				if (reason & IGNR) {
					OUTPUTC(' ');
				}
				else {
					OUTPUTC('*');
				}
			}
			switch (reason & (INCL | EXCL | CUTOFF)) {
	
			case INCL:
				OUTPUTC('I');
				break;
			case EXCL:
				OUTPUTC('X');
				break;
			case CUTOFF:
				OUTPUTC('C');
				break;
			default:
				OUTPUTC(' ');
				break;
			}
			OUTPUTC(' ');
			sid_ba(&dt.d_sid,str);
			if (fprintf(out, "%s\t", str) == EOF)
				xmsg(outname, "gen_lfile");
			date_ba(&dt.d_datetime,str);
			if (fprintf(out, "%s %s\n", str, dt.d_pgmr) == EOF)
				xmsg(outname, "gen_lfile");
		}
		while ((n = fgets(line,sizeof(line),in)) != NULL)
			if (line[0] != CTLCHAR)
				break;
			else {
				switch (line[1]) {

				case EDELTAB:
					break;
				default:
					continue;
				case MRNUM:
				case COMMENTS:
					if (dt.d_type == 'D')
						if (fprintf(out,"\t%s",&line[3]) == EOF)
							xmsg(outname, "gen_lfile");
					continue;
				}
				break;
			}
		if (n == NULL || line[0] != CTLCHAR)
			break;
		OUTPUTC('\n');
	}
	fclose(in);
	if (out != stdout) {
#ifdef NFS_OK
		if (fsync(fileno(out)) < 0)
			xmsg(outname, "gen_lfile");
#endif
		if (fclose(out) == EOF)
			xmsg(outname, "gen_lfile");
	}

#undef	OUTPUTC
}


char	Curdate[18];
char	*Curtime;
char	Gdate[DATELEN];
char	Chgdate[18];
char	*Chgtime;
char	Gchgdate[DATELEN];
char	Sid[32];
char	Mod[FILESIZE];
char	Olddir[BUFSIZ];
char	Pname[BUFSIZ];
char	Dir[BUFSIZ];
char	*Qsect;

void
idsetup(pkt)
register struct packet *pkt;
{
	extern long Timenow;
	register int n;
	register char *p;
	void	makgdate();

	date_ba(&Timenow,Curdate);
	Curtime = &Curdate[9];
	Curdate[8] = 0;
	makgdate(Curdate,Gdate);
	for (n = maxser(pkt); n; n--)
		if (pkt->p_apply[n].a_code == APPLY)
			break;
	if (n)
		date_ba(&pkt->p_idel[n].i_datetime,Chgdate);
	Chgtime = &Chgdate[9];
	Chgdate[8] = 0;
	makgdate(Chgdate,Gchgdate);
	sid_ba(&pkt->p_gotsid,Sid);
	if (p = Sflags[MODFLAG - 'a'])
		copy(p,Mod);
	else
		copy(auxf(gpkt.p_file,'g'), Mod);
	if (!(Qsect = Sflags[QSECTFLAG - 'a']))
		Qsect = Null;
}

#if	defined (__STDC__)
void 
makgdate(old,new)
register char *old, *new;
{
	struct tm	time;
	sscanf(old, "%d/%d/%d", &(time.tm_year), &(time.tm_mon), &(time.tm_mday));
	time.tm_mon--;
	strftime (new, (size_t) DATELEN, "%d %b %Y", &time);
}
	

#else	/* __STDC__ not defined */

void
makgdate(old,new)
register char *old, *new;
{
	if ((*new = old[3]) != '0')
		new++;
	*new++ = old[4];
	*new++ = '/';
	if ((*new = old[6]) != '0')
		new++;
	*new++ = old[7];
	*new++ = '/';
	*new++ = old[0];
	*new++ = old[1];
	*new = 0;
}
#endif	/* __STDC__ */


static char Zkeywd[5] = "@(#)";

char *
idsubst(pkt,line)
register struct packet *pkt;
char line[];
{
	static char tline[BUFSIZ];
	char hold[BUFSIZ];
	static char str[32];
	register char *lp, *tp;
	char *trans();
	int recursive = 0;
	extern char *Type;
	extern char *Sflags[];
 	char *getcwd();
		
	if (HADK || !any('%',line))
		return(line);

	tp = tline;
	for(lp=line; *lp != 0; lp++) {
		if((!Did_id) && (Sflags['i'-'a']) &&
			(!(strncmp(Sflags['i'-'a'],lp,strlen(Sflags['i'-'a'])))))
				++Did_id;
		if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%') {
			switch(*++lp) {

			case 'M':
				tp = trans(tp,Mod);
				break;
			case 'Q':
				tp = trans(tp,Qsect);
				break;
			case 'R':
				sprintf(str,"%d",pkt->p_gotsid.s_rel);
				tp = trans(tp,str);
				break;
			case 'L':
				sprintf(str,"%d",pkt->p_gotsid.s_lev);
				tp = trans(tp,str);
				break;
			case 'B':
				sprintf(str,"%d",pkt->p_gotsid.s_br);
				tp = trans(tp,str);
				break;
			case 'S':
				sprintf(str,"%d",pkt->p_gotsid.s_seq);
				tp = trans(tp,str);
				break;
			case 'D':
				tp = trans(tp,Curdate);
				break;
			case 'H':
				tp = trans(tp,Gdate);
				break;
			case 'T':
				tp = trans(tp,Curtime);
				break;
			case 'E':
				tp = trans(tp,Chgdate);
				break;
			case 'G':
				tp = trans(tp,Gchgdate);
				break;
			case 'U':
				tp = trans(tp,Chgtime);
				break;
			case 'Z':
				tp = trans(tp,Zkeywd);
				break;
			case 'Y':
				tp = trans(tp,Type);
				break;
			case 'W':
				if((Whatstr[0] != NULL) && (!recursive)) {
					recursive = 1;
					lp += 2;
					strcpy(hold,Whatstr);
					strcat(hold,lp);
					lp = hold;
					lp--;
					continue;
				}
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Mod);
				*tp++ = '\t';
			case 'I':
				tp = trans(tp,Sid);
				break;
			case 'P':
				copy(pkt->p_file,Dir);
				dname(Dir);
				if(getcwd(Olddir,sizeof(Olddir)) == NULL)
					fatal("getcwd() failed (ge20)");
				if(chdir(Dir) != 0)
					fatal("cannot change directory (ge21)");
			  	if(getcwd(Pname,sizeof(Pname)) == NULL)
					fatal("getcwd() failed (ge20)");
				if(chdir(Olddir) != 0)
					fatal("cannot change directory (ge21)");
				tp = trans(tp,Pname);
				*tp++ = '/';
				tp = trans(tp,(sname(pkt->p_file)));
				break;
			case 'F':
				tp = trans(tp,pkt->p_file);
				break;
			case 'C':
				sprintf(str,"%d",pkt->p_glnno);
				tp = trans(tp,str);
				break;
			case 'A':
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Type);
				*tp++ = ' ';
				tp = trans(tp,Mod);
				*tp++ = ' ';
				tp = trans(tp,Sid);
				tp = trans(tp,Zkeywd);
				break;
			default:
				*tp++ = '%';
				*tp++ = *lp;
				continue;
			}
			if (!(Sflags['i'-'a']))
				++Did_id;
			lp++;
		}
		else
			*tp++ = *lp;
	}

	*tp = 0;
	return(tline);
}


char *
trans(tp,str)
register char *tp, *str;
{
	while(*tp++ = *str++)
		;
	return(tp-1);
}

void
prfx(pkt)
register struct packet *pkt;
{
	char str[32];

	if (HADN)
		if (fprintf(pkt->p_gout, "%s\t", Mod) == EOF)
			xmsg(Gfile, "prfx");
	if (HADM) {
		sid_ba(&pkt->p_inssid,str);
		if (fprintf(pkt->p_gout, "%s\t", str) == EOF)
			xmsg(Gfile, "prfx");
	}
}

void
clean_up()
{
	/*
	clean_up is only called from fatal() upon bad termination.
	*/
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);
	if (gpkt.p_gout)
		fflush(gpkt.p_gout);
	if (gpkt.p_gout && gpkt.p_gout != stdout) {
		fclose(gpkt.p_gout);
		unlink(Gfile);
	}
	if (HADE) {
		if (! had_pfile) {
			unlink(auxf(gpkt.p_file,'p'));
		}
		else if (exists(auxf(gpkt.p_file,'q'))) {
			copy(auxf(gpkt.p_file,'p'),Pfilename);
			rename(auxf(gpkt.p_file,'q'),Pfilename);
		     }
	}
	ffreeall();
	uname(&un);
	uuname = un.nodename;
	unlockit(auxf(gpkt.p_file,'z'), getpid(),uuname);
}


static	char	warn[] = "WARNING: being edited: `%s' (ge18)\n";

void
wrtpfile(pkt,inc,exc)
register struct packet *pkt;
char *inc, *exc;
{
	char line[64], str1[32], str2[32];
	char *user;
	FILE *in, *out;
	struct pfile pf;
	register char *p;
	int fd;
	extern long Timenow;

	user = logname();
	if (exists(p = auxf(pkt->p_file,'p'))) {
		fd = xopen(p,(mode_t)2);
		in = fdfopen(fd,0);
		while (fgets(line,sizeof(line),in) != NULL) {
			p = line;
			p[length(p) - 1] = 0;
			pf_ab(p,&pf,0);
			if (!(Sflags[JOINTFLAG - 'a'])) {
				if ((pf.pf_gsid.s_rel == pkt->p_gotsid.s_rel &&
     				   pf.pf_gsid.s_lev == pkt->p_gotsid.s_lev &&
				   pf.pf_gsid.s_br == pkt->p_gotsid.s_br &&
				   pf.pf_gsid.s_seq == pkt->p_gotsid.s_seq) ||
				   (pf.pf_nsid.s_rel == pkt->p_reqsid.s_rel &&
				   pf.pf_nsid.s_lev == pkt->p_reqsid.s_lev &&
				   pf.pf_nsid.s_br == pkt->p_reqsid.s_br &&
				   pf.pf_nsid.s_seq == pkt->p_reqsid.s_seq)) {
					fclose(in);
					sprintf(Error,
					     "being edited: `%s' (ge17)",line);
					fatal(Error);
				}
				if (!equal(pf.pf_user,user))
					fprintf(stderr,warn,line);
			}
			else fprintf(stderr,warn,line);
		}
		out = fdfopen(dup(fd),1);
		fclose(in);
	}
	else
		out = xfcreat(p,(mode_t)0644);
	if (fseek(out,0L,2) == EOF)
		xmsg(p, "wrtpfile");
	sid_ba(&pkt->p_gotsid,str1);
	sid_ba(&pkt->p_reqsid,str2);
	date_ba(&Timenow,line);
	if (fprintf(out,"%s %s %s %s",str1,str2,user,line) == EOF)
		xmsg(p, "wrtpfile");
	if (inc)
		if (fprintf(out," -i%s",inc) == EOF)
			xmsg(p, "wrtpfile");
	if (exc)
		if (fprintf(out," -x%s",exc) == EOF)
			xmsg(p, "wrtpfile");
	if (cmrinsert () > 0)	/* if there are CMRS and they are okay */
		if (fprintf (out, " -z%s", cmr) == EOF)
			xmsg(p, "wrtpfile");
	if (fprintf(out, "\n") == EOF)
		xmsg(p, "wrtpfile");
	if (fflush(out) == EOF)
		xmsg(p, "wrtpfile");
#ifdef NFS_OK
	if (fsync(fileno(out)) < 0)
		xmsg(p, "wrtpfile");
#endif
	if (fclose(out) == EOF)
		xmsg(p, "wrtpfile");
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"new delta %s\n",str2);
}


getser(pkt)
register struct packet *pkt;
{
	register struct idel *rdp;
	int n, ser, def;
	char *p;
	extern char *Sflags[];

	def = 0;
	if (pkt->p_reqsid.s_rel == 0) {
		if (p = Sflags[DEFTFLAG - 'a'])
			chksid(sid_ab(p, &pkt->p_reqsid), &pkt->p_reqsid);
		else {
			pkt->p_reqsid.s_rel = MAX;
			def = 1;
		}
	}
	ser = 0;
	if (pkt->p_reqsid.s_lev == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if ((rdp->i_sid.s_br == 0 || HADT) &&
				pkt->p_reqsid.s_rel >= rdp->i_sid.s_rel &&
				rdp->i_sid.s_rel > pkt->p_gotsid.s_rel) {
					ser = n;
					pkt->p_gotsid.s_rel = rdp->i_sid.s_rel;
			}
		}
	}
	/*
	 * If had '-t' keyletter and R.L SID type, find
	 * the youngest SID
	*/
	else if ((pkt->p_reqsid.s_br == 0) && HADT) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
			    rdp->i_sid.s_lev == pkt->p_reqsid.s_lev )
				break;
		}
		ser = n;
	}
	else if (pkt->p_reqsid.s_br && pkt->p_reqsid.s_seq == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
				rdp->i_sid.s_lev == pkt->p_reqsid.s_lev &&
				rdp->i_sid.s_br == pkt->p_reqsid.s_br)
					break;
		}
		ser = n;
	}
	else {
		ser = sidtoser(&pkt->p_reqsid,pkt);
	}
	if (ser == 0)
		fatal("nonexistent sid (ge5)");
	rdp = &pkt->p_idel[ser];
	pkt->p_gotsid = rdp->i_sid;
	if (def || (pkt->p_reqsid.s_lev == 0 && pkt->p_reqsid.s_rel == pkt->p_gotsid.s_rel))
		pkt->p_reqsid = pkt->p_gotsid;
	return(ser);
}


/* Null routine to satisfy external reference from dodelt() */

void
escdodelt()
{
}

/* NULL routine to satisfy external reference from dodelt() */
void
fredck()
{
}

in_pfile(sp,pkt)
struct	sid	*sp;
struct	packet	*pkt;
{
	struct	pfile	pf;
	char	line[BUFSIZ];
	char	*p;
	FILE	*in;

	if (Sflags[JOINTFLAG - 'a']) {
		if (exists(auxf(pkt->p_file,'p'))) {
			in = xfopen(auxf(pkt->p_file,'p'),0);
			while ((p = fgets(line,sizeof(line),in)) != NULL) {
				p[length(p) - 1] = 0;
				pf_ab(p,&pf,0);
				if (pf.pf_nsid.s_rel == sp->s_rel &&
					pf.pf_nsid.s_lev == sp->s_lev &&
					pf.pf_nsid.s_br == sp->s_br &&
					pf.pf_nsid.s_seq == sp->s_seq) {
						fclose(in);
						return(1);
				}
			}
			fclose(in);
		}
		else return(0);
	}
	else return(0);
	/*NOTREACHED*/
}

void
mk_qfile(pkt)
register struct	packet *pkt;
{
	FILE	*in, *qout;
	char	line[BUFSIZ];
	char	*qfile = auxf(pkt->p_file, 'q');

	in = xfopen(auxf(pkt->p_file,'p'),0);
	qout = xfcreat(qfile,(mode_t)0644);

	while ((fgets(line,sizeof(line),in) != NULL))
		if (fputs(line, qout) == EOF)
			xmsg(qfile, "mk_qfile");
	(void) fclose(in);
	if (fflush(qout) == EOF)
		xmsg(qfile, "mk_qfile");
#ifdef NFS_OK
	if (fsync(fileno(qout)) < 0)
		xmsg(qfile, "mk_qfile");
#endif
	if (fclose(qout) == EOF)
		xmsg(qfile, "mk_qfile");
}

/* cmrinsert -- insert CMR numbers in the p.file. */

cmrinsert ()
{
	extern char *strrchr (), *Sflags[];
	extern int	cmrcheck ();
	char holdcmr[CMRLIMIT];
	char tcmr[CMRLIMIT];
	char *p;
	int bad;
	int valid;


	if (Sflags[CMFFLAG - 'a'] == 0)		/* CMFFLAG was not set. */
		return (0);

	if ( HADP && ( ! HADZ))	/* no CMFFLAG and no place to prompt. */
		fatal("Background CASSI get with no CMRs\n");

retry:
	if (cmr[0] == NULL) {					/* No CMR list.  Make one. */
		if(HADZ && ((!isatty(0)) || (!isatty(1))))
		{
			fatal("Background CASSI get with invalid CMR\n");
		}
		fprintf (stdout, "Input Comma Separated List of CMRs: ");
		fgets (cmr, CMRLIMIT, stdin);
		p=strend(cmr);
		*(--p) = NULL;
		if ((int)(p - cmr) == CMRLIMIT) {
			fprintf (stdout, "?Too many CMRs.\n");
			cmr[0] = NULL;
			goto retry;					/* Entry was too long. */
			}
		}

	/* Now, check the comma seperated list of CMRs for accuracy. */

	bad = 0;
	valid = 0;
	strcpy(tcmr,cmr);
	while(p=strrchr(tcmr,',')) {
		++p;
		if(cmrcheck(p,Sflags[CMFFLAG - 'a']))
			++bad;
		else {
			++valid;
			strcat(holdcmr,",");
			strcat(holdcmr,p);
		}
		*(--p) = NULL;
	}
	if(*tcmr)
		if(cmrcheck(tcmr,Sflags[CMFFLAG - 'a']))
			++bad;
		else {
			++valid;
			strcat(holdcmr,",");
			strcat(holdcmr,tcmr);
		}

	if(!bad && holdcmr[1]) {
		strcpy(cmr,holdcmr+1);
		return(1);
	}
	else {
		if((isatty(0)) && (isatty(1)))
			if(!valid)
				fprintf(stdout,"Must enter at least one valid CMR.\n");
			else
				fprintf(stdout,"Re-enter invalid CMRs, or press return.\n");
		cmr[0] = NULL;
		goto retry;
	}
}
