/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/string.h	1.1"
/*ident	"@(#)cfront:incl/string.h	1.6"*/

#ifndef C_STRINGH
#define C_STRINGH

extern char
	*strcpy(char*, const char*),
	*strncpy(char*, const char*, int),
	*strcat(char*, const char*),
	*strncat(char*, const char*, int),
	*strchr(const char*, char),
	*strrchr(const char*, char),
	*strpbrk(const char*, const char*),
	*strtok(char*, const char*),
	*strdup(const char*);
extern int
	strcmp(const char*, const char*),
	strncmp(const char*, const char*, int),
	strlen(const char*),
	strspn(const char*, const char*),
	strcspn(const char*, const char*);
extern void
	*memccpy(void *, const void *, int, int),
	*memchr(const void *, int, int),
	*memcpy(void *, const void *, int),
	*memset(void *, int, int);
extern int memcmp(const void *, const void *, int);

#endif

