/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:trchan.c	1.1"

/*
 *	@(#) trchan.c 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include "sys/types.h"
#include "sys/emap.h"
#include "defs.h"

static byte *buf;
static byte comp_key;
static byte *inmap, *outmap, *p_dead, *p_comp, *p_seq, *p_str, *p_strbuf;
static long count;
bool cont, Dopt;

int convert();
char *malloc();

main(argc, argv)
int argc;
char **argv;
{
	int rc, i, c, state;
	byte *first, *last;
	byte savedc;
	byte *p, d;

	SHIFT;
	cont = FALSE;
	if (argc && strcmp(*argv, "-c") == 0) {
		cont = TRUE;
		SHIFT;
	}
	if (!argc || argc > 1)
		oops("usage: trchan [-c] mapfile\n");
	if ((buf = (byte *)malloc(E_TABSZ)) == NULL)
		oops("not enough memory\n");
	rc = convert(*argv, buf);
	if (rc == BAD_MAP_FILE)
		exit(1);
	else if (rc == NULL_MAP_FILE) {
		while ((c = getchar()) != EOF)
			putchar(c);
		exit(0);
	}
	inmap = buf;
	outmap = buf+256;
	comp_key = buf[512];
	p_dead = buf+512+1+1+(4*2);
	i = 512+1+1;
	p_comp = buf + GET(i);
	i += 2;
	p_seq = buf + GET(i);
	i += 2;
	p_str = buf + GET(i);
	i += 2;
	p_strbuf = buf + GET(i);
	state = 0;
	count = 0;
	while ((c = getchar()) != EOF) {
		++count;
		switch (state) {
		case 0:		/* not in a dead or compose sequence */
			d = inmap[c];
			if (!c || d)
				echo(d);
			else if (c == comp_key)
				state = 2;
			else {
				savedc = c;
				state = 1;
			}
			break;
		case 1:		/* in a dead sequence, dead key is in savedc */
			d = inmap[c];
			if (!d) {
				if (c == comp_key)
					whoa("compose key found in dead key sequence\n");
				else
					whoa("dead key 0x%02x found in dead key sequence\n",
					     c);
				state = (c == comp_key)? 2: 0;
				continue;
			}
			p = p_dead;
			while (p < p_comp && *p < savedc)
				p += 2;
			if (*p != savedc) {
				/*
				 * the following should never happen
				 * if the mapchan file was correctly converted.
				 * ??should we bomb??
				 */
				whoa("unknown dead key: 0x%02x\n", savedc); 
				state = 0;
				continue;
			}
			first = p_seq + 2* (*(p+1));
			last = p_seq + 2* (*(p+3) -1 );
			p = first;
			while (p <= last && *p < d )
				p += 2;
			if (*p != d) {
				whoa("illegal dead key sequence: 0x%02x 0x%02x\n",
				     savedc, c);
				state = 0;
				continue;
			}
			echo(*(p+1));
			state = 0;
			break;
		case 2:		/* saw compose key */
			d = inmap[c];
			if (!d) {
				if (c == comp_key) {
					whoa("compose key found in compose key sequence\n");
					state = 2;
					continue;
				} else
					d = c; /* dead key inside of compose */
			}
			savedc = d;
			state = 3;
			break;
		case 3:     /* in compose key sequence, 1st char is in savedc */
			d = inmap[c];
			if (!d) {
				if (c == comp_key) {
					whoa("compose key found in compose key sequence\n");
					state = 2;
					continue;
				} else
					d = c; /* dead key inside of compose */
			}
			p = p_comp;
			while (p < p_seq-2 && *p < savedc)
				p += 2;
			if (*p != savedc) {
				whoa("unknown 1st compose key: 0x%02x\n", savedc); 
				state = 0;
				continue;
			}
			first = p_seq + 2* (*(p+1));
			last = p_seq + 2* (*(p+3) - 1);
			p = first;
			while (p <= last && *p < d )
				p += 2;
			if (*p != d) {
				whoa("illegal compose key sequence: 0x%02x 0x%02x\n",
				     savedc, c);
				state = 0;
				continue;
			}
			echo(*(p+1));
			state = 0;
			break;
		default:
			oops("unknown state\n");
		} /* end of switch */
	} /* end of while */
}

echo(c)
byte c;
{
	byte d;
	byte *p, *first, *last;

	d = outmap[c];
	if (d || !c) {
		putchar(d);
		return;
	}
	p = p_str;
	while (p < p_strbuf-2 && *p < c)
		p += 2;
	if (*p != c)
		oops("unknown string character: 0x%02x\n", c);
	first = p_strbuf + *(p+1);
	last = p_strbuf + *(p+3) - 1;
	for (p = first; p <= last; ++p)
		putchar(*p);
}

whoa(fmt, args)
char *fmt;
int args;
{
	fprintf(stderr, "char %ld, ", count);
	fprintf(stderr, fmt, &args);
	if (!cont)
		exit(1);
}
