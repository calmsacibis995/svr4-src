/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:print.c	1.12.8.1"
/*
 * UNIX shell
 *
 */

#include	"defs.h"
#include	<sys/param.h>

#define		BUFLEN		256

unsigned char numbuf[12];

static unsigned char buffer[BUFLEN];
static unsigned char *bufp = buffer;
static int index = 0;
static int buffd = 1;

extern void	prc_buff();
extern void	prs_buff();
extern void	prn_buff();
extern void	prs_cntl();
extern void	prn_buff();

/*
 * printing and io conversion
 */
prp()
{
	if ((flags & prompt) == 0 && cmdadr)
	{
		prs_cntl(cmdadr);
		prs(colon);
	}
}

prs(as)
unsigned char	*as;
{
	register unsigned char	*s;

	if (s = as)
		write(output, s, length(s) - 1);
}

prc(c)
unsigned char	c;
{
	if (c)
		write(output, &c, 1);
}

prt(t)
long	t;
{
	register int hr, min, sec;

	t += HZ / 2;
	t /= HZ;
	sec = t % 60;
	t /= 60;
	min = t % 60;

	if (hr = t / 60)
	{
		prn_buff(hr);
		prc_buff('h');
	}

	prn_buff(min);
	prc_buff('m');
	prn_buff(sec);
	prc_buff('s');
}

prn(n)
	int	n;
{
	itos(n);

	prs(numbuf);
}

itos(n)
{
	register unsigned char *abuf;
	register unsigned a, i;
	int pr, d;

	abuf = numbuf;

	pr = FALSE;
	a = n;
	for (i = 10000; i != 1; i /= 10)
	{
		if ((pr |= (d = a / i)))
			*abuf++ = d + '0';
		a %= i;
	}
	*abuf++ = a + '0';
	*abuf++ = 0;
}

stoi(icp)
unsigned char	*icp;
{
	register unsigned char	*cp = icp;
	register int	r = 0;
	register unsigned char	c;

	while ((c = *cp, digit(c)) && c && r >= 0)
	{
		r = r * 10 + c - '0';
		cp++;
	}
	if (r < 0 || cp == icp)
		failed(icp, badnum);
	else
		return(r);
}

int
ltos(n)
long n;
{
	int i;

	numbuf[11] = '\0';
	for (i = 10; i >= 0; i--) {
		numbuf[i] = n % 10 + '0';
		if ((n /= 10) == 0)
			break;
	}
	return i;
}

prl(n)
long n;
{
	int i;
	i = ltos(n);
	prs_buff(&numbuf[i]);
}

void
flushb()
{
	if (index)
	{
		bufp[index] = '\0';
		write(buffd, bufp, length(bufp) - 1);
		index = 0;
	}
}

void
prc_buff(c)
	unsigned char c;
{
	if (c)
	{
		if (buffd != -1 && index + 1 >= BUFLEN)
			flushb();

		bufp[index++] = c;
	}
	else
	{
		flushb();
		write(buffd, &c, 1);
	}
}

void
prs_buff(s)
	unsigned char *s;
{
	register int len = length(s) - 1;

	if (buffd != -1 && index + len >= BUFLEN)
		flushb();

	if (buffd != -1 && len >= BUFLEN)
		write(buffd, s, len);
	else
	{
		movstr(s, &bufp[index]);
		index += len;
	}
}

unsigned char *octal(c, ptr)
unsigned char c;
unsigned char *ptr;
{
	*ptr++ = '\\';
	*ptr++ = ((unsigned int)c >> 6) + '0';
	*ptr++ = (((unsigned int)c >> 3) & 07) + '0';
	*ptr++ = (c & 07) + '0';
	return(ptr);
}

void
prs_cntl(s)
unsigned char *s;
{
	register int n;
	wchar_t l;
	unsigned char *olds = s;
	register unsigned char *ptr = bufp;
	register wchar_t c;
	n = mbtowc(&l, (const char *)s, MULTI_BYTE_MAX);
	while(n != 0)
	{
		if(n < 0)
			ptr = octal(*s++, ptr);
		else { 
			c = l;
			s += n;
			if(!wisprint(c))
			{
				if(c < '\040' && c > 0)
				{
				/*
			 	 * assumes ASCII char
			 	 * translate a control character 
			 	 * into a printable sequence 
			 	 */
					*ptr++ = '^';
					*ptr++ = (c + 0100);	
				}
				else if (c == 0177) 
				{	/* '\0177' does not work */
					*ptr++ = '^';
					*ptr++ = '?';
				}
				else    /* unprintable 8-bit byte sequence 
				 	 * assumes all legal multibyte
				 	 * sequences are 
				 	 * printable 
				 	 */
					ptr = octal(*olds, ptr);
			}
			else
				while(n--)
					*ptr++ = *olds++;
		}
		if(buffd != -1 && ptr >= &bufp[BUFLEN-4]) {
			*ptr='\0';
			prs(bufp);
			ptr = bufp;
		}
		olds = s;
		n = mbtowc(&l, (const char *)s, MULTI_BYTE_MAX);
	}
	*ptr = '\0';
	prs(bufp);
}

void
prl_buff(l)
long	l;
{
	prs_buff(&numbuf[ltos(l)]);
}

void
prn_buff(n)
int	n;
{
	itos(n);

	prs_buff(numbuf);
}

prsp_buff(cnt)
{
	while (cnt--)
		prc_buff(SP);
}

int
setb(fd)
int fd;
{
	int ofd;

	if ((ofd = buffd) == -1) {
		if (bufp[index-1])
			bufp[index++] = 0;
		endstak(bufp+index);
	} else
		flushb();
	if ((buffd = fd) == -1)
		bufp = locstak();
	else
		bufp = buffer;
	index = 0;
	return ofd;
}

