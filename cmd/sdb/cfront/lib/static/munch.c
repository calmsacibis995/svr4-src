/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/lib/static/munch.c	1.1"
/* @(#) munch.c 1.8 4/11/89 10:09:08 */
/*ident	"@(#)cfront:lib/static/munch.c	1.8"*/
/*
	scan nm output and detect constructors and destructors for static objects.
	the name on an nm output line is expected to be in the right hand margin.
	the name is expected to be on the form _STD*_ or _STI*_ and less than
	100 characters long.
	nm output lines are assumed to be less than 256 characters long.
	constructors found are called by _main() called from main().
	destructors found are called by exit().
	return 0 if no constructor or destructor is found otherwise.
	The output is redirected by CC into _ctdt.c
	
*/

#include <stdio.h>
#include <ctype.h>
//extern int strcpy(char*, char*);
//extern char * strtok(char*, char*);

struct sbuf {
	sbuf* next;
	char str[1024];
	sbuf(sbuf* l, char* p);
};

sbuf::sbuf(sbuf* l, char* p)
{
	next=l;
	// strcpy(str,strtok(p," |"));
	// ``unrolled'' since strtok() is not on bsd systems
	register char* s = str;
	for (register char c = *p++; c && c!=' ' && c!='|'; c = *p++) *s++ = c;
	*s = 0;	
}

sbuf* dtor;	// list of constructors
sbuf* ctor;	// list of destructors

int read(int, char *, int);

int mygetc()
{
	char c;
	int i = read( 0, &c, 1 );
	if ( i<1 )
		return EOF;
	return c;
}

main ()
{
	char buf[1024];
	register char* p;

newline:
	p = buf;
	for(;;) {
		int c;
		switch (c=mygetc()) {
		case EOF:
			goto done;
		case '\n':
		{	*p = 0;		// terminate string
			p = buf;
			while (*p!='_') if (*p++ == 0) goto newline;
			if (p!=buf && !isspace( *(p-1)) &&
			    *(p-1) != '|')
				goto newline; // '_' not first character
			if (*++p != '_') p--; // accept _ST and __ST
			register char* st = p;
			if (st[0]!='_' || st[1]!='S' || st[2]!='T') goto newline;
			switch (st[3]) {
			case 'D':
				dtor = new sbuf(dtor,st);
				goto newline;
			case 'I':
				ctor = new sbuf(ctor,st);
			default:
				goto newline;
			}
		}
		default:
			*p++ = c;
		}
	}

done:
	int cond = dtor||ctor;

	if (cond == 0) exit(0);

	printf("typedef int (*PFV)();\n");	// "int" to dodge bsd4.2 bug
	if (ctor) {
		for (sbuf* p = ctor; p; p=p->next) printf("int %s();\n",p->str);
		printf("extern PFV _ctors[];\nPFV _ctors[] = {\n");
		for (sbuf* q = ctor; q; q=q->next) printf("\t%s,\n",q->str);
		printf("\t0\n};\n");
	}

	if (dtor) {
		for (sbuf* p = dtor; p; p=p->next) printf("int %s();\n",p->str);
		printf("extern PFV _dtors[];\nPFV _dtors[] = {\n");
		for (sbuf* q = dtor; q; q=q->next) printf("\t%s,\n",q->str);
		printf("\t0\n};\n");
	}

	exit(1);
}

