/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*ident	"@(#)libelf:common/String.c	1.2"*/
/*
 * C++ Demangler Source Code
 * @(#)master	1.5
 * 7/27/88 13:54:37
 */
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include "elf_dem.h"
#include "String.h"

/* This code emulates the C++ String package
 * in a crude way.
 */

jmp_buf jbuf;

/* This function will expand the space
 * available to a String so that more data
 * can be appended to it
 */
static String *
grow(s)
String *s;
{
	String *ns;
	int sz = s->sg.max * 2;
	assert(sz > 0);
#ifdef ELF
	if ((ns = (String *) malloc(sz + sizeof(StringGuts)+1)) == NULL)
		longjmp(jbuf, 1);
	memcpy(ns, s, s->sg.max + sizeof(StringGuts)+1);
	free(s);
#else
	if ((ns = (String *)realloc(s,sz + sizeof(StringGuts)+1)) == NULL)
		longjmp(jbuf, 1);
#endif
	ns->sg.max = sz;
	return ns;
}

/* This function will expand the space
 * available to a String so that more data
 * can be prepended to it.
 */
static String *
ror(s,n)
String *s;
int n;
{
	int i;
	assert(s != 0);
	while(s->sg.end + n > s->sg.max)
		s = grow(s);
#ifdef __STDC__
	assert(n >= 0);
	assert(s->sg.end >= s->sg.start);
	memmove(s->data + n, s->data, s->sg.end - s->sg.start);
#else
	for(i = s->sg.end - 1;i >= s->sg.start;i--)
		s->data[i+n] = s->data[i];
#endif
	s->sg.end += n;
	s->sg.start += n;
	s->data[s->sg.end] = 0;
	return s;
}

/* This function will prepend c
 * to s
 */
String *
prep_String(c,s)
char *c;
String *s;
{
	return nprep_String(c,s,ID_NAME_MAX);
}

/* This function will prepend the
 * first n characters of c to s
 */
String *
nprep_String(c,s,n)
char *c;
String *s;
int n;
{
	int len = strlen(c);
	assert(s != 0);
	if(len > n)
		len = n;
	if(len > s->sg.start)
		s = ror(s, len - s->sg.start);
	s->sg.start -= len;
	memcpy(s->data + s->sg.start, c, len);
	return s;
}

/* This function will append
 * c to s.
 */
String *
app_String(s,c)
String *s;
char *c;
{
	return napp_String(s,c,ID_NAME_MAX);
}

/* This function will append the
 * first n characters of c to s
 */
String *
napp_String(s,c,n)
String *s;
char *c;
{
	int len = strlen(c);
	int catlen;
	assert(s != 0);
	if(n < len)
		len = n;
	catlen = s->sg.end + len;
	while(catlen > s->sg.max)
		s = grow(s);
	memcpy(s->data + s->sg.end, c, len);
	s->sg.end += len;
	s->data[s->sg.end] = '\0';
	return s;
}

/* This function initializes a
 * String.  It returns its argument if
 * its argument is non-zero.
 * This prevents the same string
 * from being re-initialized.
 */
String *
mk_String(s)
String *s;
{
	if(s)
		return s;
	s = (String *)malloc(STRING_START + sizeof(StringGuts)+1);
	if (s == NULL)
		longjmp(jbuf, 1);
	s->sg.start = s->sg.end = STRING_START/2;
	s->sg.max = STRING_START;
	s->data[s->sg.end] = '\0';
	return s;
}

void
free_String(s)
String *s;
{
	if(s)
		free(s);
}

/* This function copies
 * c into s.
 * Used for initialization.
 */
String *
set_String(s,c)
String *s;
char *c;
{
	int len = strlen(c)*2;
	while(len > s->sg.max)
		s = grow(s);
	s->sg.start = s->sg.end = s->sg.max / 2;
	s = app_String(s,c);
	return s;
}

/* Chop n characters off the end of a string.
 * Return the truncated string.
 */

String *
trunc_String(s, n)
String *s;
{
	assert(n <= s->sg.end - s->sg.start);
	s->sg.end -= n;
	s->data[s->sg.end] = '\0';
	return s;
}
