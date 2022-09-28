/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/args.c	1.3"
/*
 * C++ Demangler Source Code
 * @(#)master	1.5
 * 7/27/88 13:54:37
 */
#include <ctype.h>
#ifdef __STDC__
#	include <stdlib.h>
#else
	extern long strtol();
#endif
#include "elf_dem.h"
#include "String.h"

/* This structure is used to keep
 * track of pointers to argument
 * descriptions in the mangled string.
 * This is needed for N and T encodings
 * to work.
 */
typedef struct {
	char *list[10];
	int pos;
} Place;

static Place here;

/* Strings and flags needed by the argument demangles.  The declarator
 * is built up in ptr.  Type modifiers are held in the flag fields.
 * The type itself is passed in separately.
 */
typedef struct {
	String *ptr;
	int Sign,Uns,Cons,Vol;
} Arg_Remem;

/* initialize Arg_Remem */
static void
mkar(r)
Arg_Remem *r;
{
	r->ptr = mk_String((String *)0);
	r->Sign = r->Uns = r->Cons = r->Vol = 0;
}

/* free data for Arg_Remem */
static void
delar(r)
Arg_Remem *r;
{
	free_String(r->ptr);
}

/* This routine formats a single argument
 * on the buffer sptr.
 * c is the type or class name, n is its length.
 */
static void
nsetarg(sptr,r,c,n)
String **sptr;
Arg_Remem *r;
char *c;
{
	r->ptr = nprep_String(c, r->ptr, n);
	if(r->Cons)
		r->ptr = prep_String("const ",r->ptr);
	if(r->Vol)
		r->ptr = prep_String("volatile ",r->ptr);
	if(r->Uns)
		r->ptr = prep_String("unsigned ",r->ptr);
	else if(r->Sign)
		r->ptr = prep_String("signed ",r->ptr);
	*sptr = app_String(*sptr,PTR(r->ptr));
	delar(r);
}

/* This routine formats a single argument
 * on the buffer sptr.
 * c is the null terminated type or class name.
 */
static void
setarg(sptr,r,c)
String **sptr;
Arg_Remem *r;
char *c;
{
	nsetarg(sptr,r,c,ID_NAME_MAX);
}


/* Demangle a single function argument.
 * Returns the number of characters processed from c.
 */
int
demangle_doarg(sptr,c)
String **sptr;
char *c;
{
	register int i;
	Arg_Remem ar;
	mkar(&ar);

	if(here.pos < 10 && here.pos >= 0)
		here.list[here.pos++] = c;

	for(i=0;c[i];i++) {
		/* Loop until you find a type.
		   Then call setarg and return.
		*/
		switch(c[i]) {

		case 'T':
			{
				Place tmp;
				tmp = here;
				here.pos = c[1] - '1';
				if(here.pos < 0 || here.pos >= tmp.pos-1) {
					delar(&ar);
					return -1;
				}
				demangle_doarg(sptr,here.list[here.pos]);
				here = tmp;
				delar(&ar);
				return 2;
			}
		case 'N':
			{
				Place tmp;
				int cycles,pos;
				cycles = c[1] - '0'; pos = c[2] - '1';
				here.pos += cycles - 1;
				tmp = here;
				if(cycles <= 1 || cycles > 9 || pos < 0 || pos >= tmp.pos-1) {
					delar(&ar);
					return -1;
				}
				while(cycles--) {
					here = tmp;
					here.pos = pos;
					demangle_doarg(sptr,here.list[here.pos]);
					(*sptr) = app_String(*sptr,",");
				}
				*sptr = trunc_String(*sptr, 1);
				here = tmp;
				delar(&ar);
				return 3;
			}

		/* Qualifiers to type names */
		case 'S':
			ar.Sign = 1;
			break;
		case 'U':
			ar.Uns = 1;
			break;
		case 'C':
			ar.Cons = 1;
			break;
		case 'V':
			ar.Vol = 1;
			break;

		/* Pointers, references, and Member Pointers */
		case 'P':
		case 'R':
		case 'M':
			if(ar.Cons) {
				ar.ptr = prep_String(" const",ar.ptr);
				ar.Cons = 0;
			}
			if(ar.Vol) {
				ar.ptr = prep_String(" volatile",ar.ptr);
				ar.Vol = 0;
			}
			if(c[i] == 'P')
				ar.ptr = prep_String("*",ar.ptr);
			else if(c[i] == 'R')
				ar.ptr = prep_String("&",ar.ptr);
			else {
				int cnt = 0;
				char *s;
				ar.ptr = prep_String("::*",ar.ptr);
				/* Skip over the 'M' */
				i++;
				cnt = strtol(c+i, &s, 10);
				i = s - c;
				ar.ptr = nprep_String(c+i,ar.ptr,cnt);
				ar.ptr = prep_String(" ",ar.ptr);
				i += cnt;
				/* The loop increments i */
				i--;
			}
			break;

		/* Demangle for basic built-in types */
		case 'i':
			setarg(sptr,&ar,"int");
			return i + 1;
		case 'c':
			setarg(sptr,&ar,"char");
			return i + 1;
		case 's':
			setarg(sptr,&ar,"short");
			return i + 1;
		case 'l':
			setarg(sptr,&ar,"long");
			return i + 1;
		case 'f':
			setarg(sptr,&ar,"float");
			return i + 1;
		case 'd':
			setarg(sptr,&ar,"double");
			return i + 1;
		case 'r':
			setarg(sptr,&ar,"long double");
			return i + 1;

		/* Class encodings */
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
			{
				int cnt = 0;
				char *s;
				cnt = strtol(c+i, &s, 10);
				i = s - c;
				if(strlen(c+i) < cnt) {
					delar(&ar);
					return -1;
				}
				nsetarg(sptr,&ar,c+i,cnt);
				return i+cnt;
			}

		/* Ellipsis and void */
		case 'e':
			setarg(sptr,&ar,"...");
			return i + 1;
		case 'v':
			setarg(sptr,&ar,"void");
			return i + 1;

		/* Arrays */
		case 'A':
			if(*PTR(ar.ptr)) {
				ar.ptr = prep_String("(",ar.ptr);
				ar.ptr = app_String(ar.ptr,")");
			}
			ar.ptr = app_String(ar.ptr,"[");
			{
				int cnt = 0;
				i++;
				while(isdigit(c[i+cnt]))
					cnt++;
				ar.ptr = napp_String(ar.ptr,c+i,cnt);
				i += cnt;
				if(c[i] != '_') {
					delar(&ar);
					return -1;
				}
			}
			ar.ptr = app_String(ar.ptr,"]");
			break;

		/* Functions
		 * This will always be called as a pointer
		 * to a function.
		 */
		case 'F':
			ar.ptr = prep_String("(",ar.ptr);
			ar.ptr = app_String(ar.ptr,")");
			{
				Place tmp;
				tmp = here;
				i++;
				i += demangle_doargs(&ar.ptr,c+i);
				if(c[i] != '_') {
					delar(&ar);
					return -1;
				}
				here = tmp;
			}
			break;

		/* Needed when this is called to demangle
		 * an argument of a pointer to a function.
		 */
		case '_':
			delar(&ar);
			return 0;

		default:
			delar(&ar);
			return -1;
		}
	}

	/* Did the argument list terminate properly? */
	{
		int rc = 0;
		if(*PTR(ar.ptr) || ar.Uns || ar.Sign || ar.Cons || ar.Vol)
			rc = -1;
		delar(&ar);
		return rc;
	}
}

/* This function is called to demangle
 * an argument list.
 * Returns the number of characters processed from c.
 */
int
demangle_doargs(sptr,c)
String **sptr;
char *c;
{
	int i,n = 0;
	here.pos = 0;

	*sptr = app_String(*sptr,"(");
	while(*c && (i = demangle_doarg(sptr,c)) > 0) {
		c += i;
		n += i;
		(*sptr) = app_String(*sptr,(*c && *c == 'e') ? " " : ",");
	}

	if(i < 0)
		return -1;

	*sptr = app_String(trunc_String(*sptr, 1),")");

	return n;
}
