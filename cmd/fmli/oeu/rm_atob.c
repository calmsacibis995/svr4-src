/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:oeu/rm_atob.c	1.1"
/*
 *  ``atob'' - Convert an ASCII "btoa" file to binary.
 *
 *  If bad chars, or checksums do not match: exit(1) and produce NO output.
 *  Assumes that int is 32 bits.
 *
 *  Author:
 *	Paul Rutter	(philabs!per)
 *	Received from netnews (net.sources) 3-May-84 15:47:06 EDT
 *  Adopted for uOMS:
 *	R. P. Eby	(pegasus!eby)
 *	Modified to work on 16-bit machines 20-Jul-84
 */

#include <stdio.h>

#define	ulong	unsigned long
#define streq(s0, s1)	strcmp(s0, s1) == 0
#define DE(c)		((c) - ' ')

static long	Ceor = 0;
static long	Csum = 0;
static long	Crot = 0;
static long	bcount = 0;
static long	word = 0;
static FILE	*xtmpfile;
static long	Outcount;
static long	Bufcount;
static char	Outbuf[4];
static void	myputc();

int
rm_atob(fp, filenm)
FILE *fp;
char *filenm;
{
	register long c, i;
	char buf[100];
	long n1, n2, oeor, osum, orot;

	Outcount = Bufcount = Ceor = Csum = Crot = bcount = word = 0;
	xtmpfile = fopen(filenm, "w+");
	if (xtmpfile == NULL)
		return(1);
	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, fp) == NULL) {
			fatal();
			return(1);
		}
		if (streq(buf, "xbtoa Begin\n")) {
			break;
		}
	}
	while ((c = getc(fp)) != EOF) {
		if (c == '\n') {
			continue;
		} else if (c == 'x') {
			break;
		} else {
			decode(c);
		}
	}
	if (fscanf(fp, "btoa End N %ld %lx E %lx S %lx R %lx\n", &n1, &n2, &oeor, &osum, &orot) != 5) {
		fatal();
		return(1);
	}
	while (Outcount < n1)
		myputc(' ', xtmpfile);
	if ((n1 != n2) || (oeor != Ceor) || (osum != Csum) || (orot != Crot)) {
		fatal();
		return(1);
	}
	fclose(xtmpfile);
	return(0);
}

fatal()
{
	fclose(xtmpfile);
}

decode(c)
register long c;
{
	if (c == (ulong) 'z') {
		if (bcount != 0) {
			fatal();
		} else {
			byteout(0L);
			byteout(0L);
			byteout(0L);
			byteout(0L);
		}
	} else if ((c >= (ulong) ' ') && (c < (ulong) (' ' + 85))) {
		if (bcount == 0) {
			word = DE(c);
			++bcount;
		} else if (bcount < 4) {
			word *= 85L;
			word += DE(c);
			++bcount;
		} else {
			word = ((ulong) word * (ulong) 85) + DE(c);
			byteout((word >> 24) & 255L);
			byteout((word >> 16) & 255L);
			byteout((word >> 8) & 255L);
			byteout(word & 255L);
			word = 0;
			bcount = 0;
		}
	} else {
		fatal();
	}
}

byteout(c)
register long c;
{

	Ceor ^= c;
	Csum += c;
	Csum += 1;
	if (Crot & 0x80000000L) {
		Crot <<= 1;
		Crot += 1;
	} else {
		Crot <<= 1;
	}
	Crot += c;
	myputc(c, xtmpfile);
}
static void
myputc(c, fp)
int c;
FILE *fp;
{
	int i;

	Bufcount++;
	if (Bufcount > 4) {
		putc(*Outbuf, fp);
		Outcount++;
	}
	for (i = 0; i < 3; i++)
		Outbuf[i] = Outbuf[i + 1];
	Outbuf[3] = c;
}
		
