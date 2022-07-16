#ident	"@(#)utls.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/utls.c	1.3.1.1"
/* @(#)dis_utls.c	10.2 */


#include "sys/types.h"
#include "dis.h"
#include "structs.h"
#include "../db_as.h"

#define		BADADDR	-1L	/* used by the resynchronization	*/
				/* function to indicate that a restart	*/
				/* candidate has not been found		*/

/*
 *	compoff (lng, temp)
 *
 *	This routine will compute the location to which control is to be
 *	transferred.  'lng' is the number indicating the jump amount
 *	(already in proper form, meaning masked and negated if necessary)
 *	and 'temp' is a character array which already has the actual
 *	jump amount.  The result computed here will go at the end of 'temp'.
 *	(This is a great routine for people that don't like to compute in
 *	hex arithmetic.)
 */

compoff(lng, temp)
long	lng;
char	*temp;
{
	extern	as_addr_t	loc;	/* from _extn.c */

	lng += loc.a_addr;
	sprintf(temp,"%s <%lx>",temp,lng);
}
/*
 *	convert (num, temp, flag)
 *
 *	Convert the passed number to hex
 *	leaving the result in the supplied string array.
 *	If  LEAD  is specified, precede the number with '0x' to
 *	indicate the base (used for information going to the mnemonic
 *	printout).  NOLEAD  will be used for all other printing (for
 *	printing the offset, object code, and the second byte in two
 *	byte immediates, displacements, etc.) and will assure that
 *	there are leading zeros.
 */

convert(num,temp,flag)
unsigned	num;
char	temp[];
int	flag;

{

	if (flag == NOLEAD) 
		sprintf(temp,"%4x",num);
	if (flag == LEAD)
		sprintf(temp,"0x%x",num);
	if (flag == NOLEADSHORT)
		sprintf(temp,"%2x",num);
}

/*
 *	fetchbyte ()
 *
 *	read a byte, mask it, then return the result in 'curbyte'.
 *	The getting of all single bytes is done here.  The 'fetchbyte[s]'
 *	routines are the only place where the global variable 'loc'
 *	is incremented.
 */
int
fetchbyte()
{
	extern	as_addr_t	loc;		/* from _extn.c */
	extern	unsigned short curbyte;		/* from _extn.c */
	char	temp[NCPS+1];
	char	byte;

	byte = chkget(loc);
	loc.a_addr++;
	curbyte = byte & 0377;
	convert(curbyte, temp, NOLEADSHORT);
}


/*
 *	lookbyte ()
 *
 *	read a byte, mask it, then return the result in 'curbyte'.
 *	loc is incremented.
 */

int
lookbyte()
{
	extern	as_addr_t	loc;		/* from _extn.c */
	extern	unsigned short curbyte;		/* from _extn.c */
	char	byte;

	byte = chkget(loc);
	loc.a_addr++;
	curbyte = byte & 0377;
}

/*
 *	printline ()
 *
 *	Print the disassembled line, consisting of the object code
 *	and the mnemonics.  The breakpointable line number, if any,
 *	has already been printed.
 */

printline()
{
	extern	char	mneu[];
	dbprintf("%s",mneu); /* to print hex */
}

/*
 *	chkget = get byte
 *
 *	Get the byte at `addr' if it is a legal address.
 */

chkget(addr)
	as_addr_t	addr;
{
	unsigned char	byt;

	if (db_read(addr, (char *)&byt, 1) == -1)
		return -1;
	return byt;
}
/*
 * scaled down version of sprintf.  Only handles %s and %x
 */
char *gcp;

sprintf(cp, fmt, arg)
char *cp, *fmt, *arg;
{
	char **ap, *str, c;
	long l;
	int prec;

	ap = &arg;
	while (c = *fmt)
		switch (*fmt++) {

		case '\\':
			switch (*fmt++) {

			case 'n':
				*cp++ = '\n';
				break;
			case '0':
				*cp++ = '\0';
				break;
			case 'b':
				*cp++ = '\b';
				break;
			case 't':
				*cp++ = '\t';
				break;
			default:
				cp++;

			}
			break;
		case '%':
			prec = 0;
again:
			switch (*fmt++) {

			case '%':
				*cp++ = '%';
				break;
			case 's':
				str = *ap++;
				while (*str) {
					prec--;
					*cp++ = *str++;
				}
				while (prec-- > 0)
					*cp++ = ' ';
				break;
			case 'x':
				l = *(long *)ap;
				ap++;
				gcp = cp;
				hex(l, prec);
				cp = gcp;
				break;
			case '0':	case '1':	case '2':
			case '3':	case '4':	case '5':
			case '6':	case '7':	case '8':
			case '9':
				prec = (prec * 10) + (*(fmt - 1) - '0');
			case '-':
			case 'l':
			case '.':
				goto again;
			default:
				break;

			}
			break;
		default:
			*cp++ = c;
			break;

		}
	*cp = '\0';
	return;
}
hex(l, n)
unsigned long l;
int n;
{

	if (l > 15 || n > 1)
		hex(l >> 4, n - 1);
	*gcp++ = "0123456789abcdef"[l & 0xf];
	return;
}
/*
 * print out hex number, WITH SIGN
 */
prhex(l)
int l;
{
	char buf[10];

	if (l < 0) {
		l = -l;
		if (l < 0)
			sprintf(buf,"0/0");
		else
			sprintf(buf, "-%x", l);
	} else
		sprintf(buf, "%x", l);
	dbprintf("%s", buf);
	return;
}

char *
strchr(cp, c)
char *cp, c;
{

	while (*cp)
		if (*cp == c)
			return(cp);
		else
			cp++;
	return((char *)0);
}

strtol(sp, cp, base)
char *sp, **cp;
int base;
{
	register int n;
	char c;

	if (base == 0)
		base = 10;
	n = 0;
	if (*sp == '0') {
		sp++;
		base = 8;
		if (*sp == 'x') {
			sp++;
			base = 16;
		}
	}
	while ((*sp >= '0' && *sp <= '9') ||
	       (*sp >= 'a' && *sp <= 'f')) {
		if ((c = *sp++) >= 'a' && c <= 'f')
			c -= 'a' - '9' - 1;
		n = (n * base) + (c - '0');
	}
	if (cp)
		*cp = sp;
	return(n);
}
