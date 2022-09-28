/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lex:sub1.c	1.14"
/* sub1.c */
# include "ldefs.c"
char *
getl(p)	/* return next line of input, throw away trailing '\n' */
	/* returns 0 if eof is had immediately */
  char *p;
	{
	register int c;
	register char *s, *t;
	t = s = p;
	while(((c = gch()) != 0) && c != '\n') {
		if(t >= &p[BUF_SIZ] ) error("definitions too long");
		*t++ = c;
	}
	*t = 0;
	if(c == 0 && s == t) return((char *)0);
	prev = '\n';
	pres = '\n';
	return(s);
	}
space(ch)
	{
	switch(ch)
		{
		case ' ':
		case '\t':
		case '\n':
			return(1);
		}
	return(0);
	}

digit(c)
{
	return(c>='0' && c <= '9');
}
/* VARARGS1 */
error(s,p,d)
char *s;
	{
	/* if(!eof) */
		if ( !yyline ) (void)fprintf(errorf,"Command line: ");
		else {
			(void)fprintf(errorf,!no_input?"":"\"%s\":",sargv[optind]);
			(void)fprintf(errorf,"line %d: ",yyline);
		}
	(void)fprintf(errorf,"Error: ");
	(void)fprintf(errorf,s,p,d);
	(void) putc('\n',errorf);
	if (fatal) error_tail();
}

error_tail() {

# ifdef DEBUG
		if(debug && sect != ENDSECTION) {
			sect1dump();
			sect2dump();
		}
# endif
		if(
# ifdef DEBUG
			debug ||
# endif
			report == 1) statistics();
		exit(1);	/* error return code */
		/* NOTREACHED */
}

/* VARARGS1 */
warning(s,p,d)
char *s;
	{
	if(!eof)
		if ( !yyline ) (void)fprintf(errorf,"Command line: ");
		else {
			(void)fprintf(errorf,!no_input?"":"\"%s\":",sargv[optind]);
			(void)fprintf(errorf,"line %d: ",yyline);
		}
	(void)fprintf(errorf,"Warning: ");
	(void)fprintf(errorf,s,p,d);
	(void) putc('\n',errorf);
	(void) fflush(errorf);
	if(fout)(void) fflush(fout);
	(void) fflush(stdout);
	}
index(a,s)
	char *s;
{
	register int k;
	for(k=0; s[k]; k++)
		if (s[k]== a)
			return(k);
	return(-1);
	}

alpha(c)
  int c; {
# ifdef ASCII
return('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z');
# endif
# ifdef EBCDIC
return(index(c,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") >= 0);
# endif
}
printable(c)
{
# ifdef ASCII
return( c>040 && c < 0177);
# endif
# ifdef EBCDIC
return(index(c, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,;:><+*)('&%!-=\"")>=0);
# endif
}
lgate()
{
	char fname[20];
	if (lgatflg) return;
	lgatflg=1;
	if(fout == NULL){
		(void) sprintf(fname, "lex.yy.%c", ratfor ? 'r' : 'c' );
		fout = fopen(fname, "w");
		}
	if(fout == NULL) error("Can't open %s",fname);
	if(ratfor) (void)fprintf( fout, "#\n");
	phead1();
	}
/* scopy(ptr to str, ptr to str) - copy first arg str to second */
/* returns ptr to second arg */
scopy(s,t)
  char *s, *t; {
	register char *i;
	i = t;
	while(*i++ = *s++);
	return;
	}
siconv(t)	/* convert string t, return integer value */
  char *t; {
	register int i,sw;
	register char *s;
	s = t;
	while(space(*s)) s++;
	if(!digit(*s) && *s != '-') error("missing translation value");
	sw = 0;
	if(*s == '-'){	/* neg */
		sw = 1;
		s++;
		}
	if(!digit(*s)) error("incomplete translation format");
	i = 0;
	while('0' <= *s && *s <= '9')
		i = i * 10 + (*(s++)-'0');
	return(sw ? -i : i);
	}
/* slength(ptr to str) - return integer length of string arg */
/* excludes '\0' terminator */
slength(s)
  char *s; {
	register int n;
	register char *t;
	t = s;
	for (n = 0; *t++; n++);
	return(n);
	}
/* scomp(x,y) - return -1 if x < y,
		0 if x == y,
		return 1 if x > y, all lexicographically */
scomp(x,y)
  char *x,*y; {
	register unsigned char *a,*d;
	a = (unsigned char *)x;
	d = (unsigned char *)y;
	while(*a || *d){
		if(*a > *d)
			return(1);	/* greater */
		if(*a < *d)
			return(-1);	/* less */
		a++;
		d++;
		}
	return(0);	/* equal */
	}
ctrans(ss)
	char **ss;
{
	register int c, k;
	if ((c = **ss) != '\\')
		return(c);
	switch(c= *++*ss)
	{
	case 'a': c = '\a'; warning("\\a is ANSI C \"alert\" character"); break;
	case 'v': c = '\v'; break;
	case 'n': c = '\n'; break;
	case 't': c = '\t'; break;
	case 'r': c = '\r'; break;
	case 'b': c = '\b'; break;
	case 'f': c = 014; break;		/* form feed for ascii */
	case '\\': c = '\\'; break;
	case 'x': {
		int dd;
		warning("\\x is ANSI C hex escape"); 
		if (   digit((dd = *++*ss))
		    || ('a' <= dd && dd <= 'f')
		    || ('A' <= dd && dd <= 'F') ) {
			c = 0;
			while( digit(dd)
				|| ('A' <= dd && dd <= 'F')
				|| ('a' <= dd && dd <= 'f') ) {
				if (digit(dd))
					c = c*16 + dd - '0';
				else if ( dd >= 'a' )
					c = c*16 + 10 + dd - 'a';
				else
					c = c*16 + 10 + dd - 'A';
				dd = *++*ss;
			}
		} else c = 'x';
		break;
		}
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		c -= '0';
		while ((k = *(*ss+1)) >= '0' && k <= '7')
			{
			c = c*8 + k - '0';
			(*ss)++;
			}
		break;
	}
	return(c);
}
cclinter(sw)
  int sw; {
		/* sw = 1 ==> ccl */
	register int i, j, k;
	int m;
	if(!sw){		/* is NCCL */
		for(i=1;i<NCH;i++)
			symbol[i] ^= 1;			/* reverse value */
		}
	for(i=1;i<NCH;i++)
		if(symbol[i]) break;
	if(i >= NCH) return;
	i = cindex[i];
	/* see if ccl is already in our table */
	j = 0;
	if(i){
		for(j=1;j<NCH;j++){
			if((symbol[j] && cindex[j] != i) ||
			   (!symbol[j] && cindex[j] == i)) break;
			}
		}
	if(j >= NCH) return;		/* already in */
	m = 0;
	k = 0;
	for(i=1;i<NCH;i++)
		if(symbol[i]){
			if(!cindex[i]){
				cindex[i] = ccount;
				symbol[i] = 0;
				m = 1;
				}
			else k = 1;
			}
			/* m == 1 implies last value of ccount has been used */
	if(m)ccount++;
	if(k == 0) return;	/* is now in as ccount wholly */
	/* intersection must be computed */
	for(i=1;i<NCH;i++){
		if(symbol[i]){
			m = 0;
			j = cindex[i];	/* will be non-zero */
			for(k=1;k<NCH;k++){
				if(cindex[k] == j){
					if(symbol[k]) symbol[k] = 0;
					else {
						cindex[k] = ccount;
						m = 1;
						}
					}
				}
			if(m)ccount++;
			}
		}
	return;
	}
usescape(c)
  int c; {
	register char d;
	switch(c){
	case 'a': c = '\a'; warning("\\a is ANSI C \"alert\" character"); break;
	case 'v': c = '\v'; break;
	case 'n': c = '\n'; break;
	case 'r': c = '\r'; break;
	case 't': c = '\t'; break;
	case 'b': c = '\b'; break;
	case 'f': c = 014; break;		/* form feed for ascii */
	case 'x': {
		int dd;
		warning("\\x is ANSI C hex escape"); 
		if (   digit((dd=gch()))
		    || ('A' <= dd && dd <= 'F')
		    || ('a' <= dd && dd <= 'f') ) {
			c = 0;
			while( digit(dd)
				|| ('A' <= dd && dd <= 'F')
				|| ('a' <= dd && dd <= 'f') ) {
				if ( digit(dd) )
					c = c*16 + dd - '0';
				else if ( dd >= 'a' )
					c = c*16 + 10 + dd - 'a';
				else
					c = c*16 + 10 + dd - 'A';
				if ( !digit(peek)
					&& !('A' <= peek && peek <= 'F')
					&& !('a' <= peek && peek <= 'f') )
					break;
				dd=gch();
			}
		} else c = 'x';
		break;
		}
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		c -= '0';
		while('0' <= (d=gch()) && d <= '7'){
			c = c * 8 + (d-'0');
			if(!('0' <= peek && peek <= '7')) break;
			}
		break;
	}
	return(c);
	}
lookup(s,t)
  char *s;
  char **t; {
	register int i;
	i = 0;
	while(*t){
		if(scomp(s,*t) == 0)
			return(i);
		i++;
		t++;
		}
	return(-1);
	}
cpycom(p) char *p;{
	static char *t;
	static int c;
	t = p;

	(void)fprintf(fout, "\n# line %d \"%s\"\n", yyline, sargv[optind]);

	(void) putc(*t++,fout);
	(void) putc(*t++,fout);
	while(*t) {
		if(*t == '*') {
			(void) putc(*t++,fout);
			if (*t == '/') goto backcall;
		}
		else	(void) putc(*t++,fout);
	}
	(void) putc('\n',fout);
	while( c=gch() ){
		while( c=='*' ){
			(void) putc((char)c,fout);
			if( (c=gch()) == '/' ) {
				while((c=gch()) == ' ' || c == '\t') ;
				if(!space(c))error("unacceptable statement");
				prev = '\n';
				goto backcall;
			}
		}
		(void) putc((char)c,fout);
	}
	error( "unexpected EOF inside comment" );
backcall:
	(void) putc('/',fout);
	(void) putc('\n',fout);
}

int
cpyact(){ /* copy C action to the next ; or closing } */
	register int brac, c, mth;
	static int sw, savline;

	brac = 0;
	sw = TRUE;
	savline = yyline;

	(void)fprintf(fout, "\n# line %d \"%s\"\n", yyline, sargv[optind]);

	while(!eof){
		c = gch();
	swt:
		switch( c ){
	
		case '|':
			if(brac == 0 && sw == TRUE){
				if(peek == '|')(void)gch();	/* eat up an extra '|' */
				return(0);
				}
			break;
	
		case ';':
			if( brac == 0 ){
				(void) putc((char)c,fout);
				(void) putc('\n',fout);
				return(1);
				}
			break;
	
		case '{':
			brac++;
			savline=yyline;
			break;
	
		case '}':
			brac--;
			if( brac == 0 ){
				(void) putc((char)c,fout);
				(void) putc('\n',fout);
				return(1);
				}
			break;
	
		case '/':
			(void) putc((char)c,fout);
			c = gch();
			if( c != '*' ) goto swt;
			(void) putc((char)c,fout);
			savline=yyline;
			while( c=gch() ){
				while( c=='*' ){
					(void) putc((char)c,fout);
					if( (c=gch()) == '/' ) {
						(void) putc('/',fout);
						while((c=gch()) == ' ' || c == '\t')
							(void) putc((char)c,fout);
						if ( c == '\n' && brac == 0) {
							munput('c',c);
							(void) putc('\n',fout);
							return(1);
						} else goto swt;
					}
				}
				(void) putc((char)c,fout);
			}
			yyline=savline;
			error( "EOF inside comment" );
			/* NOTREACHED */
			break;
	
		case '\'':	/* character constant */
		case '"':	/* character string */
			mth = c;
			(void) putc((char)c,fout);
			while( c=gch() ){
				if( c=='\\' ){
					(void) putc((char)c,fout);
					c=gch();
					}
				else if( c==mth ) goto loop;
				(void) putc((char)c,fout);
				if (c == '\n')
					{
					yyline--;
					error( "Non-terminated string or character constant");
					}
				}
			error( "EOF in string or character constant" );
			/* NOTREACHED */
			break;
	
		case '\0':
			yyline = savline;
			error("Action does not terminate");
			/* NOTREACHED */
			break;
		default:
			break;		/* usual character */
			}
	loop:
		if(c != ' ' && c != '\t' && c != '\n') sw = FALSE;
		(void) putc((char)c,fout);
		if(peek=='\n' && !brac && copy_line){ (void) putc('\n', fout); return(1); }
		}
	error("Premature EOF");
}
int gch(){
	register int c;
	prev = pres;
	c = pres = peek;
	if (peek != EOF) {
	peek = pushptr > pushc ? *--pushptr : getc(fin);
	while(peek == EOF) {
		if (no_input) {
			if(!yyline)
				error("Cannot read from -- %s",sargv[optind]);
			if(optind < sargc-1) {
				yyline = 0;
				(void) fclose(fin);
				fin = fopen(sargv[++optind],"r");
				if(fin == NULL)
					error("Cannot open file -- %s",sargv[optind]);
				peek = getc(fin);
			} else break;
		} else {
			(void) fclose(fin);
			if(!yyline) error("Cannot read from -- standard input");
			else break;
		}
	}
	}
	if(c == EOF) {
		eof = TRUE;
		return(0);
		}
	if(c == '\n')yyline++;
	return(c);
	}
mn2(a,d,c)
  int a,d,c;
	{
	name[tptr] = a;
	left[tptr] = d;
	right[tptr] = c;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	switch(a){
	case RSTR:
		parent[d] = tptr;
		break;
	case BAR:
	case RNEWE:
		if(nullstr[d] || nullstr[c]) nullstr[tptr] = TRUE;
		parent[d] = parent[c] = tptr;
		break;
	case RCAT:
	case DIV:
		if(nullstr[d] && nullstr[c])nullstr[tptr] = TRUE;
		parent[d] = parent[c] = tptr;
		break;
	case RSCON:
		parent[d] = tptr;
		nullstr[tptr] = nullstr[d];
		break;
# ifdef DEBUG
	default:
		warning("bad switch mn2 %d %d",a,d);
		break;
# endif
		}
	if(tptr > treesize)
		error("Parse tree too big %s",(treesize == TREESIZE?"\nTry using %e num":""));
	return(tptr++);
	}
mn1(a,d)
  int a,d;
	{
	name[tptr] = a;
	left[tptr] = d;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	switch(a){
	case RCCL:
	case RNCCL:
		if(slength((char *)d) == 0) nullstr[tptr] = TRUE;
		break;
	case STAR:
	case QUEST:
		nullstr[tptr] = TRUE;
		parent[d] = tptr;
		break;
	case PLUS:
	case CARAT:
		nullstr[tptr] = nullstr[d];
		parent[d] = tptr;
		break;
	case S2FINAL:
		nullstr[tptr] = TRUE;
		break;
# ifdef DEBUG
	case FINAL:
	case S1FINAL:
		break;
	default:
		warning("bad switch mn1 %d %d",a,d);
		break;
# endif
		}
	if(tptr > treesize)
		error("Parse tree too big %s",(treesize == TREESIZE?"\nTry using %e num":""));
	return(tptr++);
	}
mn0(a)
  int a;
	{
	name[tptr] = a;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	if(a >= NCH) switch(a){
	case RNULLS: nullstr[tptr] = TRUE; break;
# ifdef DEBUG
	default:
		warning("bad switch mn0 %d",a);
		break;
# endif
	}
	if(tptr > treesize)
		error("Parse tree too big %s",(treesize == TREESIZE?"\nTry using %e num":""));
	return(tptr++);
	}
munput(t,p)	/* implementation dependent */
  unsigned char *p;
  int t; {
	register int i,j;
	if(t == 'c'){
		*pushptr++ = peek;		/* watch out for this */
		peek = (int)p;
		}
	else if(t == 's'){
		*pushptr++ = peek;
		peek = p[0];
		i = slength((char *)p);
		for(j = i-1; j>=1; j--)
			*pushptr++ = p[j];
		}
# ifdef DEBUG
	else error("Unrecognized munput option %c",t);
# endif
	if(pushptr >= pushc+TOKENSIZE)
		error("Too many characters pushed");
	return;
	}

dupl(n)
  int n; {
	/* duplicate the subtree whose root is n, return ptr to it */
	register int i;
	i = name[n];
	if(i < NCH) return(mn0(i));
	switch(i){
	case RNULLS:
		return(mn0(i));
	case RCCL: case RNCCL: case FINAL: case S1FINAL: case S2FINAL:
		return(mn1(i,left[n]));
	case STAR: case QUEST: case PLUS: case CARAT:
		return(mn1(i,dupl(left[n])));
	case RSTR: case RSCON:
		return(mn2(i,dupl(left[n]),right[n]));
	case BAR: case RNEWE: case RCAT: case DIV:
		return(mn2(i,dupl(left[n]),dupl(right[n])));
# ifdef DEBUG
	default:
		warning("bad switch dupl %d",n);
# endif
	}
	return(0);
	}
# ifdef DEBUG
allprint(c)
  char c; {
	switch(c){
		case 014:
			(void)printf("\\f");
			charc++;
			break;
		case '\n':
			(void)printf("\\n");
			charc++;
			break;
		case '\t':
			(void)printf("\\t");
			charc++;
			break;
		case '\b':
			(void)printf("\\b");
			charc++;
			break;
		case ' ':
			(void)printf("\\\bb");
			break;
		default:
			if(!printable(c)){
				(void)printf("\\%-3o",c);
				charc += 3;
				}
			else 
				(void) putchar((char)c);
			break;
		}
	charc++;
	return;
	}
strpt(s)
  unsigned char *s; {
	charc = 0;
	while(*s){
		allprint(*s++);
		if(charc > LINESIZE){
			charc = 0;
			(void)printf("\n\t");
			}
		}
	return;
	}
sect1dump(){
	register int i;
	(void)printf("Sect 1:\n");
	if(def[0]){
		(void)printf("str	trans\n");
		i = -1;
		while(def[++i])
			(void)printf("%s\t%s\n",def[i],subs[i]);
		}
	if(sname[0]){
		(void)printf("start names\n");
		i = -1;
		while(sname[++i])
			(void)printf("%s\n",sname[i]);
		}
	if(chset == TRUE){
		(void)printf("char set changed\n");
		for(i=1;i<NCH;i++){
			if(i != ctable[i]){
				allprint(i);
				(void) putchar(' ');
				printable(ctable[i]) ? (void) putchar((char)ctable[i]) : (void)printf("%d",ctable[i]);
				(void) putchar('\n');
				}
			}
		}
	}
sect2dump(){
	(void)printf("Sect 2:\n");
	treedump();
	}
treedump()
	{
	register int t;
	register unsigned char *p;
	(void)printf("treedump %d nodes:\n",tptr);
	for(t=0;t<tptr;t++){
		(void)printf("%4d ",t);
		parent[t] ? (void)printf("p=%4d",parent[t]) : (void)printf("      ");
		(void)printf("  ");
		if(name[t] < NCH) {
				allprint(name[t]);
				}
		else switch(name[t]){
			case RSTR:
				(void)printf("%d ",left[t]);
				allprint(right[t]);
				break;
			case RCCL:
				(void)printf("ccl ");
				strpt(left[t]);
				break;
			case RNCCL:
				(void)printf("nccl ");
				strpt(left[t]);
				break;
			case DIV:
				(void)printf("/ %d %d",left[t],right[t]);
				break;
			case BAR:
				(void)printf("| %d %d",left[t],right[t]);
				break;
			case RCAT:
				(void)printf("cat %d %d",left[t],right[t]);
				break;
			case PLUS:
				(void)printf("+ %d",left[t]);
				break;
			case STAR:
				(void)printf("* %d",left[t]);
				break;
			case CARAT:
				(void)printf("^ %d",left[t]);
				break;
			case QUEST:
				(void)printf("? %d",left[t]);
				break;
			case RNULLS:
				(void)printf("nullstring");
				break;
			case FINAL:
				(void)printf("final %d",left[t]);
				break;
			case S1FINAL:
				(void)printf("s1final %d",left[t]);	
				break;
			case S2FINAL:
				(void)printf("s2final %d",left[t]);
				break;
			case RNEWE:
				(void)printf("new %d %d",left[t],right[t]);
				break;
			case RSCON:
				p = (unsigned char *)right[t];
				(void)printf("start %s",sname[*p++-1]);
				while(*p)
					(void)printf(", %s",sname[*p++-1]);
				(void)printf(" %d",left[t]);
				break;
			default:
				(void)printf("unknown %d %d %d",name[t],left[t],right[t]);
				break;
			}
		if(nullstr[t])(void)printf("\t(null poss.)");
		(void) putchar('\n');
		}
	}
# endif
