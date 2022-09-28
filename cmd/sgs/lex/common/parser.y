/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)lex:parser.y	1.20.1.1"
%}
/* parser.y */
%token CHAR CCL NCCL STR DELIM SCON ITER NEWE NULLS
%left SCON '/' NEWE
%left '|'
%left '$' '^'
%left CHAR CCL NCCL '(' '.' STR NULLS
%left ITER
%left CAT
%left '*' '+' '?'

%{
#define YYSTYPE union _yystype_
union _yystype_
{
	int	i;
	unsigned char	*cp;
};
# include "ldefs.c"
%}
%%
%{
int i;
int j,k;
int g;
char *p;
%}
acc	:	lexinput
	={	
# ifdef DEBUG
		if(debug) sect2dump();
# endif
	}
	;
lexinput:	defns delim prods end
	|	defns delim end
	={
		if(!funcflag)phead2();
		funcflag = TRUE;
	}
	| error
	={
# ifdef DEBUG
		if(debug) {
			sect1dump();
			sect2dump();
			}
# endif
		fatal = 0;
		n_error++;
		error("Illegal definition");
		fatal = 1;
		}
	;
end:		delim | ;
defns:	defns STR STR
	={	scopy($2,dp);
		def[dptr] = dp;
		dp += slength($2) + 1;
		scopy($3,dp);
		subs[dptr++] = dp;
		if(dptr >= DEFSIZE)
			error("Too many definitions");
		dp += slength($3) + 1;
		if(dp >= dchar+DEFCHAR)
			error("Definitions too long");
		subs[dptr]=def[dptr]=0;	/* for lookup - require ending null */
	}
	|
	;
delim:	DELIM
	={
# ifdef DEBUG
		if(sect == DEFSECTION && debug) sect1dump();
# endif
		sect++;
		}
	;
prods:	prods pr
	={	$$.i = mn2(RNEWE,$1.i,$2.i);
		}
	|	pr
	={	$$.i = $1.i;}
	;
pr:	r NEWE
	={
		if(divflg == TRUE)
			i = mn1(S1FINAL,casecount);
		else i = mn1(FINAL,casecount);
		$$.i = mn2(RCAT,$1.i,i);
		divflg = FALSE;
		casecount++;
		}
	| error NEWE
	={
# ifdef DEBUG
		if(debug) sect2dump();
# endif
		fatal = 0;
		yyline--;
		n_error++;
		error("Illegal rule");
		fatal = 1;
		yyline++;
		}
r:	CHAR
	={	$$.i = mn0($1.i); }
	| STR
	={
		p = (char *)$1.cp;
		i = mn0((unsigned)(*p++));
		while(*p)
			i = mn2(RSTR,i,(unsigned)(*p++));
		$$.i = i;
		}
	| '.'
	={	symbol['\n'] = 0;
		if(psave == FALSE){
			p = ccptr;
			psave = ccptr;
			for(i=1;i<'\n';i++){
				symbol[i] = 1;
				*ccptr++ = i;
				}
			for(i='\n'+1;i<NCH;i++){
				symbol[i] = 1;
				*ccptr++ = i;
				}
			*ccptr++ = 0;
			if(ccptr > ccl+CCLSIZE)
				error("Too many large character classes");
			}
		else
			p = psave;
		$$.i = mn1(RCCL,p);
		cclinter(1);
		}
	| CCL
	={	$$.i = mn1(RCCL,$1.i); }
	| NCCL
	={	$$.i = mn1(RNCCL,$1.i); }
	| r '*'
	={	$$.i = mn1(STAR,$1.i); }
	| r '+'
	={	$$.i = mn1(PLUS,$1.i); }
	| r '?'
	={	$$.i = mn1(QUEST,$1.i); }
	| r '|' r
	={	$$.i = mn2(BAR,$1.i,$3.i); }
	| r r %prec CAT
	={	$$.i = mn2(RCAT,$1.i,$2.i); }
	| r '/' r
	={	if(!divflg){
			j = mn1(S2FINAL,-casecount);
			i = mn2(RCAT,$1.i,j);
			$$.i = mn2(DIV,i,$3.i);
			}
		else {
			$$.i = mn2(RCAT,$1.i,$3.i);
			error("illegal extra slash");
			}
		divflg = TRUE;
		}
	| r ITER ',' ITER '}'
	={	if($2.i > $4.i){
			i = $2.i;
			$2.i = $4.i;
			$4.i = i;
			}
		if($4.i <= 0)
			error("iteration range must be positive");
		else {
			j = $1.i;
			for(k = 2; k<=$2.i;k++)
				j = mn2(RCAT,j,dupl($1.i));
			for(i = $2.i+1; i<=$4.i; i++){
				g = dupl($1.i);
				for(k=2;k<=i;k++)
					g = mn2(RCAT,g,dupl($1.i));
				j = mn2(BAR,j,g);
				}
			$$.i = j;
			}
	}
	| r ITER '}'
	={
		if($2.i < 0)error("can't have negative iteration");
		else if($2.i == 0) $$.i = mn0(RNULLS);
		else {
			j = $1.i;
			for(k=2;k<=$2.i;k++)
				j = mn2(RCAT,j,dupl($1.i));
			$$.i = j;
			}
		}
	| r ITER ',' '}'
	={
				/* from n to infinity */
		if($2.i < 0)error("can't have negative iteration");
		else if($2.i == 0) $$.i = mn1(STAR,$1.i);
		else if($2.i == 1)$$.i = mn1(PLUS,$1.i);
		else {		/* >= 2 iterations minimum */
			j = $1.i;
			for(k=2;k<$2.i;k++)
				j = mn2(RCAT,j,dupl($1.i));
			k = mn1(PLUS,dupl($1.i));
			$$.i = mn2(RCAT,j,k);
			}
		}
	| SCON r
	={	$$.i = mn2(RSCON,$2.i,$1.i); }
	| '^' r
	={	$$.i = mn1(CARAT,$2.i); }
	| r '$'
	={	i = mn0('\n');
		if(!divflg){
			j = mn1(S2FINAL,-casecount);
			k = mn2(RCAT,$1.i,j);
			$$.i = mn2(DIV,k,i);
			}
		else $$.i = mn2(RCAT,$1.i,i);
		divflg = TRUE;
		}
	| '(' r ')'
	={	$$.i = $2.i; }
	|	NULLS
	={	$$.i = mn0(RNULLS); }
	;
%%
#if defined(__STDC__) || defined(__cplusplus)
int yylex(void)
#else
yylex()
#endif
{
	register char *p;
	register int c, i;
	char  *t, *xp;
	int n, j, k, x;
	static int sectbegin;
	static char token[TOKENSIZE];
	static int iter;

# ifdef DEBUG
	yylval.i = 0;
# endif

	if(sect == DEFSECTION) {		/* definitions section */
		while(!eof) {
			if(prev == '\n'){    /* next char is at beginning of line */
				(void)getl(p=buf);
				switch(*p){
				case '%':
					switch(c= *(p+1)){
					case '%':
						if(scomp(p,"%%")) {
							p++;
							while(*(++p))
								if(!space(*p)) {
									warning("invalid string following %%%% be ignored");
									break;
								}
						}
						lgate();
						if(!ratfor)(void) fprintf(fout,"# ");
						(void) fprintf(fout,"define YYNEWLINE %d\n",ctable['\n']);
						if(!ratfor)(void) fprintf(fout,"yylex(){\nint nstr; extern int yyprevious;\n");
						sectbegin = TRUE;
						i = treesize*(sizeof(*name)+sizeof(*left)+
							sizeof(*right)+sizeof(*nullstr)+sizeof(*parent))+ALITTLEEXTRA;
						c = (int)myalloc(i,1);
						if(c == 0)
							error("Too little core for parse tree");
						p = (char *)c;
						cfree((char *)p,i,1);
						name = (int *)myalloc(treesize,sizeof(*name));
						left = (int *)myalloc(treesize,sizeof(*left));
						right = (int *)myalloc(treesize,sizeof(*right));
						nullstr = myalloc(treesize,sizeof(*nullstr));
						parent = (int *)myalloc(treesize,sizeof(*parent));
						if(name == 0 || left == 0 || right == 0 || parent == 0 || nullstr == 0)
							error("Too little core for parse tree");
						return(freturn(DELIM));
					case 'p': case 'P':	/* has overridden number of positions */
						p += 2;
						maxpos = siconv(p);
						if (maxpos<=0)error("illegal position number");
# ifdef DEBUG
						if (debug) (void) printf("positions (%%p) now %d\n",maxpos);
# endif
						if(report == 2)report = 1;
						continue;
					case 'n': case 'N':	/* has overridden number of states */
						p += 2;
						nstates = siconv(p);
						if(nstates<=0)error("illegal state number");
# ifdef DEBUG
						if(debug)(void) printf( " no. states (%%n) now %d\n",nstates);
# endif
						if(report == 2)report = 1;
						continue;
					case 'e': case 'E':		/* has overridden number of tree nodes */
						p += 2;
						treesize = siconv(p);
						if(treesize<=0)error("illegal number of parse tree nodes");
# ifdef DEBUG
						if (debug) (void) printf("treesize (%%e) now %d\n",treesize);
# endif
						if(report == 2)report = 1;
						continue;
					case 'o': case 'O':
						p += 2;
						outsize = siconv(p);
						if(outsize<=0)error("illegal size of output array");
						if (report ==2) report=1;
						continue;
					case 'a': case 'A':		/* has overridden number of transitions */
						p += 2;
						ntrans = siconv(p);
						if(ntrans<=0)error("illegal translation number");
# ifdef DEBUG
						if (debug)(void) printf("N. trans (%%a) now %d\n",ntrans);
# endif
						if(report == 2)report = 1;
						continue;
					case 'k': case 'K': /* overriden packed char classes */
						p += 2;
						cfree((char *)pchar, pchlen, sizeof(*pchar));
						pchlen = siconv(p);
						if(pchlen<=0)error("illegal number of packed character class");
# ifdef DEBUG
						if (debug) (void) printf( "Size classes (%%k) now %d\n",pchlen);
# endif
						pchar=pcptr=(unsigned char *)myalloc(pchlen, sizeof(*pchar));
						if (report==2) report=1;
						continue;
					case 't': case 'T': 	/* character set specifier */
						ZCH = atoi(p+2);
						if (ZCH < NCH) ZCH = NCH;
						if (ZCH > 2*NCH) error("ch table needs redeclaration");
						chset = TRUE;
						for(i = 0; i<ZCH; i++)
							ctable[i] = 0;
						while(getl(p) && scomp(p,"%T") != 0 && scomp(p,"%t") != 0){
							if((n = siconv(p)) <= 0 || n > ZCH){
								error("Character value %d out of range",n);
								continue;
								}
							while(digit(*p)) p++;
							if(!space(*p)) error("bad translation format");
							while(space(*p)) p++;
							t = p;
							while(*t){
								c = ctrans(&t);
								if(ctable[(unsigned)c]){
									if (printable(c))
										error("Character '%c' used twice",c);
									else
										error("Character %o used twice",c);
									}
								else ctable[(unsigned)c] = n;
								t++;
								}
							p = buf;
							}
						{
						char chused[2*NCH]; int kr;
						for(i=0; i<ZCH; i++)
							chused[i]=0;
						for(i=0; i<NCH; i++)
							chused[ctable[i]]=1;
						for(kr=i=1; i<NCH; i++)
							if (ctable[i]==0)
								{
								while (chused[kr] == 0)
									kr++;
								ctable[i]=kr;
								chused[kr]=1;
								}
						}
						lgate();
						continue;
					case 'r': case 'R':
						c = 'r';
						/* FALLTHRU */
					case 'c': case 'C':
						if(lgatflg)
							error("Too late for language specifier");
						ratfor = (c == 'r');
						continue;
					case '{':
						lgate();
						while(getl(p) && scomp(p,"%}") != 0)
							if(p[0]=='/' && p[1]=='*')
								cpycom(p);
							else
								(void) fprintf(fout,"%s\n",p);
						if(p[0] == '%') continue;
						if (*p) error("EOF before %%%%");
						else error("EOF before %%}");
						/* NOTREACHED */
						break;
					case 's': case 'S':		/* start conditions */
						lgate();
						while(*p && index(*p," \t,") < 0) p++;
						n = TRUE;
						while(n){
							while(*p && index(*p," \t,") >= 0) p++;
							t = p;
							while(*p && index(*p," \t,") < 0)p++;
							if(!*p) n = FALSE;
							*p++ = 0;
							if (*t == 0) continue;
							i = sptr*2;
							if(!ratfor)(void) fprintf(fout,"# ");
							(void) fprintf(fout,"define %s %d\n",t,i);
							scopy(t,sp);
							sname[sptr++] = sp;
							sname[sptr] = 0;	/* required by lookup */
							if(sptr >= STARTSIZE)
								error("Too many start conditions");
							sp += slength(sp) + 1;
							if(sp >= schar+STARTCHAR)
								error("Start conditions too long");
							}
						continue;
					default:
						error("Invalid request %s",p);
						continue;
						}	/* end of switch after seeing '%' */
					break;
				case ' ': case '\t':		/* must be code */
					lgate();
					if( p[1]=='/' && p[2]=='*' ) cpycom(p);
					else (void) fprintf(fout, "%s\n",p);
					continue;
				case '/':	/* look for comments */
					lgate();
					if((*(p+1))=='*') cpycom(p);
					/* FALLTHRU */
				default:		/* definition */
					while(*p && !space(*p)) p++;
					if(*p == 0)
						continue;
					prev = *p;
					*p = 0;
					bptr = p+1;
					yylval.cp = (unsigned char *)buf;
					if(digit(buf[0]))
						warning("Substitution strings may not begin with digits");
					return(freturn(STR));
				}
			} else { /* still sect 1, but prev != '\n' */
				p = bptr;
				while(*p && space(*p)) p++;
				if(*p == 0)
					warning("No translation given - null string assumed");
				scopy(p,token);
				yylval.cp = (unsigned char *)token;
				prev = '\n';
				return(freturn(STR));
				}
			}
		error("unexpected EOF before %%%%");
		/* end of section one processing */
	} else if(sect == RULESECTION){		/* rules and actions */
		while(!eof){
			static int first_test=TRUE, first_value;
			static int reverse=FALSE;
			switch(c=gch()){
			case '\0':
				if(n_error)error_tail();
				return(freturn(0));
			case '\n':
				if(prev == '\n') continue;
				x = NEWE;
				break;
			case ' ':
			case '\t':
				if(prev == '\n') copy_line = TRUE;
				if(sectbegin == TRUE){
					(void)cpyact();
					copy_line = FALSE;
					while((c=gch()) && c != '\n');
					continue;
					}
				if(!funcflag)phead2();
				funcflag = TRUE;
				if(ratfor)(void) fprintf(fout,"%d\n",30000+casecount);
				else (void) fprintf(fout,"case %d:\n",casecount);
				if(cpyact()){
					if(ratfor)(void) fprintf(fout,"goto 30997\n");
					else (void) fprintf(fout,"break;\n");
					}
				while((c=gch()) && c != '\n') {
					if (c=='/') {
						if((c=gch())=='*') {
							c=gch();
							while(c !=EOF) {
								while (c=='*')
									if ((c=gch()) == '/') goto w_loop;
								c = gch();
							}
							error("EOF inside comment");
						} else
							warning("undefined string");
					} else if (c=='}')
						error("illegal extra \"}\"");
				w_loop: ;
				}
				/* while ((c=gch())== ' ' || c == '\t') ; */
				/* if (!space(c)) error("undefined action string"); */
				if(peek == ' ' || peek == '\t' || sectbegin == TRUE){
					fatal = 0;
					n_error++;
					error("executable statements should occur right after %%%%");
					fatal = 1;
					continue;
					}
				x = NEWE;
				break;
			case '%':
				if(prev != '\n') goto character;
				if(peek == '{'){	/* included code */
					(void)getl(buf);
					while(!eof&& getl(buf) && scomp("%}",buf)!=0)
						if(buf[0]=='/' && buf[1]=='*')
							cpycom(buf);
						else
							(void) fprintf(fout,"%s\n",buf);
					continue;
					}
				if(peek == '%'){
					c = gch();
					c = gch();
					x = DELIM;
					break;
					}
				goto character;
			case '|':
				if(peek == ' ' || peek == '\t' || peek == '\n'){
					if(ratfor)(void) fprintf(fout,"%d\n",30000+casecount++);
					else (void) fprintf(fout,"case %d:\n",casecount++);
					continue;
					}
				x = '|';
				break;
			case '$':
				if(peek == '\n' || peek == ' ' || peek == '\t' || peek == '|' || peek == '/'){
					x = c;
					break;
					}
				goto character;
			case '^':
				if(prev != '\n' && scon != TRUE) goto character;
				/* valid only at line begin */
				x = c;
				break;
			case '?':
			case '+':
			case '*':
				if(prev == '\n' ) {
					fatal = 0;
					n_error++;
					error("illegal operator -- %c",c);
					fatal = 1;
				}
				/* FALLTHRU */
			case '.':
			case '(':
			case ')':
			case ',':
			case '/':
				x = c;
				break;
			case '}':
				iter = FALSE;
				x = c;
				break;
			case '{':	/* either iteration or definition */
				if(digit(c=gch())){	/* iteration */
					iter = TRUE;
					if(prev=='{') first_test = TRUE;
				ieval:
					i = 0;
					while(digit(c)){
						token[i++] = c;
						c = gch();
						}
					token[i] = 0;
					yylval.i = siconv(token);
					if(first_test) {
						first_test = FALSE;
						first_value = yylval.i;
					} else
						if(first_value>yylval.i)warning("the values between braces are reversed");
					munput('c',c);
					x = ITER;
					break;
					}
				else {		/* definition */
					i = 0;
					while(c && c!='}'){
						token[i++] = c;
						if(i >= TOKENSIZE)
							error("definition too long");
						c = gch();
						}
					token[i] = 0;
					i = lookup(token,def);
					if(i < 0)
						error("definition %s not found",token);
					else
						munput('s',(unsigned char *)(subs[i]));
					continue;
					}
			case '<':		/* start condition ? */
				if(prev != '\n')  /* not at line begin, not start */
					goto character;
				t = slptr;
				do {
					i = 0;
					c = gch();
					while(c != ',' && c && c != '>'){
						token[i++] = c;
						if(i >= TOKENSIZE)
							error("string name too long");
						c = gch();
						}
					token[i] = 0;
					if(i == 0)
						goto character;
					i = lookup(token,sname);
					if(i < 0) {
						fatal = 0;
						n_error++;
						error("undefined start condition %s",token);
						fatal = 1;
						continue;
						}
					*slptr++ = i+1;
					} while(c && c != '>');
				*slptr++ = 0;
				/* check if previous value re-usable */
				for (xp=slist; xp<t; )
					{
					if (strcmp(xp, t)==0)
						break;
					while (*xp++);
					}
				if (xp<t)
					{
					/* re-use previous pointer to string */
					slptr=t;
					t=xp;
					}
				if(slptr > slist+STARTSIZE)	/* note not packed */
					error("Too many start conditions used");
				yylval.cp = (unsigned char *)t;
				x = SCON;
				break;
			case '"':
				i = 0;
				while((c=gch()) && c != '"' && c != '\n'){
					if(c == '\\') c = usescape(c=gch());
					token[i++] = c;
					if(i >= TOKENSIZE){
						warning("String too long");
						i = TOKENSIZE-1;
						break;
						}
					}
				if(c == '\n') {
					yyline--;
					warning("Non-terminated string");
					yyline++;
					}
				token[i] = 0;
				if(i == 0)x = NULLS;
				else if(i == 1){
					yylval.i = (unsigned)token[0];
					x = CHAR;
					}
				else {
					yylval.cp = (unsigned char *)token;
					x = STR;
					}
				break;
			case '[':
				for(i=1;i<NCH;i++) symbol[i] = 0;
				reverse = FALSE;
				x = CCL;
				if((c = gch()) == '^'){
					x = NCCL;
					reverse = TRUE;
					c = gch();
					}
				while(c != ']' && c){
					static int light=TRUE, ESCAPE=FALSE;
					if(c == '-' && prev == '^' && reverse){
						symbol[(unsigned)c] = 1;
						c = gch();
						continue;
					}
					if(c == '\\') {
						c = usescape(c=gch());
						ESCAPE = TRUE;
					}
					if(c=='-' && !ESCAPE && prev!='[' && peek!=']'){
					/* range specified */
						if (light) {
							c = gch();
							if(c == '\\') 
								c=usescape(c=gch());
							k = c;
							if((unsigned)j > (unsigned)k) {
								n = j;
								j = k;
								k = n;
								}
							if(!(('A'<=j && k<='Z') ||
						     	     ('a'<=j && k<='z') ||
						     	     ('0'<=j && k<='9')))
								warning("Non-portable Character Class");
							/* implementation dependent */
							for(n=(unsigned)j+1;n<=(unsigned)k;n++)
								symbol[n] = 1;
							light = FALSE;
						} else {
							error("unmatched hyphen");
							if(symbol[(unsigned)c])warning("\"%c\" redefined inside brackets",c);
							else symbol[(unsigned)c] = 1;
						}
						ESCAPE = FALSE;
					} else {
						j = c;
						if(symbol[(unsigned)c])warning("\"%c\" redefined inside brackets",c);
						else symbol[(unsigned)c] = 1;
						light = TRUE;
						ESCAPE = FALSE;
					}
					c = gch();
				}
				/* try to pack ccl's */
				i = 0;
				for(j=0;j<NCH;j++)
					if(symbol[j])token[i++] = j;
				token[i] = 0;
				p = ccptr;
				if(optim){
					p = ccl;
					while(p <ccptr && scomp(token,p) != 0)p++;
					}
				if(p < ccptr)	/* found it */
					yylval.cp = (unsigned char *)p;
				else {
					yylval.cp = (unsigned char *)ccptr;
					scopy(token,ccptr);
					ccptr += slength(token) + 1;
					if(ccptr >= ccl+CCLSIZE)
						error("Too many large character classes");
					}
				cclinter(x==CCL);
				break;
			case '\\':
				c = usescape(c=gch());
			default:
			character:
				if(iter){	/* second part of an iteration */
					iter = FALSE;
					if('0' <= c && c <= '9')
						goto ieval;
					}
				if(alpha(peek)){
					i = 0;
					yylval.cp = (unsigned char *)token;
					token[i++] = c;
					while(alpha(peek)) {
						token[i++] = gch();
						if(i >= TOKENSIZE) {
							warning("string too long");
							i = TOKENSIZE - 1;
							break;
							}
						}
					if(peek == '?' || peek == '*' || peek == '+')
						munput('c',(unsigned)(token[--i]));
					token[i] = 0;
					if(i == 1){
						yylval.i = (unsigned)(token[0]);
						x = CHAR;
						}
					else x = STR;
					}
				else {
					yylval.i = (unsigned)c;
					x = CHAR;
					}
				}
			scon = FALSE;
			if(x == SCON)scon = TRUE;
			sectbegin = FALSE;
			return(freturn(x));
			/* NOTREACHED */
			}
		}
	/* section three */
	ptail();
# ifdef DEBUG
	if(debug)
		(void) fprintf(fout,"\n/*this comes from section three - debug */\n");
# endif
	while(getl(buf) && !eof)
		(void) fprintf(fout,"%s\n",buf);
	return(freturn(0));
	}
/* end of yylex */
# ifdef DEBUG
freturn(i)
  int i; {
	if(yydebug) {
		(void) printf("now return ");
		if((unsigned)i < NCH) allprint(i);
		else (void) printf("%d",i);
		(void) printf("   yylval = ");
		switch(i){
			case STR: case CCL: case NCCL:
				strpt(yylval.cp);
				break;
			case CHAR:
				allprint(yylval.i);
				break;
			default:
				(void) printf("%d",yylval.i);
				break;
			}
		(void) putchar('\n');
		}
	return(i);
	}
# endif
