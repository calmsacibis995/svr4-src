/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)yacc:y2.c	1.33"
# include "dextern"
# define IDENTIFIER 257
# define MARK 258
# define TERM 259
# define LEFT 260
# define RIGHT 261
# define BINARY 262
# define PREC 263
# define LCURLY 264
# define C_IDENTIFIER 265  /* name followed by colon */
# define NUMBER 266
# define START 267
# define TYPEDEF 268
# define TYPENAME 269
# define UNION 270
# define ENDFILE 0
#define LHS_TEXT_LEN		80	/* length of lhstext */
#define RHS_TEXT_LEN		640	/* length of rhstext */
	/* communication variables between various I/O routines */

static char *infile;			/* input file name 		*/
static int numbval;			/* value of an input number 	*/
static int toksize = NAMESIZE;
static char *tokname;	/* input token name		*/
char *parser=NULL;		/* location of common parser 	*/

static void finact();
static char *cstash();
static void defout();
static void cpyunion();
static void cpycode();
static void cpyact();
static void lhsfill();
static void rhsfill();
static void lrprnt();
static void beg_debug();
static void end_toks();
static void end_debug();
static void exp_tokname();
static void exp_prod();
static void exp_ntok();
static void exp_nonterm();
static defin();
static gettok();
static chfind();
static skipcom();

	/* storage of names */

static char cnamesblk0[CNAMSZ];	/* initial block to place token and 	*/
				/* nonterminal names are stored 	*/
static char *cnames = cnamesblk0;	/* points to initial block - more space	*/
				/* is allocated as needed.		*/
static char *cnamp = cnamesblk0;	/* place where next name is to be put in */
static int ndefout = 3;  		/* number of defined symbols output */

	/* storage of types */
static int defunion = 0;	/* union of types defined? */
static int ntypes = 0;		/* number of types defined */
static char * typeset[NTYPES];	/* pointers to type tags */

	/* symbol tables for tokens and nonterminals */

int ntokens = 0;
int ntoksz = NTERMS;
TOKSYMB *tokset;
int *toklev;

int nnonter = -1;
NTSYMB *nontrst;
int nnontersz = NNONTERM;

static int start;	/* start symbol */

	/* assigned token type values */
static int extval = 0;

	/* input and output file descriptors */

FILE * finput;		/* yacc input file */
FILE * faction;		/* file for saving actions */
FILE * fdefine;		/* file for # defines */
FILE * ftable;		/* y.tab.c file */
FILE * ftemp;		/* tempfile to pass 2 */
FILE * fdebug;		/* where the strings for debugging are stored */
FILE * foutput;		/* y.output file */

	/* output string */

static char *lhstext;
static char *rhstext;

	/* storage for grammar rules */

int *mem0; /* production storage */
int *mem;
int *tracemem;
extern int *optimmem;
int new_memsize = MEMSIZE;
int nprod= 1;	/* number of productions */
int nprodsz = NPROD;

int **prdptr;
int *levprd;
char *had_act;

static int gen_lines = 1;	/* flag for generating the # line's default is yes */
static int gen_testing = 0;	/* flag for whether to include runtime debugging */
static char *v_stmp = "n";	/* flag for version stamping--default turned off */

void setup(argc,argv) int argc; char *argv[];
{	int ii,i,j,lev,t, ty; /*ty is the sequencial number of token name in tokset*/
	int c;
	int *p;
	char actname[8];

	extern char	*optarg;
	extern int	optind;
	extern 		getopt();

	foutput = NULL;
	fdefine = NULL;
	i = 1;

	tokname = (char *) malloc(sizeof(char) * toksize);
	tokset = (TOKSYMB *)malloc(sizeof(TOKSYMB) * ntoksz);
	toklev = (int *) malloc(sizeof(int) * ntoksz);
	nontrst = (NTSYMB *) malloc (sizeof(NTSYMB) * nnontersz);
	mem0 = (int *)malloc(sizeof(int) * new_memsize);
	prdptr = (int **) malloc(sizeof(int *) * (nprodsz+2));
	levprd = (int *) malloc(sizeof(int) * (nprodsz+2));
	had_act = (char *) calloc((nprodsz+2), sizeof(char));
	lhstext = (char *) malloc(sizeof(char) * LHS_TEXT_LEN);
	rhstext = (char *) malloc(sizeof(char) * RHS_TEXT_LEN);
	aryfil(toklev,ntoksz,0);
	aryfil(levprd,nprodsz,0);
	for( ii =0; ii<ntoksz; ++ii) tokset[ii].value = 0;
	for( ii =0; ii<nnontersz; ++ii) nontrst[ii].tvalue = 0;
	aryfil(mem0, new_memsize,0);
	mem= mem0;
	tracemem = mem0;

	while ((c = getopt(argc, argv, "vVdltp:Q:")) != EOF)
		switch (c) {
			case 'v':
				foutput = fopen(FILEU, "w" );
				if( foutput == NULL ) error( "cannot open y.output" );
				break;
			case 'V':
				(void) fprintf(stderr, "yacc:%s, %s\n", ESG_PKG, ESG_REL);
				break;
			case 'Q':
				v_stmp = optarg;
				if (*v_stmp != 'y' && *v_stmp != 'n')
					error("yacc: -Q should be followed by [y/n]");
				break;
			case 'd':
				fdefine = fopen( FILED, "w" );
				if ( fdefine == NULL ) error( "cannot open y.tab.h" );
				break;
			case 'l':
				gen_lines = 0;	/* don't gen #lines */
				break;
			case 't':
				gen_testing = 1;	/* set YYDEBUG on */
				break;
			case 'p':
				parser = optarg; 
				break;

			case '?':
			default:
				(void) fprintf(stderr,"Usage: yacc [-vVdlt] [-Q(y/n)] [-p driver_file] file\n");
				exit(1);
		}

	fdebug = fopen( DEBUGNAME, "w" );
	if ( fdebug == NULL ) error( "cannot open yacc.debug" );
	ftable = fopen( OFILE, "w" );
	if ( ftable == NULL ) error( "cannot open y.tab.c" );

	ftemp = fopen( TEMPNAME, "w" );
	faction = fopen( ACTNAME, "w" );
	if( ftemp==NULL || faction==NULL ) error( "cannot open temp file" );

	if ((finput=fopen( infile=argv[optind], "r" )) == NULL )
		error( "cannot open input file" );

	lineno =1;
	cnamp = cnames;
	(void) defin(0,"$end");
	extval = 0400;
	(void) defin(0,"error");
	(void) defin(1,"$accept");
	mem=mem0;
	lev = 0;
	ty = 0;
	i=0;
	beg_debug();	/* initialize fdebug file */

	/* sorry -- no yacc parser here.....
		we must bootstrap somehow... */

	t=gettok();
	if ( *v_stmp == 'y' )
		(void) fprintf(ftable, "#ident\t\"yacc: %s\"\n", ESG_REL);
	for( ; t!=MARK && t!= ENDFILE; ){
		int tok_in_line;
		switch( t ){

		case ';':
			t = gettok();
			break;

		case START:
			if( (t=gettok()) != IDENTIFIER ){
				error( "bad %%start construction" );
				}
			start = chfind(1,tokname);
			t = gettok();
			continue;

		case TYPEDEF:
			tok_in_line = 0;
			if( (t=gettok()) != TYPENAME ) error( "bad syntax in %%type" );
			ty = numbval;
			for(;;){
				t = gettok();
				switch( t ){

				case IDENTIFIER:
					tok_in_line=1;
					if (tokname[0] == ' ')	/* LITERAL */
						break;
					else if( (t=chfind( 1, tokname ) ) < NTBASE ) {
						j = TYPE( toklev[t] );
						if( j!= 0 && j != ty ){
							error( "type redeclaration of token %s",
								tokset[t].name );
							}
						else SETTYPE( toklev[t],ty);
					} else {
						j = nontrst[t-NTBASE].tvalue;
						if( j != 0 && j != ty ){
							error( "type redeclaration of nonterminal %s",
								nontrst[t-NTBASE].name );
							}
						else nontrst[t-NTBASE].tvalue = ty;
						}
					/* FALLTHRU */
				case ',':
					continue;

				case ';':
					t = gettok();
					break;
				default:
					break;
					}
				if (!tok_in_line)
					error("missing tokens or illegal tokens");
				break;
				}
			continue;

		case UNION:
			/* copy the union declaration to the output */
			cpyunion();
			defunion = 1;
			t = gettok();
			continue;

		case LEFT:
		case BINARY:
		case RIGHT:
			i++;
			/* FALLTHRU */
		case TERM:
			tok_in_line = 0;
			lev = (t-TERM) | 04;  /* nonzero means new prec. and assoc. */
			ty = 0;

			/* get identifiers so defined */

			t = gettok();
			if( t == TYPENAME ){ /* there is a type defined */
				ty = numbval;
				t = gettok();
				}

			for(;;) {
				switch( t ){

				case ',':
					t = gettok();
					continue;

				case ';':
					break;

				case IDENTIFIER:
					tok_in_line=1;
					j = chfind(0,tokname);
					if( lev & ~04 ){
						if( ASSOC(toklev[j])&~04 ) error( "redeclaration of precedence of %s", tokname );
						SETASC(toklev[j],lev);
						SETPLEV(toklev[j],i);
					} else {
						if( ASSOC(toklev[j]) ) (void)fprintf(stderr, "Warning: redeclaration of precedence of %s, line %d\n", tokname, lineno );
						SETASC(toklev[j],lev);
						}
					if( ty ){
						if( TYPE(toklev[j]) ) error( "redeclaration of type of %s", tokname );
						SETTYPE(toklev[j],ty);
						}
					if( (t=gettok()) == NUMBER ){
						tokset[j].value = numbval;
						if( j < ndefout && j>2 ){
							error( "type number of %s should be defined earlier",
								tokset[j].name );
							}
						if (numbval >= -YYFLAG1) {
							error("token numbers must be less than %d", -YYFLAG1);
							}
						t=gettok();
						}
					continue;

					}
				if ( !tok_in_line )
					error("missing tokens or illegal tokens");
				break;
				}

			continue;

		case LCURLY:
			defout();
			cpycode();
			t = gettok();
			continue;

		default:
			error( "syntax error" );

			}

		}

	if( t == ENDFILE ){
 		error( "unexpected EOF before %%%%" );
		}

	/* t is MARK */

	defout();
	end_toks();	/* all tokens dumped - get ready for reductions */

	(void) fprintf( ftable, "\n#include <malloc.h>\n" );
	(void) fprintf( ftable, "#include <memory.h>\n" );
	(void) fprintf( ftable, "#include <values.h>\n" );
	(void) fprintf( ftable, "\n#ifdef __cplusplus\n");
	(void) fprintf( ftable, "\n#ifndef yyerror\n");
	(void) fprintf( ftable, "	void yyerror(const char *);\n" );
	(void) fprintf( ftable, "#endif\n");
	(void) fprintf( ftable, "#ifndef yylex\n");
	(void) fprintf( ftable, "	int yylex(void);\n" );
	(void) fprintf( ftable, "#endif\n");
	(void) fprintf( ftable, "	int yyparse(void);\n" );
	(void) fprintf( ftable, "\n#endif\n" );
	(void) fprintf( ftable, "#define yyclearin yychar = -1\n" );
	(void) fprintf( ftable, "#define yyerrok yyerrflag = 0\n" );
	(void) fprintf( ftable, "extern int yychar;\nextern int yyerrflag;\n");
	if (!(defunion || ntypes)) 
		(void) fprintf(ftable,"#ifndef YYSTYPE\n#define YYSTYPE int\n#endif\n");
	(void) fprintf( ftable, "YYSTYPE yylval;\n" );
	(void) fprintf( ftable, "YYSTYPE yyval;\n" );
	(void) fprintf( ftable, "typedef int yytabelem;\n" );
	(void) fprintf( ftable, "#ifndef YYMAXDEPTH\n#define YYMAXDEPTH 150\n#endif\n");
	(void) fprintf( ftable, "#if YYMAXDEPTH > 0\n" );
	(void) fprintf( ftable, "int yy_yys[YYMAXDEPTH], *yys = yy_yys;\n" );
	(void) fprintf( ftable, "YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;\n" );
	(void) fprintf( ftable, "#else	/* user does initial allocation */\n" );
	(void) fprintf( ftable, "int *yys;\nYYSTYPE *yyv;\n#endif\n" );
	(void) fprintf( ftable, "static int yymaxdepth = YYMAXDEPTH;\n" );

	prdptr[0]=mem;
	/* added production */
	*mem++ = NTBASE;
	*mem++ = start;  /* if start is 0, we will overwrite with the lhs of the first rule */
	*mem++ = 1;
	*mem++ = 0;
	prdptr[1]=mem;

	while( (t=gettok()) == LCURLY ) cpycode();

	if( t != C_IDENTIFIER ) error( "bad syntax on first rule" );

	if( !start ) prdptr[0][1] = chfind(1,tokname);

	/* read rules */

	while( t!=MARK && t!=ENDFILE ){


		/* process a rule */

		if( t == '|' ){
			rhsfill( (char *) 0 );	/* restart fill of rhs */
			*mem = *prdptr[nprod-1];
			if ( ++mem >= &tracemem[new_memsize] ) exp_mem(1);
			}
		else if( t == C_IDENTIFIER ){
			*mem = chfind(1,tokname);
			if( *mem < NTBASE )error("illegal nonterminal in grammar rule");
			if ( ++mem >= &tracemem[new_memsize] ) exp_mem(1);
			lhsfill( tokname );	/* new rule: restart strings */
			}
		else error( "illegal rule: missing semicolon or | ?" );

		/* read rule body */


		t = gettok();
	more_rule:
		while( t == IDENTIFIER ) {
			*mem = chfind(1,tokname);
			if( *mem<NTBASE ) levprd[nprod] = toklev[*mem]& ~04;
			if ( ++mem >= &tracemem[new_memsize] ) exp_mem(1);
			rhsfill( tokname );	/* add to rhs string */
			t = gettok();
			}

		if( t == PREC ){
			if( gettok()!=IDENTIFIER) error( "illegal %%prec syntax" );
			j = chfind(2,tokname);
			if( j>=NTBASE)error("nonterminal %s illegal after %%prec", nontrst[j-NTBASE].name);
			levprd[nprod]=toklev[j]& ~04;
			t = gettok();
			}

		if( t == '=' ){
			had_act[nprod] = 1;
			levprd[nprod] |= ACTFLAG;
			(void) fprintf( faction, "\ncase %d:", nprod );
			cpyact( mem-prdptr[nprod]-1 );
			(void) fprintf( faction, " break;" );
			if( (t=gettok()) == IDENTIFIER ){
				/* action within rule... */

				lrprnt();		/* dump lhs, rhs */
				(void) sprintf( actname, "$$%d", nprod );
				j = chfind(1,actname);  /* make it a nonterminal */

				/* the current rule will become rule number nprod+1 */
				/* move the contents down, and make room for the null */

				for( p=mem; p>=prdptr[nprod]; --p ) p[2] = *p;
				mem += 2;
				if ( mem >= &tracemem[new_memsize] ) exp_mem(1);

				/* enter null production for action */

				p = prdptr[nprod];

				*p++ = j;
				*p++ = -nprod;

				/* update the production information */

				levprd[nprod+1] = levprd[nprod] & ~ACTFLAG;
				levprd[nprod] = ACTFLAG;

				if( ++nprod >= nprodsz) 
					exp_prod();
				prdptr[nprod] = p;

				/* make the action appear in the original rule */
				*mem++ = j;
				if ( mem >= &tracemem[new_memsize] ) exp_mem(1);

				/* get some more of the rule */

				goto more_rule;
				}

			}

		while ( t == ';' ) t=gettok();

		*mem++ = -nprod;
		if ( mem >= &tracemem[new_memsize] ) exp_mem(1);

		/* check that default action is reasonable */

		if( ntypes && !(levprd[nprod]&ACTFLAG) && nontrst[*prdptr[nprod]-NTBASE].tvalue ){
			/* no explicit action, LHS has value */
			register tempty;
			tempty = prdptr[nprod][1];
			if( tempty < 0 ) error( "must return a value, since LHS has a type" );
			else if( tempty >= NTBASE ) tempty = nontrst[tempty-NTBASE].tvalue;
			else tempty = TYPE( toklev[tempty] );
			if( tempty != nontrst[*prdptr[nprod]-NTBASE].tvalue ){
				error( "default action causes potential type clash" );
				}
			}

		if (++nprod >= nprodsz)
			exp_prod();
		prdptr[nprod] = mem;
		levprd[nprod]=0;

		}

	/* end of all rules */

	end_debug();		/* finish fdebug file's input */
	finact();
	if( t == MARK ){
		if ( gen_lines )
			(void) fprintf( ftable, "\n# line %d \"%s\"\n",
				lineno, infile );
		while( (c=getc(finput)) != EOF ) (void) putc( (char)c, ftable );
		}
	(void) fclose( finput );
	}

static void finact(){
	/* finish action routine */

	(void) fclose(faction);

	(void) fprintf( ftable, "# define YYERRCODE %d\n", tokset[2].value );

	}

static char *
cstash( s ) register char *s; {
	char *temp;
	static int used = 0;
	static int used_save = 0;
	static int exp_cname = CNAMSZ;
	int len = strlen(s);

	/* 2/29/88 -
	** Don't need to expand the table, just allocate new space.
	*/
	used_save = used;
	while (len >= (exp_cname - used_save)) {
		exp_cname += CNAMSZ;
		if (!used) free((char *)cnames);
		if ((cnames = (char *) malloc(sizeof(char)*exp_cname)) == NULL)
			error("cannot expand string dump");
		cnamp = cnames;
		used = 0;
	}

	temp = cnamp;
	do {
		*cnamp++ = *s;
	} while ( *s++ );
	used += cnamp - temp;
	return( temp );
	}

static 
defin( t, s ) register char  *s; {
	/* define s to be a terminal if t=0 or a nonterminal if t=1 */

	register val;

	if (t) {
		if (++nnonter >= nnontersz)
			exp_nonterm();
		nontrst[nnonter].name = cstash(s);
		return( NTBASE + nnonter );
		}
	/* must be a token */
	if( ++ntokens >= ntoksz ) 
		exp_ntok();
	tokset[ntokens].name = cstash(s);

	/* establish value for token */

	if( s[0]==' ' && s[2]=='\0' ) /* single character literal */
		val = s[1];
	else if ( s[0]==' ' && s[1]=='\\' ) { /* escape sequence */
		if( s[3] == '\0' ){ /* single character escape sequence */
			switch ( s[2] ){
					 /* character which is escaped */
			case 'a': (void)fprintf(stderr,"Warning: \\a is ANSI C \"alert\" character, line %d\n", lineno);
				  val = '\a'; break;
			case 'v': val = '\v'; break;
			case 'n': val = '\n'; break;
			case 'r': val = '\r'; break;
			case 'b': val = '\b'; break;
			case 't': val = '\t'; break;
			case 'f': val = '\f'; break;
			case '\'': val = '\''; break;
			case '"': val = '"'; break;
			case '?': val = '?'; break;
			case '\\': val = '\\'; break;
			default: error( "invalid escape" );
			}
		} else if( s[2] <= '7' && s[2]>='0' ){ /* \nnn sequence */
			int i = 3;
			val = s[2] - '0';
			while(isdigit(s[i]) && i<=4) {
				if(s[i]>='0' && s[i]<='7') val = val * 8 + s[i]-'0';
				else error("illegal octal number");
				i++;
			}
			if( s[i] != '\0' ) error("illegal \\nnn construction" );
			if( val > 255 ) error(" \\nnn exceed \\377");
			if( val == 0 && i >= 4) error( "'\\000' is illegal" );
		} else if( s[2] == 'x' ) { /* hexadecimal \xnnn sequence */
			int i = 3;
			val = 0;
			(void)fprintf(stderr,"Warning: \\x is ANSI C hex escape, line %d\n", lineno);
			if (isxdigit(s[i]))
				while (isxdigit(s[i])) {
					int tmpval;
					if(isdigit(s[i]))
						tmpval = s[i] - '0';
					else if ( s[i] >= 'a' )
						tmpval = s[i] - 'a' + 10;
					else 
						tmpval = s[i] - 'A' + 10;
					val = 16 * val + tmpval;
					i++;
				}
			else error( "illegal hexadecimal number" );
			if( s[i] != '\0' ) error("illegal \\xnn construction" );
			if( val > 255 ) error(" \\xnn exceed \\xff");
			if( val == 0 ) error("'\\x00' is illegal");
		} else error( "invalid escape" );
	} else {
		val = extval++;
		}
	tokset[ntokens].value = val;
	toklev[ntokens] = 0;
	return( ntokens );
	}

static void
defout(){ /* write out the defines (at the end of the declaration section) */

	register int i, c;
	register char *cp;

	for( i=ndefout; i<=ntokens; ++i ){

		cp = tokset[i].name;
		if( *cp == ' ' )	/* literals */
		{
			(void) fprintf( fdebug, "\t\"%s\",\t%d,\n",
				tokset[i].name + 1, tokset[i].value );
			continue;	/* was cp++ */
		}

		for( ; (c= *cp)!='\0'; ++cp ){

			if(islower(c)|| isupper(c)|| isdigit(c)|| c=='_')/* EMPTY */;
			else goto nodef;
			}

		(void) fprintf( fdebug, "\t\"%s\",\t%d,\n", tokset[i].name, tokset[i].value );
		(void) fprintf( ftable, "# define %s %d\n", tokset[i].name, tokset[i].value );
		if( fdefine != NULL ) (void) fprintf( fdefine, "# define %s %d\n", tokset[i].name, tokset[i].value );

	nodef:	;
		}

	ndefout = ntokens+1;

	}

static 
gettok() {
	register i, base;
	static int peekline; /* number of '\n' seen in lookahead */
	register c, match, reserve;

begin:
	reserve = 0;
	lineno += peekline;
	peekline = 0;
	c = getc(finput);
	while( c==' ' || c=='\n' || c=='\t' || c=='\f' ){
		if( c == '\n' ) ++lineno;
		c=getc(finput);
	}
	if( c == '/' ){ /* skip comment */
		lineno += skipcom();
		goto begin;
	}

	switch(c){

	case EOF:
		return(ENDFILE);
	case '{':
		(void) ungetc( c, finput );
		return( '=' );  /* action ... */
	case '<':  /* get, and look up, a type name (union member name) */
		i = 0;
		while( (c=getc(finput)) != '>' && c!=EOF && c!= '\n' ){
			tokname[i] = (char)c;
			if( ++i >= toksize )
				exp_tokname();
			}
		if( c != '>' ) error( "unterminated < ... > clause" );
		tokname[i] = '\0';
		if( i == 0) error("missing type name in < ... > clause");
		for( i=1; i<=ntypes; ++i ){
			if( !strcmp( typeset[i], tokname ) ){
				numbval = i;
				return( TYPENAME );
				}
			}
		typeset[numbval = ++ntypes] = cstash( tokname );
		return( TYPENAME );

	case '"':	
	case '\'':
		match = c;
		tokname[0] = ' ';
		i = 1;
		for(;;){
			c = getc(finput);
			if( c == '\n' || c == EOF )
				error("illegal or missing ' or \"" );
			if( c == '\\' ){
				c = getc(finput);
				tokname[i] = '\\';
				if( ++i >= toksize )
					exp_tokname();
				}
			else if( c == match ) break;
			tokname[i] = (char)c;
			if( ++i >= toksize )
				exp_tokname();
			}
		break;

	case '%':
	case '\\':

		switch(c=getc(finput)) {

		case '0':	return(TERM);
		case '<':	return(LEFT);
		case '2':	return(BINARY);
		case '>':	return(RIGHT);
		case '%':
		case '\\':	return(MARK);
		case '=':	return(PREC);
		case '{':	return(LCURLY);
		default:	reserve = 1;
			}

	default:

		if( isdigit(c) ){ /* number */
			numbval = c-'0' ;
			base = (c=='0') ? 8 : 10 ;
			for( c=getc(finput); isdigit(c) ; c=getc(finput) ){
				numbval = numbval*base + c - '0';
				}
			(void) ungetc( c, finput );
			return(NUMBER);
			}
		else if( islower(c) || isupper(c) || c=='_' || c=='.' || c=='$' ){
			i = 0;
			while( islower(c) || isupper(c) || isdigit(c) || c=='_' || c=='.' || c=='$' ){
				tokname[i] = (char)c;
				if( reserve && isupper(c) ) tokname[i] += 'a'-'A';
				if( ++i >= toksize )
					exp_tokname();
				c = getc(finput);
				}
			}
		else return(c);

		(void) ungetc( c, finput );
		}

	tokname[i] = '\0';

	if( reserve ){ /* find a reserved word */
		if( !strcmp(tokname,"term")) return( TERM );
		if( !strcmp(tokname,"token")) return( TERM );
		if( !strcmp(tokname,"left")) return( LEFT );
		if( !strcmp(tokname,"nonassoc")) return( BINARY );
		if( !strcmp(tokname,"binary")) return( BINARY );
		if( !strcmp(tokname,"right")) return( RIGHT );
		if( !strcmp(tokname,"prec")) return( PREC );
		if( !strcmp(tokname,"start")) return( START );
		if( !strcmp(tokname,"type")) return( TYPEDEF );
		if( !strcmp(tokname,"union")) return( UNION );
		error("invalid escape, or illegal reserved word: %s", tokname );
		}

	/* look ahead to distinguish IDENTIFIER from C_IDENTIFIER */

	c = getc(finput);
	while( c==' ' || c=='\t'|| c=='\n' || c=='\f' || c== '/' ) {
		if( c == '\n' ) {
			++peekline;
		} else if( c == '/' ){ /* look for comments */
			peekline += skipcom();
			}
		c = getc(finput);
		}
	if( c == ':' ) return( C_IDENTIFIER );
	(void) ungetc( c, finput );
	return( IDENTIFIER );
}

static
fdtype( t ){ /* determine the type of a symbol */
	register v;
	if( t >= NTBASE ) v = nontrst[t-NTBASE].tvalue;
	else v = TYPE( toklev[t] );
	if( v <= 0 ) error( "must specify type for %s", (t>=NTBASE)?nontrst[t-NTBASE].name:
			tokset[t].name );
	return( v );
	}

static 
chfind( t, s ) register char *s; {
	int i;

	if (s[0]==' ')t=0;
	TLOOP(i){
		if(!strcmp(s,tokset[i].name)){
			return( i );
			}
		}
	NTLOOP(i){
		if(!strcmp(s,nontrst[i].name)) {
			return( i+NTBASE );
			}
		}
	/* cannot find name */
	if( t>1 )
		error( "%s should have been defined earlier", s );
	return( defin( t, s ) );
	}

static void
cpyunion(){
	/* copy the union declaration to the output, and the define file if present */

	int level, c;
	if ( gen_lines )
		(void) fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
	(void) fprintf( ftable, "typedef union\n" );
	if( fdefine ) (void) fprintf( fdefine, "\ntypedef union\n" );
	(void) fprintf( ftable,"#ifdef __cplusplus\n\tYYSTYPE\n#endif\n");
	if(fdefine)(void)fprintf(fdefine,"#ifdef __cplusplus\n\tYYSTYPE\n#endif\n");
	
	level = 0;
	for(;;){
		if( (c=getc(finput)) == EOF ) error( "EOF encountered while processing %%union" );
		(void) putc( (char)c, ftable );
		if( fdefine ) (void) putc( (char)c, fdefine );

		switch( c ){

		case '\n':
			++lineno;
			break;

		case '{':
			++level;
			break;

		case '}':
			--level;
			if( level == 0 ) { /* we are finished copying */
				(void) fprintf( ftable, " YYSTYPE;\n" );
				if( fdefine ) (void) fprintf( fdefine, " YYSTYPE;\nextern YYSTYPE yylval;\n" );
				return;
				}
			}
		}
	}

static void
cpycode(){ /* copies code between \{ and \} */

	int c;
	c = getc(finput);
	if( c == '\n' ) {
		c = getc(finput);
		lineno++;
		}
	if ( gen_lines )
		(void) fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
	while( c != EOF ){
		if( c=='\\' ) {
			if( (c=getc(finput)) == '}' )  return;
			else (void) putc('\\', ftable );
		} else if( c=='%' ) {
			if( (c=getc(finput)) == '}' ) return;
			else (void) putc('%', ftable );
		}
		(void) putc( (char)c , ftable );
		if( c == '\n' ) ++lineno;
		c = getc(finput);
		}
	error("eof before %%}" );
	}

static
skipcom(){ /* skip over comments */
	register c, i=0;  /* i is the number of lines skipped */

	/* skipcom is called after reading a / */

	if( getc(finput) != '*' ) error( "illegal comment" );
	c = getc(finput);
	while( c != EOF ){
		while( c == '*' ){
			if( (c=getc(finput)) == '/' ) return( i );
			}
		if( c == '\n' ) ++i;
		c = getc(finput);
		}
	error( "EOF inside comment" );
	/* NOTREACHED */
	}

static void
cpyact(offset){ /* copy C action to the next ; or closing } */
	int brac, c, match, i, t, j, s, tok, argument, m;

	if ( gen_lines )
		(void) fprintf( faction, "\n# line %d \"%s\"\n", lineno, infile );

	brac = 0;

loop:
	c = getc(finput);
swt:
	switch( c ){

	    case ';':
		if( brac == 0 ){
			(void) putc( (char)c , faction );
			return;
			}
		goto lcopy;

	    case '{':
		brac++;
		goto lcopy;

	    case '$':
		s = 1;
		tok = -1;
		argument = 1;
		while ((c=getc(finput)) == ' ' || c == '\t') /* EMPTY */;
		if( c == '<' ){ /* type description */
			(void) ungetc( c, finput );
			if( gettok() != TYPENAME )
				 error( "bad syntax on $<ident> clause" );
			tok = numbval;
			c = getc(finput);
			}
		if( c == '$' ){
			(void) fprintf( faction, "yyval");
			if( ntypes ){ /* put out the proper tag... */
				if( tok < 0 ) tok = fdtype( *prdptr[nprod] );
				(void) fprintf( faction, ".%s", typeset[tok] );
				}
			goto loop;
			}
		if ( isalpha(c) ) {
			int same = 0;
			(void) ungetc(c, finput);
			if (gettok() != IDENTIFIER) error("bad action format");
			t = chfind(1, tokname);
			while((c=getc(finput))==' ' || c=='\t')/* EMPTY */;
			if (c == '#') {
				while((c=getc(finput))==' ' || c=='\t')/* EMPTY */;
				if ( isdigit(c) ) {
					m = 0;
					while (isdigit(c)) {
						m = m*10+c-'0';
						c = getc(finput);
					}
					argument = m;
				} else error("illegal character \"#\"");
			}
 			if ( argument<1 )
				 error("illegal action argument no.");
			for( i=1; i<=offset; ++i)
				if (prdptr[nprod][i] == t)
					if (++same == argument) {
						(void) fprintf(faction, "yypvt[-%d]",offset-i);
						if(ntypes) {
							if (tok<0)
								tok=fdtype(prdptr[nprod][i]);
							(void) fprintf(faction,".%s",typeset[tok]);
						}
						goto swt;
					}
			error("illegal action arguments");
		}
		if( c == '-' ){
			s = -s;
			c = getc(finput);
			}
		if( isdigit(c) ){
			j=0;
			while( isdigit(c) ){
				j= j*10+c-'0';
				c = getc(finput);
				}

			j = j*s - offset;
			if( j > 0 ){
				error( "Illegal use of $%d", j+offset );
				}

			(void) fprintf( faction, "yypvt[-%d]", -j );
			if( ntypes ){ /* put out the proper tag */
				if( j+offset <= 0 && tok < 0 ) error( "must specify type of $%d", j+offset );
				if( tok < 0 ) tok = fdtype( prdptr[nprod][j+offset] );
				(void) fprintf( faction, ".%s", typeset[tok] );
				}
			goto swt;
			}
		(void) putc( '$' , faction );
		if( s<0 ) (void) putc('-', faction );
		goto swt;

	    case '}':
		if( --brac ) goto lcopy;
		(void) putc( (char)c, faction );
		return;

	    case '/':	/* look for comments */
		(void) putc( (char)c , faction );
		c = getc(finput);
		if( c != '*' ) goto swt;

		/* it really is a comment */

		(void) putc( (char)c , faction );
		c = getc(finput);
		while( c != EOF ){
			while( c=='*' ){
				(void) putc( (char)c , faction );
				if( (c=getc(finput)) == '/' ) goto lcopy;
				}
			(void) putc( (char)c , faction );
			if( c == '\n' )++lineno;
			c = getc(finput);
			}
		error( "EOF inside comment" );
		/* FALLTHRU */

	    case '\'':	/* character constant */
	    case '"':	/* character string */
		match = c;
		(void) putc( (char)c , faction );
		while( (c=getc(finput)) != EOF ){
			if( c=='\\' ){
				(void) putc( (char)c , faction );
				c=getc(finput);
				if( c == '\n' ) ++lineno;
				}
			else if( c==match ) goto lcopy;
			else if( c=='\n' ) error( "newline in string or char. const." );
			(void) putc( (char)c , faction );
			}
		error( "EOF in string or character constant" );
		/* FALLTHRU */

	    case EOF:
		error("action does not terminate" );
		/* FALLTHRU */

	    case '\n':	++lineno;
		goto lcopy;

		}

lcopy:
	(void) putc( (char)c , faction );
	goto loop;
	}

static void
lhsfill( s )	/* new rule, dump old (if exists), restart strings */
	char *s;
{
	static int lhs_len = LHS_TEXT_LEN;
 	int s_lhs = strlen(s);
 	if ( s_lhs >= lhs_len ) {
 		lhs_len = s_lhs + 2;
 		lhstext=(char *)realloc((char *)lhstext, sizeof(char)*lhs_len);
 		if (lhstext == NULL ) error("couldn't expanded LHS length");
 	}
	rhsfill( (char *) 0 );
	(void) strcpy( lhstext, s );	/* don't worry about too long of a name */
}


static void
rhsfill( s )
	char *s;	/* either name or 0 */
{
	static char *loc;	/* next free location in rhstext */
	static int rhs_len = RHS_TEXT_LEN;
 	static int used = 0;
 	int s_rhs = strlen(s);
	register char *p;

	if ( !s )	/* print out and erase old text */
	{
		if ( *lhstext )		/* there was an old rule - dump it */
			lrprnt();
		( loc = rhstext )[0] = '\0';
		return;
	}
	/* add to stuff in rhstext */
	p = s;

 	used = loc - rhstext;
 	if ( (s_rhs + 3) >= (rhs_len - used) ) {
 		static char *textbase;
 		textbase = rhstext;
 		rhs_len += s_rhs + RHS_TEXT_LEN;
 		rhstext=(char *)realloc((char *)rhstext, sizeof(char)*rhs_len);
 		if (rhstext == NULL ) error("couldn't expanded RHS length");
 		loc = loc - textbase + rhstext;
	}

	*loc++ = ' ';
	if ( *s == ' ' )/* special quoted symbol */
	{
		*loc++ = '\'';	/* add first quote */
		p++;
	}
 	while ( *loc = *p++ )
		if ( loc++ > &rhstext[ RHS_TEXT_LEN ] - 3 ) break;

	if ( *s == ' ' )
		*loc++ = '\'';
	*loc = '\0';		/* terminate the string */
}

static void
lrprnt()	/* print out the left and right hand sides */
{
	char *rhs;

	if ( !*rhstext )		/* empty rhs - print usual comment */
		rhs = " /* empty */";
	else
		rhs = rhstext;
	(void) fprintf( fdebug, "	\"%s :%s\",\n", lhstext, rhs );
}


static void
beg_debug()	/* dump initial sequence for fdebug file */
{
	(void) fprintf( fdebug, "typedef struct\n");
	(void) fprintf( fdebug,"#ifdef __cplusplus\n\tyytoktype\n");
	(void) fprintf( fdebug,"#endif\n{ char *t_name; int t_val; } yytoktype;\n");
	(void) fprintf( fdebug,
		"#ifndef YYDEBUG\n#\tdefine YYDEBUG\t%d", gen_testing );
	(void) fprintf( fdebug, "\t/*%sallow debugging */\n#endif\n\n",
		gen_testing ? " " : " don't " );
	(void) fprintf( fdebug, "#if YYDEBUG\n\nyytoktype yytoks[] =\n{\n" );
}


static void
end_toks()	/* finish yytoks array, get ready for yyred's strings */
{
	(void) fprintf( fdebug, "\t\"-unknown-\",\t-1\t/* ends search */\n" );
	(void) fprintf( fdebug, "};\n\nchar * yyreds[] =\n{\n" );
	(void) fprintf( fdebug, "\t\"-no such reduction-\",\n" );
}


static void
end_debug()	/* finish yyred array, close file */
{
	lrprnt();		/* dump last lhs, rhs */
	(void) fprintf( fdebug, "};\n#endif /* YYDEBUG */\n" );
	(void) fclose( fdebug );
}


/* 2/29/88 -
** The normal length for token sizes is NAMESIZE - if a token is
** seen that has a longer length, expand "tokname" by NAMESIZE.
*/
static void
exp_tokname()
{
    toksize += NAMESIZE;
    tokname = (char *) realloc(tokname, sizeof(char) * toksize);
}


/* 2/29/88 -
**
*/
static void
exp_prod()
{
	int i;
	nprodsz += NPROD;

	prdptr = (int **) realloc((char *)prdptr, sizeof(int *) * (nprodsz+2));
	levprd  = (int *)  realloc((char *)levprd, sizeof(int) * (nprodsz+2));
	had_act = (char *) realloc((char *)had_act, sizeof(char) * (nprodsz+2));
	for (i=nprodsz-NPROD; i<nprodsz+2; ++i) had_act[i] = 0;

	if ((*prdptr == NULL) || (levprd == NULL) || (had_act == NULL))
		error("couldn't expand productions");
}

/* 2/29/88 -
** Expand the number of terminals.  Initially there are NTERMS;
** each time space runs out, the size is increased by NTERMS.
** The total size, however, cannot exceed MAXTERMS because of
** the way LOOKSETS (struct looksets) is set up.
** Tables affected:
**	tokset, toklev : increased to ntoksz
**
**	tables with initial dimensions of TEMPSIZE must be changed if
**	(ntoksz + NNONTERM) >= TEMPSIZE : temp1[]
*/
static void
exp_ntok()
{
	ntoksz += NTERMS;

	tokset = (TOKSYMB *) realloc((char *)tokset, sizeof(TOKSYMB) * ntoksz);
	toklev = (int *) realloc((char *)toklev, sizeof(int) * ntoksz);

	if ((tokset == NULL) || (toklev == NULL))
		error("couldn't expand NTERMS");
}


static void
exp_nonterm()
{
	nnontersz += NNONTERM;

	nontrst = (NTSYMB *) 
		realloc((char *)nontrst, sizeof(TOKSYMB) * nnontersz);
	if (nontrst == NULL) error("couldn't expand NNONTERM");
}

void exp_mem( flag )
int flag;
{
	int i;
	static int *membase;
	new_memsize += MEMSIZE;

	membase = tracemem;
	tracemem = (int *) realloc((char *)tracemem, sizeof(int) * new_memsize);
	if (tracemem == NULL) error("couldn't expand mem table");
	if ( flag ) {
		for ( i=0; i<=nprod; ++i)
			prdptr[i] = prdptr[i] - membase + tracemem;
		mem = mem - membase + tracemem;
	} else {
 		size += MEMSIZE;
 		temp1 = (int *)realloc((char *)temp1, sizeof(int)*size);
		optimmem = optimmem - membase + tracemem;
	}
}
