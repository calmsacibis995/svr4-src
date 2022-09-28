/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*ident	"@(#)libelf:common/demangle.c	1.1"*/
/*
 * C++ Demangler Source Code
 * @(#)master	1.5
 * 7/27/88 13:54:37
 */
#include <ctype.h>
#include <setjmp.h>
#ifdef __STDC__
#	include <stdlib.h>
#else
	extern long strtol();
#endif
#include "elf_dem.h"
#include "String.h"

/* The variable "hold" contains the pointer to the array initially
 * handed to demangle.  It is returned if it is not possible to
 * demangle the string.  NULL is returned if a memory allocation
 * problem is encountered.  Thus one can do the following:
 *
 * char *mn = "Some mangled name";
 * char *dm = mangle(mn);
 * if (dm == NULL)
 *	printf("allocation error\n");
 * else if (dm == mn)
 * 	printf("name could not be demangled\n");
 * else
 *	printf("demangled name is: %s\n",dm);
 */
static char *hold;

/* this String is the working buffer for the demangle
 * routine.  A pointer into this String is returned
 * from demangle when it is possible to demangle the
 * String.  For this reason, the pointer should not
 * be saved between calls of demangle(), nor freed.
 */
static String *s = 0;

static int
getint(c)
char **c;
{
	return strtol(*c, c, 10);
}

/* If a mangled name has a __
 * that is not at the very beginning
 * of the string, then this routine
 * is called to demangle that part
 * of the name.  All overloaded functions,
 * and class members fall into this category.
 *
 * c should start with two underscores followed by a non-zero digit or an F.
 */
static char *
second(c)
char *c;
{
	int n;
	if(strncmp(c,"__",2))
		return hold;
	c += 2;

	if (!(isdigit(*c) || *c == 'F'))
		return hold;

	if (isdigit(*c)) {
		/* a member */
		n = getint(&c);
		if (n == 0 || strlen(c) < n)
			return hold;
		s = prep_String("::",s);
		s = nprep_String(c,s,n);
		c += n;
	}
	if(*c == 'F') {
		/* an overloaded function */
		switch (*++c) {
		case '\0':
			return hold;
		case 'v':
			s = app_String(s,"()");
			break;
		default:
			if(demangle_doargs(&s,c) < 0)
				return hold;
		}
	}
	return PTR(s);
}

char *
demangle(c)
char *c;
{
	register int i = 0;
	extern jmp_buf jbuf;

	if (setjmp(jbuf))
		return 0;

	hold = c;
	s = mk_String(s);
	s = set_String(s,"");

	if(c == 0 || *c == 0)
		return hold;

	if(strncmp(c,"__",2) != 0) {
		/* If a name does not begin with a __
		 * but it does contain one, it is either
		 * a member or an overloaded function.
		 */
		while(c[i] && strncmp(c+i,"__",2))
			i++;
		if (c[i]) {
			/* Advance to first non-underscore */
			while (c[i+2] == '_')
				i++;
		}
		if(strncmp(c+i,"__",2) == 0) {
			/* Copy the simple name */
			s = napp_String(s,c,i);
			/* Process the signature */
			return second(c+i);
		} else
			return hold;
	} else {
		char *x;
		int oplen;
		c += 2;

		/* For automatic variables, or internal static
		 * variables, a __(number) is prepended to the
		 * name.  If this is encountered, strip this off
		 * and return.
		 */
		if(isdigit(*c)) {
			while(isdigit(*c))
				c++;
			return c;
		}

		/* Handle operator functions -- this
		 * automatically calls second, since
		 * all operator functions are overloaded.
		 */
		if(x = findop(c, &oplen)) {
			s = app_String(s,"operator");
			s = app_String(s,x);
			c += oplen;
			return second(c);
		}

		/* Operator cast does not fit the mould
		 * of the other operators.  Its type name
		 * is encoded.  The cast function must
		 * take a void as an argument.
		 */
		if(strncmp(c,"op",2) == 0) {
			int r;
			s = app_String(s,"operator ");
			c += 2;
			r = demangle_doarg(&s,c);
			if(r < 0)
				return hold;
			c += r;
			return second(c);
		}

		/* Constructors and Destructors are also
		 * a special case of operator name.  Note
		 * that the destructor, while overloaded,
		 * must always take the same arguments --
		 * none.
		 */
		if ((*c == 'c' || *c == 'd') && strncmp(c+1,"t__",3) == 0) {
			int n;
			char *c2 = c+2;
			char cx = c[0];
			c += 4;
			n = getint(&c);
			if(n == 0)
				return hold;
			s = napp_String(s,c,n);
			if(cx == 'd')
				s = prep_String("~",s);
			return second(c2);
		}
		return hold;
	}
}
