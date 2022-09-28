/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*ident	"@(#)libelf:common/String.h	1.1"*/
/*
 * C++ Demangler Source Code
 * @(#)master	1.5
 * 7/27/88 13:54:37
 */
#define STRING_START 32
#define PTR(S) ((S)->data + (S)->sg.start)

typedef struct {
	int start,end,max;
} StringGuts;

typedef struct {
	StringGuts sg;
	char data[1];
} String;

extern String *prep_String();
extern String *nprep_String();
extern String *app_String();
extern String *napp_String();
extern String *mk_String();
extern void free_String();
extern String *set_String();
extern String *trunc_String();
extern char *findop();
