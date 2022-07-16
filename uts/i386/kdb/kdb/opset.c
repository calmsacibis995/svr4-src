/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/opset.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"

typedef long		L_INT;
typedef unsigned long	ADDR;

char *findsymname();

char *sl_name;

/*
 *	UNIX debugger
 *
 *		Instruction printing routines.
 *		MACHINE DEPENDENT.
 *		Tweaked for i386.
 */

/* prassym(): symbolic printing of disassembled instruction */
prassym()
{
	int cnt, jj;
	long value, diff;
	register char *os;
	char *pos;

	extern	char	mneu[];		/* in dis/extn.c */

	/* depends heavily on format output by disassembler */
	cnt = 0;
	os = mneu;	/* instruction disassembled by dis_dot() */
	while(*os != '\t' && *os != ' ' && *os != '\0')
		os++;		/* skip over instr mneumonic */
	while (*os) {
		while(*os == '\t' || *os == ',' || *os == ' ')
			os++;
		value = jj = 0;
		pos = os;
		switch (*os) {
		/*
		** This counts on disassembler not lying about
		** lengths of displacements.
		*/
		case '$':
			pos++;
			/* fall through */

		case '0':
			value = strtol(pos,&pos,0);
			jj = (value != 0);
			if (*pos != '(')
				break;
			/* fall through */

		case '(':
			while (*pos != ')')
				pos++;
			os = pos;
			break;

		case '+':
		case '-':
			while(*os != '\t' && *os != ' ' && *os != '\0')
				os++;
			if ((os[0] == ' ' || os[0] == '\t') && os[1] == '<') {
				char *cp;

				value = strtol(os + 2, &cp, 16);
				if (*cp == '>')
					jj = 1;
				os = cp;
			}
			if (value == 0) /* probably a .o, unrelocated displacement*/
				jj = 0;
			break;
		}
		if (jj > 0 && (diff = adrtoext((ADDR) value)) != -1) {
			if (cnt < 0) {
				dbprintf(" [-,");
				cnt = 1;
			} else if (cnt++ == 0)
				dbprintf(" [");
			else
				dbprintf(",");
			dbprintf("%s", sl_name);
			prdiff(diff);
		} else if (cnt > 0)
			dbprintf(",-");
		else
			--cnt;
		while(*os != '\t' && *os != ',' && *os != ' ' && *os != '\0')
			os++;
	} /* for */
	if (cnt > 0)
		dbprintf("]");
}

prdiff(diff) {
	if (diff) {
		dbprintf("+");
		prhex(diff);
	}
}

adrtoext(val)
ulong val;
{
	ulong	addr;
	uint	valid;
	extern char *findsyminfo();

	sl_name = findsyminfo(val, &addr, &valid);
	if (!valid) {
		sl_name = "";
		return -1;
	}
	return(val - addr);
}
