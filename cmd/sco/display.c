/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:display.c	1.1"

/*
 *	@(#) display.c 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include "defs.h"
#include <ctype.h>

static int base;
static bool beepflag;
static byte comp_key;
static byte *p_dead, *p_comp, *p_seq, *p_str, *p_strbuf;
static byte *inmap, *outmap;

static void input(), output();

display(buf, abase)
byte *buf;
int abase;
{
	int i;

	base = abase;
	inmap = buf;
	outmap = buf+256;
	comp_key = buf[512];
	beepflag = buf[513];
	p_dead = buf+512+1+1+(4*2);
	i = 512+1+1;
	p_comp = buf + GET(i);
	i += 2;
	p_seq = buf + GET(i);
	i += 2;
	p_str = buf + GET(i);
	i += 2;
	p_strbuf = buf + GET(i);

	input();
	output();
	dead();
	compose();
	beep();
}

static void
input()
{
	int i;

	printf("input\n");
	for (i = 0; i < 256; ++i)
		if (inmap[i]) {
			if (inmap[i] != i) {
				show_byte(i, SP);
				show_byte(inmap[i], NL);
			}
		}
	putchar('\n');
}

show_byte(i, c)
int i;
char c;
{
	if (isalnum(i))
		printf("'%c'", i);
	else
		printf((base == 8 )? "0%o":
		       (base == 10)? "%d":
		       (base == 16)? "0x%x":
				     "%d", i);
	putchar(c);
}

static void
output()
{
	int i, j, first, last;
	byte *p;

	printf("output\n");
	for (i = 0; i < 256; ++i) {
		if (outmap[i] == i)
			continue;
		if (outmap[i]) {
			show_byte(i, SP);
			show_byte(outmap[i], NL);
		} else {
			p = p_str;
			/*
			 * the -2 below is necessary because
			 * of the extra terminating entry at the end.
			 */
			while (p < p_strbuf-2 && *p != i)
				p += 2;
			if (*p != i)
				oops("error in buf: %d not in strings\n", i);
			first = *(p+1);
			last = *(p+3) - 1;
			/*
			 * note the -1 above
			 * the ending index of the current key
			 * is one less than the index of the next one.
			 */
			show_byte(i, SP);
			for (j = first; j < last; ++j)
				show_byte(p_strbuf[j], SP);
			show_byte(p_strbuf[last], NL);
		}
	}
	putchar('\n');
}

dead()
{
	byte *p, *q, *last;

	p = p_dead;
	while (p < p_comp) {
		printf("dead ");
		show_byte(*p, NL);
		q = p_seq + 2*(*(p+1));
		last = p_seq + 2*(*(p+3) - 1);
		/*
		 * note the -1 above
		 * the ending index of the current key
		 * is one less than the index of the next one.
		 */
		while (q <= last) {
			show_byte(*q, SP);
			show_byte(*(q+1), NL);
			q += 2;
		}
		p += 2;
		putchar('\n');
	}
}

compose()
{
	byte *p, *q, *last;

	if (!comp_key)
		return;
	printf("compose ");
	show_byte(comp_key, NL);
	p = p_comp;
	while (p < p_seq-2) {			/* note the -2 */
		q = p_seq + 2*(*(p+1));
		last = p_seq + 2*(*(p+3) - 1);	/* note the -1 */
		while (q <= last) {
			show_byte(*p, SP);
			show_byte(*q, SP);
			show_byte(*(q+1), NL);
			q += 2;
		}
		p += 2;
	}
	putchar('\n');
}

beep()
{
	if (beepflag)
		printf("beep\n");
}
