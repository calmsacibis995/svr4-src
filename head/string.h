/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _STRING_H
#define _STRING_H

#ident	"@(#)head:string.h	1.7.1.12"

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int	size_t;
#endif

#ifndef NULL
#define NULL	0
#endif

#if defined(__STDC__)

extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *, const char *, size_t);

extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, size_t);

extern int memcmp(const void *, const void *, size_t);
extern int strcmp(const char *, const char *);
extern int strcoll(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern size_t strxfrm(char *, const char *, size_t);

extern void *memchr(const void *, int, size_t);
extern char *strchr(const char *, int);
extern size_t strcspn(const char *, const char *);
	#pragma int_to_unsigned strcspn
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern size_t strspn(const char *, const char *);
	#pragma int_to_unsigned strspn
extern char *strstr(const char *, const char *);
extern char *strtok(char *, const char *);

extern void *memset(void *, int, size_t);
extern char *strerror(int);
extern size_t strlen(const char *);
	#pragma int_to_unsigned strlen

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)

extern void *memccpy(void *, const void *, int, size_t);
#endif

#if __STDC__ == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)

extern char *strdup(const char *);
extern int ffs(const int);

#endif

#else

extern char
	*strcpy(),
	*strncpy(),
	*strcat(),
	*strncat(),
	*strchr(),
	*strrchr(),
	*strpbrk(),
	*strtok(),
	*strdup();
extern int
	strcmp(),
	strncmp();
extern size_t
	strlen(),
	strspn(),
	strcspn();
extern char
	*memccpy(),
	*memchr(),
	*memcpy(),
	*memmove(),
	*memset();
extern int memcmp();

#endif	/* __STDC__ */ 		

#endif	/* _STRING_H */
