/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/library.h	1.1"
/* library function return value declarations */

#if BSD
#define	strchr	index
#define strrchr	rindex
#undef	tolower		/* BSD toupper and tolower don't test the character */
#undef	toupper
#define	tolower(c)	(islower(c) ? (c) : (c) - 'A' + 'a')	
#define	toupper(c)	(isupper(c) ? (c) : (c) - 'a' + 'A')	
#endif

/* private library */
char	*basename(), *compath(), *egrepinit(), *getwd(), *logdir();
char	*mycalloc(), *mymalloc(), *myrealloc(), *stralloc();
FILE	*mypopen(), *vpfopen();
void	egrepcaseless();

/* standard C library */
char	*ctime(), *getenv(), *mktemp();
char	*strcat(), *strcpy(), *strncpy(), *strpbrk(), *strchr(), *strrchr();
char	*strtok();
long	lseek(), time();
unsigned sleep();
void	exit(), free(), perror(), qsort();
#if BSD
FILE	*popen();	/* not in stdio.h */
#endif

/* Programmer's Workbench library (-lPW) */
char	*regcmp(), *regex();
