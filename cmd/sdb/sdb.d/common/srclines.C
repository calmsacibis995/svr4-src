#ident	"@(#)sdb:sdb.d/common/srclines.C	1.3"

#include <stdio.h>
#include "utility.h"
#include <string.h>
#include "Interface.h"

/*
** These procedures manage the source files examined by debug.
*/

/* Print the current line. */
void fprint()
{
	SrcFile *sf;
	long currline;

	sf=find_srcfile(current_src(current_process,&currline));

	printx("%ld:\t%s",currline,src_text(sf,currline));
}

/*
**	Make line `num' current.
*/
void ffind(long num)
{
	SrcFile *sf;
	char *fname;
	long currline, numlines;

	fname=current_src(current_process,&currline);
	sf=find_srcfile(fname);

	numlines=num_lines(sf);

	if(num>=1&&num<=numlines)
		set_current_src(current_process,fname,num);
	else
		printe("only %ld lines in `%s'\n",numlines,fname);
}

/* Go back n lines. */
void fback(long n)
{
	SrcFile *sf;
	char *fname;
	long currline;

	fname=current_src(current_process,&currline);
	sf=find_srcfile(fname);

	currline -= n;
	if(currline < 1)
		currline = 1;

	set_current_src(current_process,fname,currline);
}

/* Go forwards n lines. */
void fforward(long n)
{
	SrcFile *sf;
	char *fname;
	long currline;

	fname=current_src(current_process,&currline);
	sf=find_srcfile(fname);

	currline += n;
	if(currline > num_lines(sf))
		currline = num_lines(sf);

	set_current_src(current_process,fname,currline);
}

/* Print n lines. */
fprintn(int n)
{
	SrcFile *sf;
	long fline, lline;
	char *fname;

	fname=current_src(current_process,&fline);
	sf=find_srcfile(fname);

	lline=fline+n-1;

	if(lline>num_lines(sf)) {
		lline = num_lines(sf);
		n=lline-fline+1;
	}

	while (fline<=lline) {
		printx("%ld:\t%s",fline,src_text(sf,fline));
		fline++;
	}
	set_current_src(current_process,fname,lline);

	return n;
}

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CCEOF	11	/*  CEOF --> CCEOF (conflict with <termio.h>) */
#define	CKET	12
#define	CBACK	18

#define	CSTAR	01

#define	LBSIZE	512
#define	ESIZE	256
#define	NBRA	9

static char	expbuf[ESIZE];
static int	circf;
static char	*braslist[NBRA];
static char	*braelist[NBRA];
static char	bittab[] = {
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128
};

static int ecmp(char *, char *, int);
static int advance(register char *lp, register char *ep);
static int match(char *p1);
static void compile(char *astr);

int
dore(char *nre, int redir)
{
	register SrcFile *sf;
	register char *p, *fname;
	long cline,tline,mline;
	static char re[256];

	circf = 0;

	if(
		(fname=current_src(current_process,&cline))==0
			||
		(sf=find_srcfile(fname))==0
			||
		(mline=num_lines(sf))<=0
	) {
		printe("no source file\n");
		return 0;
	}

	if(nre[0]!='\0')
	{
		int len = strlen(nre);
		strcpy(re,nre);
	}

	compile(re);

	tline=cline;
	do {
		if(redir) {
			if(tline==mline)
				tline=1;
			else
				tline+=1;
		}
		else {
			if(tline==1)
				tline=mline;
			else
				tline-=1;
		}
		p = src_text(sf,tline);
		if (match(p)) {
			set_current_src(current_process,fname,tline);
			fprint();
			return 1;
		}
	} while (tline!=cline);
	printe("No match\n");
	return 0;
}

static void compile(char *astr)
{
	register c;
	register char *ep, *sp;
	char *cstart;
	char *lastep;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	char numbra;
	char neg;

	ep = expbuf;
	sp = astr;
	lastep = 0;
	bracketp = bracket;
	closed = numbra = 0;
	if (*sp == '^') {
		circf++;
		sp++;
	}
	memset(expbuf, 0, ESIZE);
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		if ((c = *sp++) != '*')
			lastep = ep;
		switch (c) {

		case '\0':
			*ep++ = CCEOF;
			return;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= &expbuf[ESIZE])
				goto cerror;
			*ep++ = CCL;
			neg = 0;
			if((c = *sp++) == '^') {
				neg = 1;
				c = *sp++;
			}
			cstart = sp;
			do {
				if (c=='\0')
					goto cerror;
				if (c=='-' && sp>cstart && *sp!=']') {
					for (c = sp[-2]; c<*sp; c++)
						ep[c>>3] |= bittab[c&07];
					sp++;
				}
				ep[c>>3] |= bittab[c&07];
			} while((c = *sp++) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] ^= -1;
				ep[0] &= 0376;
			}

			ep += 16;

			continue;

		case '\\':
			if((c = *sp++) == '(') {
				if(numbra >= NBRA) {
					goto cerror;
				}
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if(c == ')') {
				if(bracketp <= bracket) {
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;
			}

			if(c >= '1' && c <= '9') {
				if((c -= '1') >= closed)
					goto cerror;
				*ep++ = CBACK;
				*ep++ = c;
				continue;
			}

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
    cerror:
	printe("RE error\n", (char *)NULL);
}

static int match(char *p1)
{
	register char *p2;
	register c;
		p2 = expbuf;
		if (circf) {
			if (advance(p1, p2))
				goto found;
			goto nfound;
		}
		/* fast check for first character */
		if (*p2==CCHR) {
			c = p2[1];
			do {
				if (*p1!=c)
					continue;
				if (advance(p1, p2))
					goto found;
			} while (*p1++);
			goto nfound;
		}
		/* regular algorithm */
		do {
			if (advance(p1, p2))
				goto found;
		} while (*p1++);
	nfound:
		return(0);
	found:
		return(1);
}

static int advance(register char *lp, register char *ep)
{
	register char *curlp;
	char c;
	char *bbeg;
	int ct;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if ((*lp=='\0') || (*lp == '\n'))
			continue;
		return(0);

	case CCEOF:
		return(1);

	case CCL:
		c = *lp++ & 0177;
		if(ep[c>>3] & bittab[c & 07]) {
			ep += 16;
			continue;
		}
		return(0);
	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		if(ecmp(bbeg, lp, ct)) {
			lp += ct;
			continue;
		}
		return(0);

	case CBACK|CSTAR:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		curlp = lp;
		while(ecmp(bbeg, lp, ct))
			lp += ct;
		while(lp >= curlp) {
			if(advance(lp, ep))	return(1);
			lp -= ct;
		}
		return(0);


	case CDOT|CSTAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|CSTAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL|CSTAR:
		curlp = lp;
		do {
			c = *lp++ & 0177;
		} while(ep[c>>3] & bittab[c & 07]);
		ep += 16;
		goto star;

	star:
		if(--lp == curlp) {
			continue;
		}

		if(*ep == CCHR) {
			c = ep[1];
			do {
				if(*lp != c)
					continue;
				if(advance(lp, ep))
					return(1);
			} while(lp-- > curlp);
			return(0);
		}

		do {
			if (advance(lp, ep))
				return(1);
		} while (lp-- > curlp);
		return(0);

	default:
		printe("RE botch\n");
	}
}
static int ecmp(register char *a, register char *b, register int cc)
{
	while(cc--)
		if(*a++ != *b++)	return(0);
	return(1);
}
