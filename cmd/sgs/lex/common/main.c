/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lex:main.c	1.15"
# include "once.c"

	/* lex [-[dcyvntf]] [file] ... [file] */

	/* Copyright 1976, Bell Telephone Laboratories, Inc.,
	    written by Eric Schmidt, August 27, 1976   */

main(argc,argv)  int argc; char *argv[]; {
	register int i;
	int c;
	extern char *optarg;
	extern int  getopt();

# ifdef DEBUG
	signal(10,buserr);
	signal(11,segviol);
# endif

	sargv = argv;
	sargc = argc;
# ifdef DEBUG
	while ((c = getopt(argc, argv, "l:dyctvnVQ:")) != EOF ) {
# else
	while ((c = getopt(argc, argv, "l:ctvnVQ:")) != EOF ) {
# endif
		switch (c) {
# ifdef DEBUG
			case 'd': 
				debug++; 
				break;
			case 'y': 
				yydebug = TRUE; 
				break;
# endif
			case 'V':
				(void) fprintf(stderr,"lex:%s, %s\n", ESG_PKG, ESG_REL);
				break;
			case 'Q':
				v_stmp = optarg;
				if (*v_stmp != 'y' && *v_stmp != 'n' )
					error("lex: -Q should be followed by [y/n]");
				break;
			case 'l':
				cname = optarg;
				break;
			case 'c':
				ratfor=FALSE; 
				break;
			case 't':
				fout = stdout;
				break;
			case 'v':
				report = 1;
				break;
			case 'n':
				report = 0;
				break;
			default:
# ifdef DEBUG
				(void)fprintf(stderr,"Usage: lex [-dyctvnV] [-Q(y/n)] [files...]\n");
# else
				(void) fprintf(stderr,"Usage: lex [-ctvnV] [-Q(y/n)] [files...]\n");
# endif
				exit(1);
		}
	}
	no_input = argc - optind;
	if (no_input) {
		fin = fopen(argv[optind], "r");
		if(fin == NULL) error ("Can't open input file -- %s", argv[optind]);
	} else fin = stdin;

	/* may be gotten: def, subs, sname, schar, ccl, dchar */
	(void)gch();

	/* may be gotten: name, left, right, nullstr, parent */
	get1core();

	scopy("INITIAL",sp);
	sname[0] = sp;
	sp += slength("INITIAL") + 1;
	sname[1] = 0;

	/* may be disposed of: def, subs, dchar */
	if(yyparse(0)) exit(1);	/* error return code */

	/* maybe get: 	
	**		tmpstat, foll, positions, gotof, nexts, 
	**		nchar, state, atable, sfall, cpackflg 
	*/
	free1core();
	get2core();
	ptail();
	mkmatch();
# ifdef DEBUG
	if(debug) pccl();
# endif
	sect  = ENDSECTION;
	if(tptr>0)cfoll(tptr-1);
# ifdef DEBUG
	if(debug)pfoll();
# endif
	cgoto();
# ifdef DEBUG
	if(debug){
		(void)printf("Print %d states:\n",stnum+1);
		for(i=0;i<=stnum;i++)stprt(i);
		}
# endif
	/* may be disposed of: 
	**		positions, tmpstat, foll, state, name, 
	**		left, right, parent, ccl, schar, sname
	** maybe get:	 verify, advance, stoff 
	*/
	free2core();
	get3core();
	layout();
	/* may be disposed of: 
	**		verify, advance, stoff, nexts, nchar,
	**		gotof, atable, ccpackflg, sfall 
	*/

# ifdef DEBUG
	free3core();
# endif
	fother = fopen(ratfor?ratname:cname,"r");
	if(fother == NULL)
		error("Lex driver missing, file %s",ratfor?ratname:cname);
	while ( (i=getc(fother)) != EOF)
		(void) putc((char)i,fout);

	(void) fclose(fother);
	(void) fclose(fout);
	if(
# ifdef DEBUG
		debug   ||
# endif
			report == 1)statistics();
	(void) fclose(stdout);
	(void) fclose(stderr);
	exit(0);	/* success return code */
	/* NOTREACHED */
}

get1core(){
	ccptr =	ccl = myalloc(CCLSIZE,sizeof(*ccl));
	pcptr = pchar = (unsigned char *)myalloc(pchlen, sizeof(*pchar));
	def = (char **)myalloc(DEFSIZE,sizeof(*def));
	subs = (char **)myalloc(DEFSIZE,sizeof(*subs));
	dp = dchar = myalloc(DEFCHAR,sizeof(*dchar));
	sname = (char **)myalloc(STARTSIZE,sizeof(*sname));
	sp = 	schar = myalloc(STARTCHAR,sizeof(*schar));
	if(ccl == 0 || def == 0 || pchar == 0 || subs == 0 
		|| dchar == 0 || sname == 0 || schar == 0)
		error("Too little core to begin");
	}

free1core(){
	cfree((char *)def,DEFSIZE,sizeof(*def));
	cfree((char *)subs,DEFSIZE,sizeof(*subs));
	cfree((char *)dchar,DEFCHAR,sizeof(*dchar));
	}

get2core(){
	register int i;
	gotof = (int *)myalloc(nstates,sizeof(*gotof));
	nexts = (int *)myalloc(ntrans,sizeof(*nexts));
	nchar = (unsigned char *)myalloc(ntrans,sizeof(*nchar));
	state = (int **)myalloc(nstates,sizeof(*state));
	atable = (int *)myalloc(nstates,sizeof(*atable));
	sfall = (int *)myalloc(nstates,sizeof(*sfall));
	cpackflg = myalloc(nstates,sizeof(*cpackflg));
	tmpstat = myalloc(tptr+1,sizeof(*tmpstat));
	foll = (int **)myalloc(tptr+1,sizeof(*foll));
	nxtpos = positions = (int *)myalloc(maxpos,sizeof(*positions));
	if(tmpstat == 0 || foll == 0 || positions == 0 ||
		gotof == 0 || nexts == 0 || nchar == 0 || state == 0 || atable == 0 || sfall == 0 || cpackflg == 0 )
		error("Too little core for state generation");
	for(i=0;i<=tptr;i++)foll[i] = 0;
	}
free2core(){
	cfree((char *)positions,maxpos,sizeof(*positions));
	cfree((char *)tmpstat,tptr+1,sizeof(*tmpstat));
	cfree((char *)foll,tptr+1,sizeof(*foll));
	cfree((char *)name,treesize,sizeof(*name));
	cfree((char *)left,treesize,sizeof(*left));
	cfree((char *)right,treesize,sizeof(*right));
	cfree((char *)parent,treesize,sizeof(*parent));
	cfree((char *)nullstr,treesize,sizeof(*nullstr));
	cfree((char *)state,nstates,sizeof(*state));
	cfree((char *)sname,STARTSIZE,sizeof(*sname));
	cfree((char *)schar,STARTCHAR,sizeof(*schar));
	cfree((char *)ccl,CCLSIZE,sizeof(*ccl));
	}
get3core(){
	verify = (int *)myalloc(outsize,sizeof(*verify));
	advance = (int *)myalloc(outsize,sizeof(*advance));
	stoff = (int *)myalloc(stnum+2,sizeof(*stoff));
	if(verify == 0 || advance == 0 || stoff == 0)
		error("Too little core for final packing");
	}
# ifdef DEBUG
free3core(){
	cfree((char *)advance,outsize,sizeof(*advance));
	cfree((char *)verify,outsize,sizeof(*verify));
	cfree((char *)stoff,stnum+1,sizeof(*stoff));
	cfree((char *)gotof,nstates,sizeof(*gotof));
	cfree((char *)nexts,ntrans,sizeof(*nexts));
	cfree((char *)nchar,ntrans,sizeof(*nchar));
	cfree((char *)atable,nstates,sizeof(*atable));
	cfree((char *)sfall,nstates,sizeof(*sfall));
	cfree((char *)cpackflg,nstates,sizeof(*cpackflg));
	}
# endif
char *myalloc(a,b)
  int a,b; {
	register char *i;
	i = calloc(a, b);
	if(i==0)
		warning("calloc returns a 0");
	/* else if(i == -1){
# ifdef DEBUG
		warning("calloc returns a -1");
# endif
		return(0);
		} */
	return(i);
	}
# ifdef DEBUG
buserr(){
	(void) fflush(errorf);
	(void) fflush(fout);
	(void) fflush(stdout);
	(void)fprintf(errorf,"Bus error\n");
	if(report == 1)statistics();
	(void) fflush(errorf);
	}
segviol(){
	(void) fflush(errorf);
	(void) fflush(fout);
	(void) fflush(stdout);
	(void)fprintf(errorf,"Segmentation violation\n");
	if(report == 1)statistics();
	(void) fflush(errorf);
	}
# endif

#ifdef __cplusplus
void yyerror(const char *s)
#else
yyerror(s)
char *s;
#endif
{
	(void)fprintf(stderr, "\"%s\":line %d: Error: %s\n", sargv[optind], yyline, s);
}
