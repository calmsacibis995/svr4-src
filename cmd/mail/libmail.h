/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:libmail.h	1.4.3.1"

#include <stdio.h>
#include <maillock.h>
#include "s_string.h"
/* The following typedefs must be used in SVR4 */
#ifdef SVR3
typedef int mode_t;
#else
# include <sys/types.h>
#endif
#if defined(__STDC__) || defined(__cplusplus)
extern	string *abspath(char *path, char *dot, string *to);
extern	int casncmp(char *s1, char *s2, int n);
extern	int copystream(FILE *infp, FILE *outfp);
extern	int delempty(mode_t m, char *mailname);
extern	char *maildomain(void);
extern	int pclosevp(FILE *fp);
extern	FILE *popenvp(char *file, char **argv, char *mode, int resetid);
extern	char **setup_exec(char *s);
extern	char *skipspace(char *p);
extern	int substr(char *string1, char *string2);
extern	void strmove(char *from, char *to);
extern	int systemvp(char *file, char **argv, int resetid);
extern	int trimnl(char *s);
extern	char *Xgetenv(char *env);
extern	char *xgetenv(char *env);
extern	int xsetenv(char *file);
#else
extern	string *abspath();
extern	int casncmp();
extern	int copystream();
extern	int delempty();
extern	char *maildomain();
extern	int pclosevp();
extern	FILE *popenvp();
extern	char **setup_exec();
extern	char *skipspace();
extern	void strmove();
extern	int substr();
extern	int systemvp();
extern	int trimnl();
extern	char *Xgetenv();
extern	char *xgetenv();
extern	int xsetenv();
#endif
