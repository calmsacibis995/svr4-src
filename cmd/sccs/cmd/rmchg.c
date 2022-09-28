/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:cmd/rmchg.c	6.12"
# include "../hdr/defines.h"
# include "../hdr/had.h"
# include "../hdr/filehand.h"
# include "sys/utsname.h"

/*
	Program to remove a specified delta from an SCCS file,
	when invoked as 'rmdel',
	or to change the MRs and/or comments of a specified delta,
	when invoked as 'cdc'.
	(The program has two links to it, one called 'rmdel', the
	other 'cdc'.)

	The delta to be removed (or whose MRs and/or comments
	are to be changed) is specified via the
	r argument, in the form of an SID.

	If the delta is to be removed, it must be the most recent one
	in its branch in the delta tree (a so-called 'leaf' delta).
	For either function, the delta being processed must not
	have any 'delivered' MRs, and the user must have basically
	the same permissions as are required to make deltas.

	If a directory is given as an argument, each SCCS file
	within the directory will be processed as if it had been
	specifically named. If a name of '-' is given, the standard
	input will be read for a list of names of SCCS files to be
	processed. Non SCCS files are ignored.
	If the file is CASSI controlled special processing is performed:
	if the cdc command is being executed then the cmrs for the delta are
	list with the choice given about which to delete. This choice is checked 
	against the fred file for validity. If the rmdel command is being performed
	then all cmrs for the delta are checked against fred, if there are any
	non editable cmrs then the change is rejected.
*/

# define COPY 0
# define NOCOPY 1

struct stat Statbuf;
char Null[1];
char Error[128];

struct sid sid;
int num_files;
char D_type;
char 	*Sidhold;
char	*auxf();
char	*Mrs;
char	*Comments;
char	*Darg[NVARGS];
char	*testcmr[25];
char	*Earg[NVARGS];
char	*NVarg[NVARGS];
extern char *Sflags[];
int D_serial;
struct utsname un;
char *uuname;

void	rdpfile(), escdodelt(), rmchg(), put_delmrs(), ckmrs();
void	clean_up(), putmrs(), split_mrs();

extern	char *savecmt();

extern char *gf();
char *strtok();

main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	register char *p;
	char c;
	char *sid_ab();
	extern int Fcnt;

	/*
	Set flags for 'fatal' to issue message, call clean-up
	routine, and terminate processing.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

	for(i=1; i<argc; i++)
		if(argv[i][0] == '-' && (c = argv[i][1])) {
			p = &argv[i][2];
			switch (c) {

			case 'r':
				if (!(*p))
					fatal("r has no sid (rc11)");
				Sidhold=p;
				chksid(sid_ab(p,&sid),&sid);
				break;
			case 'm':	/* MR entry */
				Mrs = p;
				repl(Mrs,'\n',' ');
				break;
			case 'y':	/* comment line */
				Comments = p;
				break;
			case 'z':
				break;
			default:
				fatal("unknown key letter (cm1)");
			}

			if (had[c - 'a']++)
				fatal("key letter twice (cm2)");
			argv[i] = 0;
		}
		else
			num_files++;
	if(HADM && HADZ)
	{
		fatal("Cassi not compatable with cmrs on command line");
	}
	if(num_files == 0)
		fatal("missing file arg (cm3)");

	if (*(p = sname(argv[0])) == 'n')
		p++;
	if (equal(p,"rmdel"))
		D_type = 'R';		/* invoked as 'rmdel' */
	else if (equal(p,"cdc"))
		D_type = 'D';		/* invoked as 'cdc' */
	else
		fatal("bad invocation (rc10)");
	if (! logname())
		fatal("User ID not in password file (cm9)");

	setsig();

	/*
	Change flags for 'fatal' so that it will return to this
	routine (main) instead of terminating processing.
	*/
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	/*
	Call 'rmchg' routine for each file argument.
	*/
	for (i=1; i<argc; i++)
		if (p = argv[i])
			do_file(p,rmchg);

	exit(Fcnt ? 1 : 0);
}


/*
	Routine that actually causes processing of the delta.
	Processing on the file takes place on a
	temporary copy of the SCCS file (the x-file).
	The name of the x-file is the same as that of the
	s-file (SCCS file) with the 's.' replaced by 'x.'.
	At end of processing, the s-file is removed
	and the x-file is renamed with the name of the old s-file.

	This routine makes use of the z-file to lock out simultaneous
	updates to the SCCS file by more than one user.
*/

struct packet gpkt;	/* see file s.h */
char line[BUFSIZ];
int Domrs;

void
rmchg(file)
char *file;
{
	static int first_time = 1;
	struct stats stats;	/* see file s.defines.h */
	struct stat sbuf;
	struct idel *dodelt();
	extern char *Sflags[];
	int n, numdelts;
	char *p, *cp, *getline();
	int keep, found;
	extern char Pgmr[LOGSIZE];
	int fowner, downer, user;

	if (setjmp(Fjmp))	/* set up to return here from 'fatal' */
		return;		/* and return to caller of rmchg */

	if (!HADR)
		fatal("missing r (rc1)");

	if (D_type == 'D' && first_time) {
		first_time = 0;
		dohist(file);
	}

	if (!exists(file)) {
		sprintf(Error,"file %s does not exist (rc2)",file);
		fatal(Error);
	}

	/*
	Lock out any other user who may be trying to process
	the same file.
	*/
	uname(&un);
	uuname = un.nodename;
	if (lockit(auxf(file,'z'),2,getpid(),uuname))
		fatal("cannot create lock file (cm4)");

	sinit(&gpkt,file,1);	/* initialize packet and open s-file */

	/*
	Flag for 'putline' routine to tell it to open x-file
	and allow writing on it.
	*/
	gpkt.p_upd = 1;

	/*
	Save requested SID for later checking of
	permissions (by 'permiss').
	*/
	gpkt.p_reqsid = sid;

	/*
	Now read-in delta table. The 'dodelt' routine
	will read the table and change the delta entry of the
	requested SID to be of type 'R' if this is
	being executed as 'rmdel'; otherwise, for 'cdc', only
	the MR and comments sections will be changed 
	(by 'escdodelt', called by 'dodelt').
	*/
	if (dodelt(&gpkt,&stats,&sid,D_type) == 0)
		fmterr(&gpkt);

	/*
	Get serial number of requested SID from
	delta table just processed.
	*/
	D_serial = sidtoser(&gpkt.p_reqsid,&gpkt);

	/*
	If SID has not been zeroed (by 'dodelt'),
	SID was not found in file.
	*/
	if (sid.s_rel != 0)
		fatal("nonexistent sid (rc3)");
	/*
	Replace 'sid' with original 'sid'
	requested.
	*/
	sid = gpkt.p_reqsid;

	/*
	Now check permissions.
	*/
	finduser(&gpkt);
	doflags(&gpkt);
	permiss(&gpkt);

	/*
	Check that user is either owner of file or
	directory, or is one who made the delta.
	*/
	fstat(fileno(gpkt.p_iop),&Statbuf);
	fowner = Statbuf.st_uid;
	copy(gpkt.p_file,line);		/* temporary for dname() */
	if (stat(dname(line),&Statbuf))
		downer = -1;
	else
		downer = Statbuf.st_uid;
	user = getuid();
	if (user != fowner && user != downer)
		if (!equal(Pgmr,logname())) {
			sprintf(Error,
				"you are neither owner nor '%s' (rc4)",Pgmr);
			fatal(Error);
		}

	/*
	For 'rmdel', check that delta being removed is a
	'leaf' delta, and if ok,
	process the body.
	*/
	if (D_type == 'R') {
		struct idel *ptr;
		for (n = maxser(&gpkt); n > D_serial; n--) {
			ptr = &gpkt.p_idel[n];
			if (ptr->i_pred == D_serial)
				fatal("not a 'leaf' delta (rc5)");
		}

		/*
		   For 'rmdel' check that the sid requested is
		   not contained in p-file, should a p-file
		   exist.
		*/

		if (exists(auxf(gpkt.p_file,'p')))
			rdpfile(&gpkt,&sid);

		flushto(&gpkt,EUSERTXT,COPY);

		keep = YES;
		found = NO;
		numdelts = 0;			/* keeps count of numbers of deltas */
		gpkt.p_chkeof = 1;		/* set EOF is ok */
		while ((p = getline(&gpkt)) != NULL) {
			if (*p++ == CTLCHAR) {
				cp = p++;
				NONBLANK(p);
				/*
				Convert serial number to binary.
				*/
				if (*(p = satoi(p,&n)) != '\n')
					fmterr(&gpkt);
				if (n == D_serial) {
					gpkt.p_wrttn = 1;
					if (*cp == INS) {
						found = YES;
						keep = NO;
					}
					else
						keep = YES;
				}
				if (*cp == INS)
					/*
					   keep track of number of deltas  -
					   do not remove if it's the last one
					*/
					numdelts++;
			}
			else
				if (keep == NO)
					gpkt.p_wrttn = 1;
		}
	}
	else {
		numdelts = 2;    /* dummy - needed for rmdel, not cdc */
		/*
		This is for invocation as 'cdc'.
		Check MRs if not CASSI file.
		*/
		if ((Mrs != NULL) && (*Mrs != '\0')) {
			if (!(p = Sflags[VALFLAG - 'a']))
				fatal("MRs not allowed (rc6)");
			if (*p && valmrs(&gpkt,p))
				fatal("invalid MRs (rc7)");
		}

		/*
		Indicate that EOF at this point is ok, and
		flush rest of s-file to x-file.
		*/
		gpkt.p_chkeof = 1;
		while (getline(&gpkt))
			;
	}

	flushline(&gpkt,(struct stats *) 0);
	
	/* if z flag on and -fz exists on the sccs s.file 
		verify the deleted cmrs and send message
	*/
	if((HADZ && !Sflags[CMFFLAG - 'a']) ||
		(!(HADZ) && Sflags[CMFFLAG - 'a']))
		{
			fatal("invalid use of 'z' flag \n CASSI incompatablity\n");
		}
	else
		{
		if(HADZ)
			{
			/*now process the testcmr table and send messages*/
				if(!verif(testcmr,&gpkt))
				{
					fatal("uneditable cmr for CASSI controlled file- Change not allowed");
				}
			}
		}

	if (numdelts < 2 && found == YES)
		/* they tried to delete the last remaining delta */
		fatal("may not remove the last delta (rc13)");

	/*
	Delete old s-file, change x-file name to s-file.
	*/
	stat(gpkt.p_file,&sbuf);
	rename(auxf(gpkt.p_file,'x'),(char *)&gpkt);
	chmod(gpkt.p_file,sbuf.st_mode);
	chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
	clean_up();
}

void
escdodelt(pkt)
struct packet *pkt;
{
	static int first_time = 1;
	extern int First_esc;
	extern int First_cmt;
	extern int CDid_mrs;
	char	*p;
	char	*date_ba();
	extern char *Sflags[];
	extern long Timenow;

	static short int doneesc;
	/* the CMF -z option is on and MR numb being processed*/
	if((HADZ) && (pkt->p_line[1] == MRNUM) && (D_type == 'D') && (!doneesc))
		{
		 doneesc = 1;
		 if(!esccmfdelt(pkt))
			{
			 fatal("bad cdc replace of cmr's (CASSI)");
			}
		 else
			return;
		}
	/* non cassi processing begins*/	
	if (first_time) {
		/*
		 * if first time calling `escdodelt'
		 * then split the MR list from mrfixup.
		*/

		split_mrs();
		first_time = 0;
	}
	if (D_type == 'D' && First_esc) {	/* cdc, first time */
		First_esc = 0;
		if ((Mrs != NULL) && (*Mrs != '\0')) {
			/*
			 * if adding MRs then put out the new list
			 * from `split_mrs' (if any)
			*/

			putmrs(pkt);
			CDid_mrs = 1;	/* indicate that some MRs were read */
		}
	}

	if (pkt->p_line[1] == MRNUM) {		/* line read is MR line */
		p = pkt->p_line;
		while (*p)
			p++;
		if (*(p - 2) == DELIVER)
			fatal("delta specified has delivered MR (rc9)");
		if (!CDid_mrs)	/* if no MRs list from `dohist' then return */
			return;
		else
			/*
			 * check to see if this MR should be removed
			*/

			ckmrs(pkt,pkt->p_line);
	}
	else if (D_type == 'D') {		/* this line is a comment */
		if (First_cmt) {		/* first comment encountered */
			First_cmt = 0;
			/*
			 * if comments were read by `dohist' then
			 * put them out.
			*/

			if (*Comments) {
				sprintf(line,"%c%c %s\n",CTLCHAR,
					COMMENTS,savecmt(Comments));
				putline(pkt,line);
			}

			/*
			 * if any MRs were deleted, print them out
			 * as comments for this invocation of `cdc'
			*/

			put_delmrs(pkt);
			/*
			 * if comments were read by `dohist' then
			 * notify user that comments were CHANGED.
			*/

			if (*Comments) {
				sprintf(line,"%c%c ",CTLCHAR,COMMENTS);
				putline(pkt,line);
				putline(pkt,"*** CHANGED *** ");
				/* get date and time */
				date_ba(&Timenow,line);	
				putline(pkt,line);
				sprintf(line," %s\n",logname());
				putline(pkt,line);
			}
		}
		else return;
	}
}


/* esccmfdelt(pkt) takes a packet line of cassi cmrs 
*  prompts for those which will be deleted from the delta
*  checks the ones to be deleted against fred
*  it turns the old line into a comment line
*  and makes a new line containing the non deleted cmrs
*  a chpost message is sent for the deleted cmrs
*  whole function 
*/
	esccmfdelt(pkt)
		struct packet *pkt;
		{
 		 extern char *Sidhold; 
		 extern char *Sflags[];
		 char *p,*pp,*pend,holder[100],outhold[110],answ[80],*holdptr[25],str[80];
		 int numcmrs,i,n,j,changed=0;		 
		 /* move the cmr data to a holder*/
		 p = pkt->p_line;
		 p += 3;
		 strcpy(holder,p);
		 i=0;
		 outhold[0]=NULL;
		 holdptr[0]=strtok(holder,",\n");
		 while(holdptr[++i] = strtok(0,",\n"));
		 if(!holdptr[1])
			{
			 /* only one mr on the list */
			 printf("only one cmr left for this delta no change possible \n");
			 return(1);
			}
		 numcmrs = i;
		 n=0;
 		 j=0;
		 while (holdptr[j])
			{
			 printf("do you want to delete %s\n",holdptr[j]);
			 gets(answ);
			 if(imatch("y",answ))
				{
				 if(numcmrs <= 1)
					{
						printf("%s  is not editable now\n",holdptr[j]);
						 cat(outhold,outhold,holdptr[j],",",0);
						 break;
						}
					else
				 		{
						 /* store the cmrnumber in a test table*/
						 testcmr[n]=(char *)malloc((unsigned)strlen(holdptr[j]) +1);
						 strcpy(testcmr[n++],holdptr[j]);
						 numcmrs--;
						 changed=1;
						}
				}
			 else
				{
				 cat(outhold,outhold,holdptr[j],",",0);
				}
			j++;
			}
			if(!changed)
				{
				 return(0);
				}
			/* turn old line into comment */
			p -= 2;
			*p = 'c';
			pend=strend(p);
			*--pend='\n';
			putline(pkt,0);
			/* place new line */
			pp=strend(outhold);
			*pp=NULL;
			*--pp=NULL;
			sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,outhold);
			putline(pkt,str);
			return(0);
		}
/*the fredchk routine verifies that the cmrs for the delta are all editableor
*		 degenerate  before changing the delta*/
void
fredck(pkt)
struct packet *pkt;
{
	char *mrhold[20],*p,holder[80];
	extern char *Sidhold;
	int i,j;
	p = pkt->p_line;
	p += 3;
	strcpy(holder,p);
	mrhold[0]=strtok(holder,"\n,");
	i=0;
	while(mrhold[++i] = strtok(0,"\n,"));
	for(j=0;j<i;j++)
	{
		testcmr[j]=(char *)malloc((unsigned)strlen(mrhold[j]) + 1);
		strcpy(testcmr[j],mrhold[j]);
	}
}
/*verif takes the list of deleted cmrs and checks them agains the fredfile
  if any are invalid the function returns 0
*/
verif(test,pkt)
char *test[];
struct packet *pkt;
{
	char *fred;
	int i;
	/* get the fred file name*/
	if(!(fred=(char *)gf(Sflags[CMFFLAG - 'a'])))
		fatal("No fred file found- see sccs administrator");
	/* check validity of values in test */
	for(i=0;test[i];i++)
		{
			if(!(testfred(test[i],fred)))
				{
					fatal(" %s is not editable \n",test[i]);
					return(0);
				}
		}
	/* write the messages for the valid cmrs */
	for(i=0;test[i];i++)
		{
			msg(Sflags[CMFFLAG - 'a'],pkt->p_file,test[i],"nc",Sidhold,fred);
		}
	return(1);
}

/*the  msg subroutine creates the command line to go to the mr subsystem*/
msg(app,name,cmrs,stats,sids,fred)
	char *app,*name,*cmrs,*stats,*sids,*fred;

	{
 	FILE *fd;
	 char pname[FILESIZE],*ptr,holdfred[100],dir[100],path[FILESIZE];
	 struct stat stbuf;
	 int noexist = 0;
	 char *k;
	char *p, *getcwd();
	extern char *strchar();
	
	/*if -fm its value is made the file name */
	if(k=Sflags[MODFLAG -'a'])
	{
		name = k;
	}
	if( *name != '/' )   /* NOT full path name */
	{
		if(( p = getcwd(path,sizeof(path))) == NULL)
                 	fatal("getcwd() failed (ge=20)");
		cat(pname,path,"/",name,0);
	}
	else
	{
		strcpy(pname,name);
	}
	strcpy(holdfred,fred);
	ptr=(char *)strchr(holdfred,'.');
	*ptr=(char)NULL;
	 strcat(holdfred,"source");
	 strcpy(dir,holdfred);
	 strcat(holdfred,"/termLOG");
	 if(stat(holdfred,&stbuf) == -1)
		noexist = 1; /*new termLOG */
	if(!(fd=fopen(holdfred,"a")))
		{
			fatal("CASSI msg not writable \n");
			return(0);
		}
	fprintf(fd,"%s chpost %s q %s sw MID=%s MFS=%s MPA=%s q q\n",app,cmrs,pname,sids,stats,logname());
	fclose(fd);
	 if(noexist) /*new termLOG make owner of /BD/source owner of file */
	 {
		if(stat(dir,&stbuf) == -1)
		{
			fatal("Cassi BD/source not writeable\n");
		}
		chmod(holdfred,0666);
		chown(holdfred,(int)stbuf.st_uid,(int)stbuf.st_gid); 
	}
	return(1);
}
/* testfred takes the fredfile and the cmr and checks to see if the cmr is in 
*the fred  file
*/
testfred(cmr,fredfile)
	char *cmr,*fredfile;
	{
		extern sweep();
		char dcmr[50];
		char *cmrh[2],*dcmrh[2];
		cmrh[1] = (char *) NULL;
		dcmrh[1] = (char *) NULL;
		cat(dcmr,"D",cmr,0);
		cmrh[0]=cmr;
		dcmrh[0]=dcmr;
		return(sweep(SEQVERIFY,fredfile,NULL,'\n',WHITE,80,cmrh,NULL,NULL,
		(int(*)())NULL,(int(*)())NULL) == FOUND || sweep(SEQVERIFY,fredfile
		,NULL,'\n',WHITE,80,dcmrh,NULL,NULL,(int(*)())NULL,(int(*)())NULL)
		== FOUND);
	}
	

extern char **Varg;

void
split_mrs()
{
	register char **argv;
	char	**dargv;
	char	**nargv;
	char	*ap, *stalloc();

	dargv = &Darg[VSTART];
	nargv = &NVarg[VSTART];
	for (argv = &Varg[VSTART]; *argv; argv++)
		if (*argv[0] == '!') {
			*argv += 1;
			ap = *argv;
			copy(ap,*dargv++ = stalloc(size(ap)));
			*dargv = 0;
			continue;
		}
		else {
			copy(*argv,*nargv++ = stalloc(size(*argv)));
			*nargv = 0;
		}
	Varg = NVarg;
}

void
putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char	str[64];

	for(argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}

void
clean_up()
{
	if(gpkt.p_iop)
		fclose(gpkt.p_iop);
	xrm();
	if (exists(auxf(gpkt.p_file,'x')))
		xunlink(auxf(gpkt.p_file,'x'));
	ffreeall();
	if (gpkt.p_file[0]) {
		uname(&un);
		uuname = un.nodename;
		unlockit(auxf(gpkt.p_file,'z'),getpid(),uuname);
	}
}

void
rdpfile(pkt,sp)
register struct packet *pkt;
struct sid *sp;
{
	struct pfile pf;
	char rline[BUFSIZ];
	FILE *in, *fdfopen();

	in = xfopen(auxf(pkt->p_file,'p'),0);
	while (fgets(rline,sizeof(line),in) != NULL) {
		pf_ab(rline,&pf,1);
		if (sp->s_rel == pf.pf_gsid.s_rel &&
			sp->s_lev == pf.pf_gsid.s_lev &&
			sp->s_br == pf.pf_gsid.s_br &&
			sp->s_seq == pf.pf_gsid.s_seq) {
				fclose(in);
				fatal("being edited -- sid is in p-file (rc12)");
		}
	}
	fclose(in);
	return;
}

void
ckmrs(pkt,p)
struct packet *pkt;
char *p;
{
	register char **argv, **eargv;
	char mr_no[BUFSIZ];
	char *mr_ptr, *stalloc();

	eargv = &Earg[VSTART];
	copy(p,mr_no);
	mr_ptr = mr_no;
	repl(mr_ptr,'\n','\0');
	mr_ptr += 3;
	for (argv = &Darg[VSTART]; *argv; argv++)
		if (equal(mr_ptr,*argv)) {
			pkt->p_wrttn = 1;
			copy(mr_ptr,*eargv++ = stalloc(size(mr_ptr)));
			*eargv = 0;
		}
}

void
put_delmrs(pkt)
struct	packet	*pkt;
{

	register char	**argv;
	register int first;
	char	str[64];

	first = 1;
	for(argv = &Earg[VSTART]; *argv; argv++) {
		if (first) {
			putline(pkt,"c *** LIST OF DELETED MRS ***\n");
			first = 0;
		}
		sprintf(str,"%c%c %s\n",CTLCHAR,COMMENTS,*argv);
		putline(pkt,str);
	}
}
