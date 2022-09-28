/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/size.c	1.1"
/*ident	"@(#)cfront:src/size.c	1.8" */
/*********************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


size.c:

	initialize alignment and sizeof "constants"

**********************************************************************/

#include "cfront.h"
#include "size.h"

int BI_IN_WORD = DBI_IN_WORD;
int BI_IN_BYTE = DBI_IN_BYTE;
int SZ_CHAR = DSZ_CHAR;
int AL_CHAR = DAL_CHAR;
int SZ_SHORT = DSZ_SHORT;
int AL_SHORT = DAL_SHORT;
int SZ_INT = DSZ_INT;
int AL_INT = DAL_INT;
int SZ_LONG = DSZ_LONG;
int AL_LONG = DAL_LONG;
int SZ_FLOAT = DSZ_FLOAT;
int AL_FLOAT = DAL_FLOAT;
int SZ_DOUBLE = DSZ_DOUBLE;
int AL_DOUBLE = DAL_DOUBLE;
int SZ_STRUCT = DSZ_STRUCT;
int AL_STRUCT = DAL_STRUCT;
//int SZ_FRAME = DSZ_FRAME;
//int AL_FRAME = DAL_FRAME;
int SZ_WORD = DSZ_WORD;
int SZ_WPTR = DSZ_WPTR;
int AL_WPTR = DAL_WPTR;
int SZ_BPTR = DSZ_BPTR;
int AL_BPTR = DAL_BPTR;
//int SZ_TOP = DSZ_TOP;
//int SZ_BOTTOM = DSZ_BOTTOM;
char* LARGEST_INT = DLARGEST_INT;
int F_SENSITIVE = DF_SENSITIVE;
int F_OPTIMIZED =  DF_OPTIMIZED;

int arg1 = 0;
int arg2 = 0;

int get_line(FILE* fp)
{
	char s[32];

        // fscanf returning 0 rather than EOF reading align file in some instances! 
	if (fscanf(fp," %s %d %d",s,&arg1,&arg2) <= 0) return 0;

	if (strcmp("char",s) == 0) {
		SZ_CHAR = arg1;
		AL_CHAR = arg2;
		return 1;
	}
	if (strcmp("short",s) == 0) {
		SZ_SHORT = arg1;
		AL_SHORT = arg2;
		return 1;
	}
	if (strcmp("int",s) == 0) {
		SZ_INT = arg1;
		AL_INT = arg2;
		if (fscanf(fp," %s",s) <= 0) return 0;
		LARGEST_INT = new char[strlen(s)+1];
		strcpy(LARGEST_INT,s);
		return 1;
	}
	if (strcmp("long",s) == 0) {
		SZ_LONG = arg1;
		AL_LONG = arg2;
		return 1;
	}
	if (strcmp("float",s) == 0) {
		SZ_FLOAT = arg1;
		AL_FLOAT = arg2;
		return 1;
	}
	if (strcmp("double",s) == 0) {
		SZ_DOUBLE = arg1;
		AL_DOUBLE = arg2;
		return 1;
	}
	if (strcmp("bit",s) == 0) {
		BI_IN_BYTE = arg1;
		BI_IN_WORD = arg2;
		return 1;
	}
	if (strcmp("struct",s) == 0) {
		SZ_STRUCT = arg1;
		AL_STRUCT = arg2;
		return 1;
	}
	if (strcmp("struct2",s) == 0) {
		F_SENSITIVE = arg1;
		F_OPTIMIZED = arg2;
		return 1;
	}
	if (strcmp("frame",s) == 0) {
//		SZ_FRAME = arg1;
//		AL_FRAME = arg2;
		return 1;
	}
	if (strcmp("word",s) == 0) {
		SZ_WORD = arg1;
		return 1;
	}
	if (strcmp("wptr",s) == 0) {
		SZ_WPTR = arg1;
		AL_WPTR = arg2;
		return 1;
	}
	if (strcmp("bptr",s) == 0) {
		SZ_BPTR = arg1;
		AL_BPTR = arg2;
		return 1;
	}
	if (strcmp("top",s) == 0) {
//		SZ_TOP = arg1;
//		SZ_BOTTOM = arg2;
		return 1;
	}
	return 0;
}

extern int read_align(char* f)
{
	FILE* fp = fopen(f,"r");
	if (fp == 0) return 1;
	while (get_line(fp)) ;
	return 0;
}

extern print_align(char* s)
{
	fprintf(stderr,"%s sizes and alignments\n\n",s);

	fprintf(stderr,"	size	align	largest\n");
	fprintf(stderr,"char	%d	%d\n",SZ_CHAR,AL_CHAR);
	fprintf(stderr,"short	%d	%d\n",SZ_SHORT,AL_SHORT);
	fprintf(stderr,"int	%d	%d	%s\n",SZ_INT,AL_INT,LARGEST_INT);
	fprintf(stderr,"long	%d	%d\n",SZ_LONG,AL_LONG);
	fprintf(stderr,"float	%d	%d\n",SZ_FLOAT,AL_FLOAT);
	fprintf(stderr,"double	%d	%d\n",SZ_DOUBLE,AL_DOUBLE);
	fprintf(stderr,"bptr	%d	%d\n",SZ_BPTR,AL_BPTR);
	fprintf(stderr,"wptr	%d	%d\n",SZ_WPTR,AL_WPTR);
	fprintf(stderr,"struct	%d	%d\n",SZ_STRUCT,AL_STRUCT);
	fprintf(stderr,"struct2	%d	%d\n",F_SENSITIVE,F_OPTIMIZED);
//	fprintf(stderr,"frame	%d	%d\n",SZ_FRAME,AL_FRAME);

	fprintf(stderr,"%d bits in a byte, %d bits in a word, %d bytes in a word\n",
			BI_IN_BYTE, BI_IN_WORD, SZ_WORD);
	return 1;
}


int c_strlen(const char* s)
/*
	return sizeof(s) with escapes processed
	sizeof("") == 1		the terminating 0
	sizeof("a") == 2
	sizeof("\0x") == 3	0 x 0
	sizeof("\012") == 2	'\012'
	sizeof("\01")		'\001'
	sizeof("\x") == 2 	\ ignored

*/
{
	int i = 1;
        for (const char* p = s; *p; i++,p++) {
		if (*p == '\\')	{					// '\?
			switch (*++p) {
			case '0':
				switch (p[1]) {		// '\01' or '\012'
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					break;
				default:
					continue;	// '\0'
				}
				/* no break */
			case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':		// '\123'
				switch (*++p) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					switch (*++p) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						break;
					default:
						--p;
					}
					break;
				default:
					--p;
				}
			}
		}
	}
	return i;
}
