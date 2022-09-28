/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)cscope:common/egrep.y	1.4"
/*
 * egrep -- fine lines containing a regular expression
 */
%}

%token CHAR DOT CCL NCCL OR CAT STAR PLUS QUEST
%left OR
%left CHAR DOT CCL NCCL '('
%left CAT
%left STAR PLUS QUEST

%{
#include <ctype.h>
#include <stdio.h>
#ifdef BSD	/* build command requires #ifdef instead of #if */
#undef	tolower		/* BSD tolower doesn't test the character */
#define	tolower(c)	(islower(c) ? (c) : (c) - 'A' + 'a')	
char	*memset();
#else
#ifdef V9
char	*memset();
#else /*V9*/
#include <memory.h>	/* memset */
#endif /*V9*/
#endif
#include <setjmp.h>	/* jmp_buf */

#define nextch()	(*input++)

#define MAXLIN 350
#define MAXPOS 4000
#define NCHARS 256
#define NSTATES 128
#define FINAL -1
static	char gotofn[NSTATES][NCHARS];
static	int state[NSTATES];
static	char out[NSTATES];
static	int line;
static	int name[MAXLIN];
static	int left[MAXLIN];
static	int right[MAXLIN];
static	int parent[MAXLIN];
static	int foll[MAXLIN];
static	int positions[MAXPOS];
static	char chars[MAXLIN];
static	int nxtpos;
static	int nxtchar;
static	int tmpstat[MAXLIN];
static	int initstat[MAXLIN];
static	int xstate;
static	int count;
static	int icount;
static	char *input;
static	long lnum;
static	int iflag;
static	jmp_buf	env;	/* setjmp/longjmp buffer */
static	char *message;	/* error message */
static	int	cstate(), member(), notin();
static	void	synerror(), overflo(), add(), follow();
%}

%%
s:	t
		={ unary(FINAL, $1);
		  line--;
		}
	;
t:	b r
		={ $$ = node(CAT, $1, $2); }
	| OR b r OR
		={ $$ = node(CAT, $2, $3); }
	| OR b r
		={ $$ = node(CAT, $2, $3); }
	| b r OR
		={ $$ = node(CAT, $1, $2); }
	;
b:
		={ $$ = enter(DOT);
		   $$ = unary(STAR, $$); }
	;
r:	CHAR
		={ $$ = enter($1); }
	| DOT
		={ $$ = enter(DOT); }
	| CCL
		={ $$ = cclenter(CCL); }
	| NCCL
		={ $$ = cclenter(NCCL); }
	;

r:	r OR r
		={ $$ = node(OR, $1, $3); }
	| r r %prec CAT
		={ $$ = node(CAT, $1, $2); }
	| r STAR
		={ $$ = unary(STAR, $1); }
	| r PLUS
		={ $$ = unary(PLUS, $1); }
	| r QUEST
		={ $$ = unary(QUEST, $1); }
	| '(' r ')'
		={ $$ = $2; }
	| error 
	;

%%
static void
yyerror(s)
char	*s;
{
	message = s;
	longjmp(env, 1);
}

static
yylex() {
	extern int yylval;
	int cclcnt, x;
	register char c, d;
	switch(c = nextch()) {
		case '$':
		case '^': c = '\n';
			goto defchar;
		case '|': return (OR);
		case '*': return (STAR);
		case '+': return (PLUS);
		case '?': return (QUEST);
		case '(': return (c);
		case ')': return (c);
		case '.': return (DOT);
		case '\0': return (0);
		case '\n': return (OR);
		case '[': 
			x = CCL;
			cclcnt = 0;
			count = nxtchar++;
			if ((c = nextch()) == '^') {
				x = NCCL;
				c = nextch();
			}
			do {
				if (c == '\0') synerror();
				if (c == '-' && cclcnt > 0 && chars[nxtchar-1] != 0) {
					if ((d = nextch()) != 0) {
						c = chars[nxtchar-1];
						while ((unsigned)c < (unsigned)d) {
							if (nxtchar >= MAXLIN) overflo();
							chars[nxtchar++] = ++c;
							cclcnt++;
						}
						continue;
					}
				}
				if (nxtchar >= MAXLIN) overflo();
				chars[nxtchar++] = c;
				cclcnt++;
			} while ((c = nextch()) != ']');
			chars[count] = cclcnt;
			return (x);
		case '\\':
			if ((c = nextch()) == '\0') synerror();
		defchar:
		default: yylval = c; return (CHAR);
	}
}

static void
synerror() {
	yyerror("syntax error");
}

static
enter(x) int x; {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = 0;
	right[line] = 0;
	return(line++);
}

static
cclenter(x) int x; {
	register linno;
	linno = enter(x);
	right[linno] = count;
	return (linno);
}

static
node(x, l, r) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = l;
	right[line] = r;
	parent[l] = line;
	parent[r] = line;
	return(line++);
}

static
unary(x, d) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = d;
	right[line] = 0;
	parent[d] = line;
	return(line++);
}

static void
overflo() {
	yyerror("internal table overflow");
}

static void
cfoll(v) {
	register i;
	if (left[v] == 0) {
		count = 0;
		for (i=1; i<=line; i++) tmpstat[i] = 0;
		follow(v);
		add(foll, v);
	}
	else if (right[v] == 0) cfoll(left[v]);
	else {
		cfoll(left[v]);
		cfoll(right[v]);
	}
}

static void
cgotofn() {
	register c, i, k;
	int n, s;
	char symbol[NCHARS];
	int j, pc, pos;
	unsigned nc;
	int curpos, num;
	int number, newpos;
	count = 0;
	for (n=3; n<=line; n++) tmpstat[n] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[0] = 1;
	}
	for (n=3; n<=line; n++) initstat[n] = tmpstat[n];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 0);
	n = 0;
	for (s=0; s<=n; s++)  {
		if (out[s] == 1) continue;
		for (i=0; i<NCHARS; i++) symbol[i] = 0;
		num = positions[state[s]];
		count = icount;
		for (i=3; i<=line; i++) tmpstat[i] = initstat[i];
		pos = state[s] + 1;
		for (i=0; i<num; i++) {
			curpos = positions[pos];
			if ((c = name[curpos]) >= 0) {
				if (c < NCHARS) symbol[c] = 1;
				else if (c == DOT) {
					for (k=0; k<NCHARS; k++)
						if (k!='\n') symbol[k] = 1;
				}
				else if (c == CCL) {
					nc = chars[right[curpos]];
					pc = right[curpos] + 1;
					for (k=0; k<nc; k++)
						symbol[(unsigned)(chars[pc++])] = 1;
				}
				else if (c == NCCL) {
					nc = chars[right[curpos]];
					for (j = 0; j < NCHARS; j++) {
						pc = right[curpos] + 1;
						for (k = 0; k < nc; k++)
							if (j==(unsigned)(chars[pc++])) goto cont;
						if (j!='\n') symbol[j] = 1;
						cont:;
					}
				}
			}
			pos++;
		}
		for (c=0; c<NCHARS; c++) {
			if (symbol[c] == 1) { /* nextstate(s,c) */
				count = icount;
				for (i=3; i <= line; i++) tmpstat[i] = initstat[i];
				pos = state[s] + 1;
				for (i=0; i<num; i++) {
					curpos = positions[pos];
					if ((k = name[curpos]) >= 0)
						if (
							(k == c)
							|| (k == DOT)
							|| (k == CCL && member(c, right[curpos], 1))
							|| (k == NCCL && member(c, right[curpos], 0))
						) {
							number = positions[foll[curpos]];
							newpos = foll[curpos] + 1;
							for (k=0; k<number; k++) {
								if (tmpstat[positions[newpos]] != 1) {
									tmpstat[positions[newpos]] = 1;
									count++;
								}
								newpos++;
							}
						}
					pos++;
				} /* end nextstate */
				if (notin(n)) {
					if (n >= NSTATES) overflo();
					add(state, ++n);
					if (tmpstat[line] == 1) out[n] = 1;
					gotofn[s][c] = n;
				}
				else {
					gotofn[s][c] = xstate;
				}
			}
		}
	}
}

static
cstate(v) {
	register b;
	if (left[v] == 0) {
		if (tmpstat[v] != 1) {
			tmpstat[v] = 1;
			count++;
		}
		return(1);
	}
	else if (right[v] == 0) {
		if (cstate(left[v]) == 0) return (0);
		else if (name[v] == PLUS) return (1);
		else return (0);
	}
	else if (name[v] == CAT) {
		if (cstate(left[v]) == 0 && cstate(right[v]) == 0) return (0);
		else return (1);
	}
	else { /* name[v] == OR */
		b = cstate(right[v]);
		if (cstate(left[v]) == 0 || b == 0) return (0);
		else return (1);
	}
}

static
member(symb, set, torf) {
	register i, num, pos;
	num = chars[set];
	pos = set + 1;
	for (i=0; i<(unsigned)num; i++)
		if (symb == (unsigned)(chars[pos++])) return (torf);
	return (!torf);
}

static
notin(n) {
	register i, j, pos;
	for (i=0; i<=n; i++) {
		if (positions[state[i]] == count) {
			pos = state[i] + 1;
			for (j=0; j < count; j++)
				if (tmpstat[positions[pos++]] != 1) goto nxt;
			xstate = i;
			return (0);
		}
		nxt: ;
	}
	return (1);
}

static void
add(array, n) int *array; {
	register i;
	if (nxtpos + count > MAXPOS) overflo();
	array[n] = nxtpos;
	positions[nxtpos++] = count;
	for (i=3; i <= line; i++) {
		if (tmpstat[i] == 1) {
			positions[nxtpos++] = i;
		}
	}
}

static void
follow(v) int v; {
	int p;
	if (v == line) return;
	p = parent[v];
	switch(name[p]) {
		case STAR:
		case PLUS:	cstate(v);
				follow(p);
				return;

		case OR:
		case QUEST:	follow(p);
				return;

		case CAT:	if (v == left[p]) {
					if (cstate(right[p]) == 0) {
						follow(p);
						return;
					}
				}
				else follow(p);
				return;
		case FINAL:	if (tmpstat[line] != 1) {
					tmpstat[line] = 1;
					count++;
				}
				return;
	}
}

char *
egrepinit(egreppat)
char	*egreppat;
{
	/* initialize the global data */
	memset((char *) gotofn, 0, sizeof(gotofn));
	memset((char *) state, 0, sizeof(state));
	memset((char *) out, 0, sizeof(out));
	line = 1;
	memset((char *) name, 0, sizeof(name));
	memset((char *) left, 0, sizeof(left));
	memset((char *) right, 0, sizeof(right));
	memset((char *) parent, 0, sizeof(parent));
	memset((char *) foll, 0, sizeof(foll));
	memset((char *) positions, 0, sizeof(positions));
	memset((char *) chars, 0, sizeof(chars));
	nxtpos = 0;
	nxtchar = 0;
	memset((char *) tmpstat, 0, sizeof(tmpstat));
	memset((char *) initstat, 0, sizeof(initstat));
	xstate = 0;
	count = 0;
	icount = 0;
	input = egreppat;
	message = NULL;
	if (setjmp(env) == 0) {
		yyparse();
		cfoll(line-1);
		cgotofn();
	}
	return(message);
}

egrep(file, output, format)
register char *file;
FILE	*output;
char	*format;
{
	register char *p;
	register unsigned cstat;
	register ccount;
	char buf[2*BUFSIZ];
	char *nlp;
	unsigned int istat;
	int in_line;
	register FILE	*fptr;

	if ((fptr = fopen(file, "r")) == NULL) {
		return(-1);
	}
	ccount = 0;
	lnum = 1;
	in_line = 0;
	p = buf;
	nlp = p;
	if ((ccount = fread(p, sizeof(char), BUFSIZ, fptr)) <= 0) goto done;
	in_line = 1;
	istat = cstat = (unsigned)gotofn[0]['\n'];
	if (out[cstat]) goto found;
	for (;;) {
		if (!iflag)
			cstat = (unsigned)gotofn[cstat][(unsigned)*p&0377];
			/* all input chars made positive */
		else
			cstat = (unsigned)gotofn[cstat][tolower((int)*p&0377)];
			/* for -i option*/
		if (out[cstat]) {
		found:	for(;;) {
				if (*p++ == '\n') {
					in_line = 0;
				succeed:
					(void) fprintf(output, format, file, lnum);
					if (p <= nlp) {
						while (nlp < &buf[2*BUFSIZ]) (void) putc(*nlp++, output);
						nlp = buf;
					}
					while (nlp < p) (void) putc(*nlp++, output);
					lnum++;
					nlp = p;
					if ((out[(cstat=istat)]) == 0) goto brk2;
				}
				cfound:
				if (--ccount <= 0) {
					if (p <= &buf[BUFSIZ]) {
						ccount = fread(p, sizeof(char), BUFSIZ, fptr);
					}
					else if (p == &buf[2*BUFSIZ]) {
						p = buf;
						ccount = fread(p, sizeof(char), BUFSIZ, fptr);
					}
					else {
						ccount = fread(p, sizeof(char), &buf[2*BUFSIZ] - p, fptr);
					}
					if (ccount <= 0) {
						if (in_line) {
							in_line = 0;
							goto succeed;
						}
						goto done;
					}
				}
				in_line = 1;
			}
		}
		if (*p++ == '\n') {
			in_line = 0;
			lnum++;
			nlp = p;
			if (out[(cstat=istat)]) goto cfound;
		}
		brk2:
		if (--ccount <= 0) {
			if (p <= &buf[BUFSIZ]) {
				ccount = fread(p, sizeof(char), BUFSIZ, fptr);
			}
			else if (p == &buf[2*BUFSIZ]) {
				p = buf;
				ccount = fread(p, sizeof(char), BUFSIZ, fptr);
			}
			else {
				ccount = fread(p, sizeof(char), &buf[2*BUFSIZ] - p, fptr);
			}
			if (ccount <= 0) {
				break;
			}
		}
		in_line = 1;
	}
done:	(void) fclose(fptr);
	return(0);
}
#if BSD
/*LINTLIBRARY*/
/*
 * Set an array of n chars starting at sp to the character c.
 * Return sp.
 */
char *
memset(sp, c, n)
register char *sp, c;
register int n;
{
	register char *sp0 = sp;

	while (--n >= 0)
		*sp++ = c;
	return (sp0);
}
#endif

void
egrepcaseless(i)
int i;
{
	iflag = i;	/* simulate "egrep -i" */
}
	
