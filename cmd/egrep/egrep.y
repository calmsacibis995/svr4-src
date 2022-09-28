/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


%{
#ident	"@(#)egrep:egrep.y	1.10.1.10"
%}
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */
/*
 * egrep -- print lines containing (or not containing) a regular expression
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error; matches irrelevant
 */
%token CHAR MCHAR DOT MDOT CCL NCCL MCCL NMCCL OR CAT STAR PLUS QUEST
%left OR
%left CHAR MCHAR DOT CCL NCCL MCCL NMCCL '('
%left CAT
%left STAR PLUS QUEST

%{
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <libw.h>
#include <stdlib.h>
#include <limits.h>
#include <locale.h>
#define BLKSIZE 512	/* size of reported disk blocks */
#define MAXLIN 350
#define NCHARS 256
#define MAXPOS 4000
#define NSTATES 64
#define FINAL -1
#define RIGHT '\n'	/* serves as record separator and as $ */
#define LEFT '\n'	/* beginning of line */
#define Isalpha(c) (c <= 0177 || !multibyte ? isalpha(c) : 0)
#define SS2 0216
#define SS3 0217
#define MASK 0x8080
#define MASK1 0x8080
#define MASK2 0x0080
#define MASK3 0x8000
char gotofn[NSTATES][NCHARS];
int state[NSTATES];
char out[NSTATES];
int line  = 1;
int *name;
int *left;
int *right;
int *parent;
int *foll;
int *positions;
char *chars;
wchar_t *lower;
wchar_t *upper;
int maxlin, maxclin, maxwclin, maxpos;
int nxtpos = 0;
int inxtpos;
int nxtchar = 0;
int *tmpstat;
int *initstat;
int istat;
int nstate = 1;
int xstate;
int count;
int icount;
char *input;

eucwidth_t wp;
#define multibyte wp._multibyte
#define eucw1 wp._eucw1
#define eucw2 wp._eucw2
#define eucw3 wp._eucw3

wchar_t lyylval;
wchar_t nextch();
wchar_t maxmin();
int compare();
void overflo();

char reinit = 0;

long	lnum;
int	bflag;
int	cflag;
int	eflag;
int	fflag;
int	hflag;
int	iflag;
int	lflag;
int	nflag;
int	sflag;
int	vflag;
int	nfile;
long	blkno;
long	tln;
int	nsucc;
int	badbotch;
extern 	char *optarg;
extern 	int optind;

int	f;
FILE	*expfile;
%}

%%
s:	t
		{ 
		  unary(FINAL, $1);
		  line--;
		}
	;
t:	b r
		{ $$ = node(CAT, $1, $2); }
	| OR b r OR
		{ $$ = node(CAT, $2, $3); }
	| OR b r
		{ $$ = node(CAT, $2, $3); }
	| b r OR
		{ $$ = node(CAT, $1, $2); }
	;
b:
		{ /* if(multibyte)
			$$ = mdotenter();
		  else */
			$$ = enter(DOT);
		  $$ = unary(STAR, $$); 
		}
	;
r:	CHAR
		{ $$ = iflag && isalpha($1) ? node(OR, enter(tolower($1)), enter(toupper($1))) : enter($1); }
	| MCHAR
		{ $$ = mchar(lyylval); }
	| DOT
		{ if(multibyte)
			$$ = mdotenter();
		  else
			$$ = enter(DOT); 
		}
	| CCL
		{ $$ = cclenter(CCL); }
	| NCCL
		{ $$ = cclenter(NCCL); }
	| MCCL
		{ $$ = ccl(CCL); }
	| NMCCL
		{ $$ = ccl(NCCL); }
	;

r:	r OR r
		{ $$ = node(OR, $1, $3); }
	| r r %prec CAT
		{ $$ = node(CAT, $1, $2); }
	| r STAR
		{ $$ = unary(STAR, $1); }
	| r PLUS
		{ $$ = unary(PLUS, $1); }
	| r QUEST
		{ $$ = unary(QUEST, $1); }
	| '(' r ')'
		{ $$ = $2; }
	| error 
	;

%%
yyerror(s) {
	fprintf(stderr, "egrep: %s\n", s);
	exit(2);
}

yylex() {
	extern int yylval;
	int cclcnt, x, ccount, oldccount;
	register wchar_t c, lc;
		
	c = nextch();
	switch(c) {
		case '^': 
			yylval = LEFT;
			return(CHAR);
		case '$': 
			c = RIGHT;
			goto defchar;
		case '|': return (OR);
		case '*': return (STAR);
		case '+': return (PLUS);
		case '?': return (QUEST);
		case '(': return (c);
		case ')': return (c);
		case '.': return(DOT);
		case '\0': return (0);
		case RIGHT: return (OR);
		case '[': 
			x = (multibyte ? MCCL : CCL);
			cclcnt = 0;
			count = nxtchar++;
			if ((c = nextch()) == '^') {
				x = (multibyte ? NMCCL : NCCL);
				c = nextch();
			}
			lc = 0;
			do {
				if(iflag && Isalpha(c))
					c = tolower(c);
				if (c == '\0') synerror();
				if (c == '-' && cclcnt > 0 && lc != 0) {
					if ((c = nextch()) != 0) {
						if(c == ']') {
							chars[nxtchar++] = '-';
							cclcnt++;
							break;
						}
						if(iflag && Isalpha(c))
							c = tolower(c);
						if(!multibyte || (c & MASK) == (lc & MASK) && lc < c && (c > 0377 || !iscntrl(c)) && (lc > 0377 || !iscntrl(lc))) {
							if(nxtchar >= maxclin)
								if(allocchars() == 0)
									overflo();
							chars[nxtchar++] = '-';
							cclcnt++;
						}
					}
				}
				ccount = oldccount = nxtchar;
				if(ccount + MB_LEN_MAX >= maxclin)
					if(allocchars() == 0)
						overflo();
				ccount += wctomb(&chars[ccount], c);
				cclcnt += ccount - oldccount;
				nxtchar += ccount - oldccount;
				lc = c;
			} while ((c = nextch()) != ']');
			chars[count] = cclcnt;
			return(x);
		
		case '\\':
			if ((c = nextch()) == '\0') synerror();
		defchar:
		default: if(c <= 0177 || !multibyte) {
				yylval = c; return (CHAR);
			 }
			 lyylval = c;
			 return(MCHAR);
	}
}
int mgetc();

wchar_t nextch() {
	wchar_t lc;
	char multic[MB_LEN_MAX];
	int length, d;
	if (fflag) {
		if ((length = mbftowc(multic, &lc, mgetc, &d)) < 0)
			synerror();
		if(length == 0)
			lc = '\0';
	}
	else  {
		if((length = mbtowc(&lc, input, MB_LEN_MAX)) == -1)
			synerror();
		if(length == 0)
			return(0);
		input += length;
	}
	return(lc);
}

int mgetc()
{
	return(getc(expfile));
}
	
synerror() {
	fprintf(stderr, "egrep: syntax error\n");
	exit(2);
}

enter(x) int x; {
	if(line >= maxlin) 
		if(alloctree() == 0)
			overflo();
	name[line] = x;
	left[line] = 0;
	right[line] = 0;
	return(line++);
}

cclenter(x) int x; {
	register linno;
	linno = enter(x);
	right[linno] = count;
	return (linno);
}

node(x, l, r) {
	if(line >= maxlin) 
		if(alloctree() == 0)
			overflo();
	name[line] = x;
	left[line] = l;
	right[line] = r;
	parent[l] = line;
	parent[r] = line;
	return(line++);
}

unary(x, d) {
	if(line >= maxlin) 
		if(alloctree() == 0)
			overflo();
	name[line] = x;
	left[line] = d;
	right[line] = 0;
	parent[d] = line;
	return(line++);
}

allocchars()
{
	maxclin += MAXLIN;
	if((chars = realloc(chars, maxclin)) == (char *)0)
		return 0;
	return 1;
}

alloctree()
{
	maxlin += MAXLIN;
	if((name = (int *)realloc(name, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((left = (int *)realloc(left, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((right = (int *)realloc(right, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((parent = (int *)realloc(parent, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((foll = (int *)realloc(foll, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((tmpstat = (int *)realloc(tmpstat, maxlin*sizeof(int))) == (int *)0)
		return 0;
	if((initstat = (int *)realloc(initstat, maxlin*sizeof(int))) == (int *)0)
		return 0;
	return 1;
}

void overflo() 
{
	fprintf(stderr, "egrep: regular expression too long\n");
	exit(2);
}

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

cgotofn() {
	register i;
	count = 0;
	inxtpos = nxtpos;
	for (i=3; i<=line; i++) tmpstat[i] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[1] = 1;
	}
	for (i=3; i<=line; i++) initstat[i] = tmpstat[i];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 1);
	istat = nxtst(1, LEFT);
}

nxtst(s, c)
{
	register i, num, k;
	int pos, curpos, number, newpos;
	num = positions[state[s]];
	count = icount;
	for (i=3; i<=line; i++) tmpstat[i] = initstat[i];
	pos = state[s] + 1;
	for (i=0; i<num; i++) {
		curpos = positions[pos];
		k = name[curpos];
		if (k >= 0)
			if (
				(k == c)
				|| (k == DOT && dot(c))
				|| (k == MDOT && mdot(c))
				|| (k == CCL && dot(c) && member(c, right[curpos], 1))
				|| (k == NCCL && dot(c) && member(c, right[curpos], 0))
				|| (k == MCCL && mdot(c) && member(c, right[curpos], 1))
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
	}
	if (notin(nstate)) {
		if (++nstate >= NSTATES) {
			for (i=1; i<NSTATES; i++)
				out[i] = 0;
			for (i=1; i<NSTATES; i++)
				for (k=0; k<NCHARS; k++)
					gotofn[i][k] = 0;
			nstate = 1;
			nxtpos = inxtpos;
			reinit = 1;
			add(state, nstate);
			if (tmpstat[line] == 1) out[nstate] = 1;
			return nstate;
		}
		add(state, nstate);
		if (tmpstat[line] == 1) out[nstate] = 1;
		gotofn[s][c] = nstate;
		return nstate;
	}
	else {
		gotofn[s][c] = xstate;
		return xstate;
	}
}


cstate(v) 
{
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


dot(c)
int c;
{
	if(multibyte && c >= 0200 && (!iscntrl(c) || c == SS2 && eucw2 || c == SS3 && eucw3))
		return(0);
	if(c == RIGHT || c == LEFT)
		return(0);
	return(1);
}

mdot(c)
int c;
{
	if(c >= 0200 && !iscntrl(c))
		return(1);
	return(0);
}

member(symb, set, torf) 
{
	register i, num, pos, c, lc;
	if(symb == RIGHT || symb == LEFT)
		return(0);
	num = chars[set];
	pos = set + 1;
	lc = 0;
	if(iflag) 
		symb = tolower(symb);
	for (i=0; i<num; i++) {
		c = (unsigned char)chars[pos++];
		if(c == '-' && lc != 0 && i++ < num) {
			c = (unsigned char)chars[pos++];
			if(lc <= symb && symb <= c)
				return(torf);
		}
		if (symb == c)
			return (torf);
		lc = c;
	}
	return(!torf);
}

notin(n)
int n;
{
	register int i, j, pos;
	for (i=1; i<=n; i++) {
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

add(array, n) 
int *array; 
{
	register i;
	if (nxtpos + count >= maxpos) { 
		maxpos += MAXPOS + count;
		if((positions = (int *)realloc(positions, maxpos *sizeof(int))) == (int *)0)
			overflo();
	}
	array[n] = nxtpos;
	positions[nxtpos++] = count;
	for (i=3; i <= line; i++) {
		if (tmpstat[i] == 1) {
			positions[nxtpos++] = i;
		}
	}
}

follow(v) 
int v; 
{
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


main(argc, argv)
char **argv;
{
	register c;
	char *usage;
	char nl = '\n';
	int errflag = 0;
	usage = "[ -bchilnv ] [ -e exp ] [ -f file ] [ strings ] [ file ] ...";
	
	(void)setlocale(LC_ALL, "");
	getwidth(&wp);
	while((c = getopt(argc, argv, "ybcie:f:hlnv")) != -1)
		switch(c) {

		case 'b':
			bflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'e':
			eflag++;
			input = optarg;
			continue;

		case 'f':
			fflag++;
			expfile = fopen(optarg, "r");
			if(expfile == NULL) {
				fprintf(stderr, "egrep: can't open %s\n", optarg);
				exit(2);
			}
			continue;

		case 'h':
			hflag++;
			continue;

		case 'y':
		case 'i':
			iflag++;
			continue;

		case 'l':
			lflag++;
			continue;

		case 'n':
			nflag++;
			continue;

		case 's':
			sflag++;
			continue;

		case 'v':
			vflag++;
			continue;

		case '?':
			errflag++;
		}
	if (errflag || ((argc <= 0) && !fflag && !eflag)) {
		fprintf(stderr, "usage: egrep %s\n", usage);
		exit(2);
	}
	if(!eflag && !fflag) {
		input = argv[optind];
		optind++;
	}

	argc -= optind;
	argv = &argv[optind];
	
	/* allocate initial space for arrays */
	if((name = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((left = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((right = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((parent = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((foll = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((tmpstat = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((initstat = (int *)malloc(MAXLIN*sizeof(int))) == (int *)0)
		overflo();
	if((chars = (char *)malloc(MAXLIN)) == (char *)0)
		overflo();
	if((lower = (wchar_t *)malloc(MAXLIN*sizeof(wchar_t))) == (wchar_t *)0)
		overflo();
	if((upper = (wchar_t *)malloc(MAXLIN*sizeof(wchar_t))) == (wchar_t *)0)
		overflo();
	if((positions = (int *)malloc(MAXPOS*sizeof(int))) == (int *)0)
		overflo();
	maxlin = MAXLIN;
	maxclin = MAXLIN;
	maxwclin = MAXLIN;
	maxpos = MAXPOS;
		
	yyparse();

	cfoll(line-1);
	cgotofn();
	nfile = argc;
	if (argc<=0) {
		if (lflag) exit(1);
		execute(0);
	}
	else while (--argc >= 0) {
		if (reinit == 1) clearg();
		execute(*argv++);
	}
	exit(badbotch ? 2 : nsucc==0);
}

execute(file)
char *file;
{
	register char *p;
	register cstat;
	register wchar_t c;
	register t;
	register count;
	int count1, count2;
	int nchars;
	int succ;
	char *ptr, *ptrend, *lastptr;
	char buf[2*BUFSIZ];
	FILE *f;
	int nlflag;
	if (file) {
		if ((f = fopen(file, "r")) == NULL) {
			fprintf(stderr, "egrep: can't open %s\n", file);
			badbotch=1;
			return;
		}
	}
	else 
		f = stdin;
	lnum = 1;
	tln = 0;
	if((count = read(fileno(f), buf, BUFSIZ)) <= 0) {
		fclose(f);

		if (cflag) {
			if (nfile>1 && !hflag)
				fprintf(stdout, "%s:", file);
			fprintf(stdout, "%ld\n", tln);
		}
		return;
	}
	blkno = count;
	ptr = buf;
	for(;;) {
		if((ptrend = memchr(ptr, '\n', buf + count - ptr)) == NULL) {
			count = buf + count - ptr;
			if(count > BUFSIZ) {
				ptr = p = buf;
				count = 0;
			} else {
				if(ptr != buf)
					(void)memmove(buf, ptr, count);
				p = buf + count;
				ptr = buf;
			}
			if((count1 = read(fileno(f), p, BUFSIZ)) > 0) {
				count += count1;
				blkno += count1;
				continue;
			}
			ptrend = ptr + count;
			nlflag = 0;
		} else
			nlflag = 1;
		*ptrend = '\n';
		p = ptr;
		lastptr = ptr;
		cstat = istat;
		succ = 0;
		for(;;) {
			if(out[cstat]) {
				if(multibyte && p > ptr) {
					wchar_t wchar;
					int length;
					char *endptr = p;
					p = lastptr;
					while(p < endptr) {
						length = mbtowc(&wchar, p, MB_LEN_MAX);
						if(length <= 1)
							p++;
						else
							p += length;
					}
					if(p == endptr) {
						succ = !vflag;
						break;
					}
					cstat = 1;
					length = mbtowc(&wchar, lastptr, MB_LEN_MAX);
					if(length <= 1)
						lastptr++;
					else
						lastptr += length;
					p = lastptr;
					continue;
				}
				succ = !vflag;
				break;
			}
			c = (unsigned char)*p++;
			if ((t = gotofn[cstat][c]) == 0)
				cstat = nxtst(cstat, c);
			else
				cstat = t;
			if(c == RIGHT) {
				if(out[cstat]) {
					succ = !vflag;
					break;
				}
				succ = vflag;
				break;
			}
		}
		if(succ) {
			nsucc = 1;
			if (cflag) tln++;
			else if (sflag)
				;	/* ugh */
			else if (lflag) {
				printf("%s\n", file);
				fclose(f);
				return;
			}
			else {
				if (nfile > 1 && !hflag) 
					printf("%s:", file);
				if (bflag) {
					nchars = blkno - (buf + count - ptrend) - 2;
					if(nlflag)
						nchars++;
					printf("%ld:", nchars/BLKSIZE);
				}
				if (nflag) 
					printf("%ld:", lnum);
				if(nlflag)
					nchars = ptrend - ptr + 1;
				else
					nchars = ptrend - ptr;
				fwrite(ptr, 1, nchars, stdout);
			}
		}
		if(!nlflag)
			break;
		ptr = ptrend + 1;
		if(ptr >= buf + count) {
			ptr = buf;
			if((count = read(fileno(f), buf, BUFSIZ)) <= 0)
				break;
			blkno += count;
		}
		lnum++;
		if (reinit == 1) 
			clearg();
	}
	fclose(f);
	if (cflag) {
		if (nfile > 1 && !hflag)
			printf("%s:", file);
		printf("%ld\n", tln);
	}
}

clearg() {
	register i, k;
	for (i=1; i<=nstate; i++)
		out[i] = 0;
	for (i=1; i<=nstate; i++)
		for (k=0; k<NCHARS; k++)
			gotofn[i][k] = 0;
	nstate = 1;
	nxtpos = inxtpos;
	reinit = 0;
	count = 0;
	for (i=3; i<=line; i++) tmpstat[i] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[1] = 1;
	}
	for (i=3; i<=line; i++) initstat[i] = tmpstat[i];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 1);
	istat = nxtst(1, LEFT);
}

mdotenter()
{
	int i, x1, x2;
	x1 = enter(DOT);
	x2 = enter(MDOT);
	for(i = 1; i < eucw1; i++) 
		x2 = node(CAT, x2, enter(MDOT));
	x1 = node(OR, x1, x2);
	if(eucw2) {
		x2 = enter('\216');
		for(i = 1; i <= eucw2; i++) 
			x2 = node(CAT, x2, enter(MDOT));
		x1 = node(OR, x1, x2);
	}
	if(eucw3) {
		x2 = enter('\217');
		for(i = 1; i <= eucw3; i++) 
			x2 = node(CAT, x2, enter(MDOT));
		x1 = node(OR, x1, x2);
	}
	return(x1);
}

mchar(c)
wchar_t c;
{
	char multichar[MB_LEN_MAX+1];
	char *p;
	int x1, lc, length;
	
	length = wctomb(multichar, c);
	p = multichar;
	*(p + length) = '\0';
	x1 = enter(*p++);
	while(lc = (unsigned char)*p++)
		x1 = node(CAT, x1, enter(lc));
	return(x1);
}

ccl(type) 
int type;
{
	wchar_t c, lc;
	char multic1[MB_LEN_MAX];
	char multic2[MB_LEN_MAX];
	int x1, x2, length, current, last, cclcnt;
	x2 = 0;
	current = 0;
	last = genrange(type);
	nxtchar = count + 1;
	cclcnt = 0;
	/* create usual character class for single byte characters */
	while(current <= last && (isascii(c = lower[current]) || c <= 0377 && iscntrl(c))) {
		cclcnt++;
		chars[nxtchar++] = c;
		if(lower[current] != upper[current]) {
			chars[nxtchar++] = '-';
			chars[nxtchar++] = upper[current];
			cclcnt += 2;
		}
		current++;
	}
	
	if(cclcnt)
		chars[count] = cclcnt;
	else
		nxtchar = count;
	if(current > 0)
		/* single byte part of character class */
		x2 = cclenter(type);
	else if(type == NCCL)
		/* all single byte characters match */
		x2 = enter(DOT);
	while(current <= last) {
		if(upper[current] == lower[current]) 
			x1 = mchar(lower[current]);
		else {
			length = wctomb(multic1, lower[current]);
			wctomb(multic2, upper[current]);
			x1 = range(multic1, multic2, length);
		}
		if(x2)
			x2 = node(OR, x2, x1);
		else
			x2 = x1;
		current++;
	}
	return x2;
}

range(p1, p2, length)
unsigned char *p1, *p2;
int length;
{
	char multic[MB_LEN_MAX+1];
	char *p;
	int i, x1, x2;
	if(length == 1)
		return(classenter(*p1, *p2));
	if(p1[0] == p2[0]) 
		return(node(CAT, enter(p1[0]), range(p1+1, p2+1, length - 1)));		
	p = multic;
	for(i = 1; i < length; i++)
		*p++ = 0377;
	x1 = node(CAT, enter(p1[0]), range(p1+1, multic, length - 1));
	if(p1[0] + 1 < p2[0]) {
		x2 = classenter(p1[0] + 1, p2[0] - 1);
		for(i = 1; i < length; i++)
			x2 = node(CAT, x2, enter(MDOT));
		x1 = node(OR, x1, x2);
	}
	p = multic;
	for(i = 1; i < length; i++) 
		*p++ = 0200;
	x2 = node(CAT, enter(p2[0]), range(multic, p2+1, length - 1));
	return node(OR, x1, x2);
}

classenter(x1, x2)
int x1, x2;
{
	static int max, min;
	if(!max) {
		int i;
		for(i = 0200; i <= 0377; i++)
			if(!iscntrl(i))
				break;
		min = i;
		for(i = 0377; i >= 0200; i--)
			if(!iscntrl(i))
				break;
		max = i;
	}
	if(x1 <= min && x2 >= max)
		return enter(MDOT);
	if(nxtchar + 4 >= maxclin)
		if(allocchars() == 0)	
			overflo();
	count = nxtchar++;
	chars[nxtchar++] = x1;
	chars[nxtchar++] = '-';
	chars[nxtchar++] = x2;
	chars[count] = 3;
	return cclenter(MCCL);
}

genrange(type)
int type;
{
	char *p, *endp;
	int current, nel, i, last, length;
	wchar_t c, lc;

	current = 0;
	p = &chars[count+1];
	endp = &chars[count+1] + chars[count];
	lc = 0;
		
	/* convert character class into union of ranges */
	while(p < endp) {
		length = mbtowc(&c, p, MB_LEN_MAX);
		p += length;
		if(c == '-' && lc != 0) {
			length = mbtowc(&c, p, MB_LEN_MAX);
			upper[current-1] = c;
			p += length;
		} else {
			lower[current] = c;
			upper[current++] = c;
		}
		lc = c;
	}
	nel = current;
	/* sort lower and upper bounds of ranges */
	qsort((char *)lower, nel, sizeof(wchar_t), compare);
	qsort((char *)upper, nel, sizeof(wchar_t), compare);
	last = current - 1;
	current = 0;
	/* combine overlapping or adjacent ranges */
	for(i = 0; i < last; i++)
		if(upper[i] >= lower[i+1] - 1)
			upper[current] = upper[i+1];
		else {
			lower[++current] = lower[i+1];
			upper[current] = upper[i+1];
		}
	if(type == NCCL) {
		/* find complement of character class */
		int j, next;
		i = 0;
		while(i <= current && isascii(c=lower[i]) || c <= 0377 && iscntrl(c))
			i++;
		if(i > current) {
			/* match all multibyte characters */
			if(eucw2) {
				lower[i] = maxmin(MASK2, 0);
				upper[i++] = maxmin(MASK2, 1);
			}
			if(eucw3) {
				lower[i] = maxmin(MASK3, 0);
				upper[i++] = maxmin(MASK3, 1);
			}
			lower[i] = maxmin(MASK1, 0);
			upper[i++] = maxmin(MASK1, 1);
			return i - 1;
		}
		next = current + 1;
		if(next + current + 2 >= maxwclin) {
			maxwclin += MAXLIN + next + current + 2;
			if((lower = (wchar_t *)realloc(lower, maxwclin *sizeof(wchar_t))) == (wchar_t *)0 ||
			   (upper = (wchar_t *)realloc(upper, maxwclin * sizeof(wchar_t))) == (wchar_t *)0)
				overflo();
		}
		if(eucw2 && lower[i] > maxmin(MASK2, 0)) {
			lower[next] = maxmin(MASK2, 0);
			if((lower[i] & MASK) != MASK2) {
				upper[next++] = maxmin(MASK2, 1);
				if((lower[i] & MASK) == MASK1 && eucw3) {
					lower[next] = maxmin(MASK3, 0);
					upper[next++] = maxmin(MASK3, 1);
				}
				if(lower[i] > maxmin(lower[i] & MASK, 0)) {
					lower[next] = maxmin(lower[i] & MASK, 0);
					upper[next++] = lower[i] - 1;
				}
			} else
				upper[next++] = lower[i] - 1;
		} else if(lower[i] > maxmin(lower[i] & MASK, 0)) {
			lower[next] = maxmin(lower[i] & MASK, 0);
			upper[next++] = lower[i] - 1;
		}
		for(j = i; j < current; j++) {
			if(upper[j] < maxmin(upper[j] & MASK, 1)) {
				lower[next] = upper[j] + 1;
				if((upper[j] & MASK) != (lower[j+1] & MASK)) {
					upper[next++] = maxmin(upper[j] & MASK, 1);
					if(eucw3 && (upper[j] & MASK) == MASK2 && (lower[j+1] & MASK) == MASK1) {
						lower[next] = maxmin(MASK3, 0);
						upper[next++] = maxmin(MASK3, 1);
					}
					if(lower[j+1] > maxmin(lower[j+1] & MASK, 0)) {
						lower[next] = maxmin(lower[j+1] & MASK, 0);
						upper[next++] = lower[j+1] - 1;
					}
				} else
					upper[next++] = lower[j+1] - 1;
			} else if(lower[j+1] > maxmin(lower[j+1], 0)) {
				lower[next] = maxmin(lower[j+1], 0);
				upper[next++] = lower[j+1] - 1;
			}
		}
		if(upper[current] < maxmin(upper[current] & MASK, 1)) {
			lower[next] = upper[current] + 1;
			upper[next++] = maxmin(upper[current] & MASK, 1); 
		}
		if((upper[current] & MASK) != MASK1) {
			if((upper[current] & MASK) == MASK2 && eucw3) {
				lower[next] = maxmin(MASK3, 0);
				upper[next++] = maxmin(MASK3, 1);
			}
			lower[next] = maxmin(MASK1, 0);
			upper[next++] = maxmin(MASK1, 1);
		}
		for(j = current + 1; j < next; j++) {
			lower[i] = lower[j];
			upper[i++] = upper[j];
		}
		current = i - 1;
	}
	return(current);
}

int compare(c, d)
wchar_t *c, *d;
{
	if(*c < *d)
		return -1;
	if(*c == *d)
		return 0;
	return 1;
}

wchar_t maxmin(c, flag)
wchar_t c;
int flag;
{
	static wchar_t minmax1[2], minmax2[2], minmax3[2];

	if(!minmax1[0]) {
		/* compute min and max process codes for all code sets */
		int length, i;
		char multic[MB_LEN_MAX], minmax[2];
		for(i = 0377; i >= 0200; i--)
			if(!iscntrl(i))
				break;
		minmax[1] = i;
		for(i = 0200; i <= 0377; i++)
			if(!iscntrl(i))
				break;
		minmax[0] = i;
		for(i = 0; i <= 1; i++) {
			length = MB_LEN_MAX;
			while(length--)
				multic[length] = minmax[i];
			mbtowc(&minmax1[i], multic, MB_LEN_MAX);
			if(eucw2) {
				multic[0] = SS2;
				mbtowc(&minmax2[i], multic, MB_LEN_MAX);
			}
			if(eucw3) {
				multic[0] = SS3;
				mbtowc(&minmax3[i], multic, MB_LEN_MAX);
			}
		}
	}
	switch(c) {
		case MASK1: return minmax1[flag];
		case MASK2: return minmax2[flag];
		case MASK3: return minmax3[flag];
	}
}
