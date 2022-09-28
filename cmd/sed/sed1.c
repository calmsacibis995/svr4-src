/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sed:sed1.c	1.11"
#include <stdio.h>
#include <ctype.h>
#include "sed.h"
union reptr     *abuf[ABUFSIZE];
union reptr **aptr;
char    ibuf[512];
char    *cbp;
char    *ebp;
char    genbuf[LBSIZE];
char    *lbend;
char	*lcomend;
int     dolflag;
int     sflag;
int     jflag;
int     delflag;
long    lnum;
char    holdsp[LBSIZE+1];
char    *spend;
char    *hspend;
int     nflag;
long    tlno[NLINES];
int     f;
int	numpass;
union reptr     *pending;
char	*trans[040]  = {
	"\\01",
	"\\02",
	"\\03",
	"\\04",
	"\\05",
	"\\06",
	"\\07",
	"-<",
	"->",
	"\n",
	"\\13",
	"\\14",
	"\\15",
	"\\16",
	"\\17",
	"\\20",
	"\\21",
	"\\22",
	"\\23",
	"\\24",
	"\\25",
	"\\26",
	"\\27",
	"\\30",
	"\\31",
	"\\32",
	"\\33",
	"\\34",
	"\\35",
	"\\36",
	"\\37"
};

execute(file)
char *file;
{
	register char *p1, *p2;
	register union reptr	*ipc;
	int	c;
	char	*execp;

	if (file) {
		if ((f = open(file, 0)) < 0) {
			fprintf(stderr, "Can't open %s\n", file);
		}
	} else
		f = 0;

	ebp = ibuf;
	cbp = ibuf;

	if(pending) {
		ipc = pending;
		pending = 0;
		goto yes;
	}

	for(;;) {
		if((execp = gline(linebuf)) == badp) {
			close(f);
			return;
		}
		spend = execp;

		for(ipc = ptrspace; ipc->r1.command; ) {

			p1 = ipc->r1.ad1;
			p2 = ipc->r1.ad2;

			if(p1) {

				if(ipc->r1.inar) {
					if(*p2 == CEND) {
						p1 = 0;
					} else if(*p2 == CLNUM) {
						c = (unsigned char)p2[1];
						if(lnum > tlno[c]) {
							ipc->r1.inar = 0;
							if(ipc->r1.negfl)
								goto yes;
							ipc++;
							continue;
						}
						if(lnum == tlno[c]) {
							ipc->r1.inar = 0;
						}
					} else if(match(p2, 0)) {
						ipc->r1.inar = 0;
					}
				} else if(*p1 == CEND) {
					if(!dolflag) {
						if(ipc->r1.negfl)
							goto yes;
						ipc++;
						continue;
					}

				} else if(*p1 == CLNUM) {
					c = (unsigned char)p1[1];
					if(lnum != tlno[c]) {
						if(ipc->r1.negfl)
							goto yes;
						ipc++;
						continue;
					}
					if(p2)
						ipc->r1.inar = 1;
				} else if(match(p1, 0)) {
					if(p2)
						ipc->r1.inar = 1;
				} else {
					if(ipc->r1.negfl)
						goto yes;
					ipc++;
					continue;
				}
			}

			if(ipc->r1.negfl) {
				ipc++;
				continue;
			}
	yes:
			command(ipc);

			if(delflag)
				break;

			if(jflag) {
				jflag = 0;
				if((ipc = ipc->r2.lb1) == 0) {
					ipc = ptrspace;
					break;
				}
			} else
				ipc++;

		}
		if(!nflag && !delflag) {
			for(p1 = linebuf; p1 < spend; p1++)
				putc(*p1, stdout);
			putc('\n', stdout);
		}

		if(aptr > abuf) {
			arout();
		}

		delflag = 0;

	}
}

match(expbuf, gf)
char	*expbuf;
{
	register char   *p1;

	if(gf) {
		if(expbuf[0])	
			return(0);
		locs = p1 = loc2;
	} else {
		p1 = linebuf;
		locs = 0;
	}
	return(step(p1, expbuf));
}

substitute(ipc)
union reptr	*ipc;
{
	if(match(ipc->r1.re1, 0) == 0)	return(0);

	numpass = 0;
	sflag = 0;		/* Flags if any substitution was made */
	dosub(ipc->r1.rhs, ipc->r1.gfl);

	if(ipc->r1.gfl) {
		while(*loc2) {
			if(match(ipc->r1.re1, 1) == 0) break;
			dosub(ipc->r1.rhs, ipc->r1.gfl);
		}
	}
	return(sflag);
}

dosub(rhsbuf,n)
char	*rhsbuf;
int	n;
{
	register char *lp, *sp, *rp;
	int c;

	if(n > 0 && n < 999)
		{numpass++;
		if(n != numpass) return;
		}
	sflag = 1;
	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while(c = *rp++) {
		if (c == '&') {
			sp = place(sp, loc1, loc2);
			continue;
		} 
		if(c == '\\') {
			c = *rp++;
			if (c >= '1' && c <= '9') {
				sp = place(sp, braslist[c-'1'], braelist[c-'1']);
				continue;
			}
		}
		*sp++ = c;
		if (sp >= &genbuf[LBSIZE]) 
			break;
	}
	lp = loc2;
	loc2 = sp - genbuf + linebuf;
	while (sp < &genbuf[LBSIZE - 1] && (*sp++ = *lp++));
	if (sp >= &genbuf[LBSIZE - 1]) {
		fprintf(stderr, "Output line too long.\n");
		genbuf[LBSIZE - 1] = '\0';
	}
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
	spend = lp-1;
}

char	*place(asp, al1, al2)
char	*asp, *al1, *al2;
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			fprintf(stderr, "Output line too long.\n");
	}
	return(sp);
}

static int col; /* column count for 'l' command */

command(ipc)
union reptr	*ipc;
{
	register int	i;
	register char   *p1, *p2, *p3;
	char   *oldp1, *oldp2;
	int length;
	long int c;
	char	*execp;


	switch(ipc->r1.command) {

		case ACOM:
			*aptr++ = ipc;
			if(aptr >= &abuf[ABUFSIZE]) {
				fprintf(stderr, "Too many appends after line %ld\n",
					lnum);
			}
			*aptr = 0;
			break;

		case CCOM:
			delflag = 1;
			if(!ipc->r1.inar || dolflag) {
				for(p1 = ipc->r1.re1; *p1; )
					putc(*p1++, stdout);
				putc('\n', stdout);
			}
			break;
		case DCOM:
			delflag++;
			break;
		case CDCOM:
			p1 = p2 = linebuf;

			while(*p1 != '\n') {
				if(*p1++ == 0) {
					delflag++;
					return;
				}
			}

			p1++;
			while(*p2++ = *p1++);
			spend = p2-1;
			jflag++;
			break;

		case EQCOM:
			fprintf(stdout, "%ld\n", lnum);
			break;

		case GCOM:
			p1 = linebuf;
			p2 = holdsp;
			while(*p1++ = *p2++);
			spend = p1-1;
			break;

		case CGCOM:
			*spend++ = '\n';
			p1 = spend;
			p2 = holdsp;
			while(*p1++ = *p2++);
			spend = p1-1;
			break;

		case HCOM:
			p1 = holdsp;
			p2 = linebuf;
			while(*p1++ = *p2++);
			hspend = p1-1;
			break;

		case CHCOM:
			*hspend++ = '\n';
			p1 = hspend;
			p2 = linebuf;
                        if((hspend + strlen(p2)) > &holdsp[LBSIZE]){
                                fprintf(stderr,"sed: hold space overflow !\n");
                                exit(1);
                        }
			while(*p1++ = *p2++);
			hspend = p1-1;
			break;

		case ICOM:
			for(p1 = ipc->r1.re1; *p1; )
				putc(*p1++, stdout);
			putc('\n', stdout);
			break;

		case BCOM:
			jflag = 1;
			break;


		case LCOM:
			p1 = linebuf;
			p2 = genbuf;
			col = 1;
			while(*p1) {
				if((unsigned char)*p1 >= 040) {
					int width;
					length = mbtowc(&c, p1, MULTI_BYTE_MAX);
					if(length < 0 || (width = scrwidth(c)) == 0) { /* unprintable bytes */
						if(length < 0)
							length = 1;
						while(length--) {
							*p2++ = '\\';
							if(++col >= 72) {
								*p2++ = '\\';
								*p2++ = '\n';
								col = 1;
							}
							*p2++ = (((int)(unsigned char)*p1 >> 6) & 03) + '0';
							if(++col >= 72) {
								*p2++ = '\\';
								*p2++ = '\n';
								col = 1;
							}
							*p2++ = (((int)(unsigned char)*p1 >> 3) & 07) + '0';
							if(++col >= 72) {
								*p2++ = '\\';
								*p2++ = '\n';
								col = 1;
							}
							*p2++ = ((unsigned char)*p1++ & 07) + '0';
							if(++col >= 72) {
								*p2++ = '\\';
								*p2++ = '\n';
								col = 1;
							}
						}
					} else {
						col += width;
						if(col >= 72) {
					/*
					 * print out character on current
					 * line if it doesn't go
					 * go past column 71
					 */
							if(col == 72)
								while(length) {
									*p2++ = *p1++;
									length--;
								}
							*p2++ = '\\';
							*p2++ = '\n';
							col = 1;
						}
						while(length--)
							*p2++ = *p1++;
					}
				} else {
					p3 = trans[(unsigned char)*p1-1];
					while(*p2++ = *p3++)
						if(++col >= 72) {
							*p2++ = '\\';
							*p2++ = '\n';
							col = 1;
						}
					p2--;
					p1++;
				}
				if(p2 >= lcomend) {
					*p2 = '\0';
					fprintf(stdout, "%s", genbuf);
					p2 = genbuf;
				}
			}
			*p2 = 0;
			fprintf(stdout, "%s\n", genbuf);
			break;

		case NCOM:
			if(!nflag) {
				for(p1 = linebuf; p1 < spend; p1++)
					putc(*p1, stdout);
				putc('\n', stdout);
			}

			if(aptr > abuf)
				arout();
			if((execp = gline(linebuf)) == badp) {
				pending = ipc;
				delflag = 1;
				break;
			}
			spend = execp;

			break;
		case CNCOM:
			if(aptr > abuf)
				arout();
			*spend++ = '\n';
			if((execp = gline(spend)) == badp) {
				pending = ipc;
				delflag = 1;
				break;
			}
			spend = execp;
			break;

		case PCOM:
			for(p1 = linebuf; p1 < spend; p1++)
				putc(*p1, stdout);
			putc('\n', stdout);
			break;
		case CPCOM:
	cpcom:
			for(p1 = linebuf; *p1 != '\n' && *p1 != '\0'; )
				putc(*p1++, stdout);
			putc('\n', stdout);
			break;

		case QCOM:
			if(!nflag) {
				for(p1 = linebuf; p1 < spend; p1++)
					putc(*p1, stdout);
				putc('\n', stdout);
			}
			if(aptr > abuf) arout();
			fclose(stdout);
			exit(0);
		case RCOM:

			*aptr++ = ipc;
			if(aptr >= &abuf[ABUFSIZE])
				fprintf(stderr, "Too many reads after line%ld\n",
					lnum);

			*aptr = 0;

			break;

		case SCOM:
			i = substitute(ipc);
			if(ipc->r1.pfl && nflag && i)
				if(ipc->r1.pfl == 1) {
					for(p1 = linebuf; p1 < spend; p1++)
						putc(*p1, stdout);
					putc('\n', stdout);
				}
				else
					goto cpcom;
			if(i && ipc->r1.fcode)
				goto wcom;
			break;

		case TCOM:
			if(sflag == 0)  break;
			sflag = 0;
			jflag = 1;
			break;

		wcom:
		case WCOM:
			fprintf(ipc->r1.fcode, "%s\n", linebuf);
			break;
		case XCOM:
			p1 = linebuf;
			p2 = genbuf;
			while(*p2++ = *p1++);
			p1 = holdsp;
			p2 = linebuf;
			while(*p2++ = *p1++);
			spend = p2 - 1;
			p1 = genbuf;
			p2 = holdsp;
			while(*p2++ = *p1++);
			hspend = p2 - 1;
			break;

		case YCOM: 
			p1 = linebuf;
			p2 = ipc->r1.re1;
			if(!multibyte)
				while(*p1 = p2[(unsigned char)*p1])
					p1++;
			else {
				char *ep;
				wchar_t c, d;
				int length;
				ep = ipc->r1.re1;
				p3 = genbuf;
				
				length = mbtowc(&c, p1, MULTI_BYTE_MAX);
				while(length) {
					char multic[MULTI_BYTE_MAX];
					if(length == -1) {
						if(p3 + 1 > &genbuf[LBSIZE]) {
							fprintf(stderr, "Output line too long.\n");
							break;
						}
						*p3++ = *p1++;
						length = mbtowc(&c, p1, MULTI_BYTE_MAX);
						continue;
					}
					p1 += length;
					if(c <= 0377 && ep[c] != 0) 
						d = (unsigned char)ep[c];
					else {
						p2 = ep + 0400;
						while(1) {
							length = mbtowc(&d, p2, MULTI_BYTE_MAX); 
							p2 += length;
							if(length == 0 || d == c)
								break;
							p2 += mbtowc(&d, p2, MULTI_BYTE_MAX);
						}
						if(length == 0)
							d = c;
						else
							(void)mbtowc(&d, p2, MULTI_BYTE_MAX);
					}
					length = wctomb(multic, d);
					if(p3 + length > &genbuf[LBSIZE]) {
						fprintf(stderr, "Output line too long.\n");
						break;
					}
					(void)strncpy(p3, multic, length);
					p3 += length;
					length = mbtowc(&c, p1, MULTI_BYTE_MAX);
				}
				*p3 = '\0';
				p3 = genbuf;
				p1 = linebuf;
				while(*p1++ = *p3++);
				spend = p1 - 1;
			}
			break;
	}

}

char	*gline(addr)
char	*addr;
{
	register char   *p1, *p2;
	register	c;
	p1 = addr;
	p2 = cbp;
	for (;;) {
		if (p2 >= ebp) {
			if ((c = read(f, ibuf, 512)) <= 0) {
				return(badp);
			}
			p2 = ibuf;
			ebp = ibuf+c;
		}
		if ((c = *p2++) == '\n') {
			if(p2 >=  ebp) {
				if((c = read(f, ibuf, 512)) <= 0) {
					close(f);
					if(eargc == 0)
							dolflag = 1;
				}

				p2 = ibuf;
				ebp = ibuf + c;
			}
			break;
		}
		if(c)
		if(p1 < lbend)
			*p1++ = c;
	}
	lnum++;
	*p1 = 0;
	cbp = p2;

	return(p1);
}

char *comple(ep, x3, x4)
char *x3;
wchar_t x4;
register char **ep;
{
	register char *pcp, *p;
	wchar_t c;
	register int length;
	int cclass = 0;

	pcp = cp;
	length = mbtowc(&c, pcp, MULTI_BYTE_MAX);
	while(length > 0) {
		if(c == x4 && !cclass)
			break;
		if(c == '\n')
			return(badp);
		if(cclass && c == ']') {
			cclass = 0;
			continue;
		}
		if(c == '[' && !cclass) {
			cclass = 1;
			pcp += length;
			if((length = mbtowc(&c, pcp, MULTI_BYTE_MAX)) <= 0 || c == '\n')
				return(badp);
		}
		if(c == '\\' && !cclass) {
			pcp += length;
			if((length = mbtowc(&c, pcp, MULTI_BYTE_MAX)) <= 0 || c == '\n')
				return(badp);
		}
		pcp += length;
		length = mbtowc(&c, pcp, MULTI_BYTE_MAX);
	}
	if(length <= 0)
		return(badp);
	c = (unsigned char)*pcp;
	*pcp = '\0';
	p = compile(cp, *ep, reend);
	while(regerrno == 50) {
		char *p2;
		int size;
		size = reend - *ep;
		if(*ep == respace)
			p2 = realloc(respace, size + RESIZE);
		else
			p2 = malloc(size + RESIZE);
		if(p2 == (char *)0) {
			fprintf(stderr, TMMES, linebuf);
			exit(2);
		}
		respace = *ep = p2;
		reend = p2 + size + RESIZE - 1;
		p = compile(cp, *ep, reend);
	}
	*pcp = c;
	cp = pcp + length;
	if(regerrno && regerrno != 41)
		return(badp);
	if(regerrno == 41)
		return(*ep);
	return(p);
}

arout()
{
	register char   *p1;
	FILE	*fi;
	char	c;
	int	t;

	aptr = abuf - 1;
	while(*++aptr) {
		if((*aptr)->r1.command == ACOM) {
			for(p1 = (*aptr)->r1.re1; *p1; )
				putc(*p1++, stdout);
			putc('\n', stdout);
		} else {
			if((fi = fopen((*aptr)->r1.re1, "r")) == NULL)
				continue;
			while((t = getc(fi)) != EOF) {
				c = t;
				putc(c, stdout);
			}
			fclose(fi);
		}
	}
	aptr = abuf;
	*aptr = 0;
}

