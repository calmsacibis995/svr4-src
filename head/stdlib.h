/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _STDLIB_H
#define _STDLIB_H

#ident	"@(#)head:stdlib.h	1.23"

typedef	struct {
	 int	quot;
	 int	rem;
	} div_t;

typedef struct {
	 long	quot;
	 long	rem;
	} ldiv_t;

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int    size_t;
#endif

#ifndef NULL
#define NULL            0
#endif

#define EXIT_FAILURE	1
#define EXIT_SUCCESS    0
#define RAND_MAX	32767

#ifndef _WCHAR_T
#define _WCHAR_T
typedef long wchar_t;
#endif

#if defined(__STDC__)

extern unsigned char 	__ctype[];

#define MB_CUR_MAX	(int)__ctype[520]

extern double atof(const char *);
extern int atoi(const char *);
extern long int atol(const char *);
extern double strtod(const char *, char **);
extern long int strtol(const char *, char **, int);
extern unsigned long int strtoul(const char *, char **, int );

extern int rand(void);
extern void srand(unsigned int);

extern void *calloc(size_t, size_t);
extern void free(void *);
extern void *malloc(size_t);
extern void *realloc(void *, size_t);

extern void abort(void);
extern int atexit(void (*)(void));
extern void exit(int);
extern char *getenv(const char *);
extern int system(const char *);

extern void *bsearch(const void *, const void *, size_t, size_t,
	int (*)(const void *, const void *));
extern void qsort(void *, size_t, size_t,
	int (*)(const void *, const void *));

extern int abs(int);
extern div_t div(int, int);
extern long int labs(long);
extern ldiv_t ldiv(long, long);

extern int mbtowc(wchar_t *, const char *, size_t);
extern int mblen(const char *, size_t);
extern int wctomb(char *, wchar_t);

extern size_t mbstowcs(wchar_t *, const char *, size_t);
extern size_t wcstombs(char *, const wchar_t *, size_t);

#if __STDC__ == 0 && !defined(_POSIX_SOURCE) || !defined(_XOPEN_SOURCE)

extern long a64l(const char *);
extern int dup2(int, int);
extern char *ecvt(double, int, int *, int *);
extern char *fcvt(double, int, int *, int *);
extern char *getcwd(char *, int);
extern char *getlogin(void);
extern int getopt(int, char *const *, const char *);
extern int getsubopt(char **, char *const *, char **);
extern char *optarg;
extern int optind, opterr, optopt;
extern char *getpass(const char *);
extern int getpw(int, char *);
extern char *gcvt(double, int, char *);
extern int isatty(int);
extern void l3tol(long *, const char *, int);
extern char *l64a(long);
extern void ltol3(char *, const long *, int);
extern void * memalign(size_t, size_t);
extern char *mktemp(char *);
extern int putenv(char *);
extern char *realpath(char *, char *);
extern void swab(const char *, char *, int);
extern char *ttyname(int);
extern int ttyslot(void);
extern void *valloc(size_t);

extern double drand48(void);
extern double erand48(unsigned short *);
extern long jrand48(unsigned short *);
extern void lcong48(unsigned short *);
extern long lrand48(void);
extern long mrand48(void);
extern long nrand48(unsigned short *);
extern unsigned short *seed48(unsigned short *);
extern void srand48(long);
#endif	

#else /* not __STDC__ */

extern unsigned char 	_ctype[];

#define MB_CUR_MAX	(int)_ctype[520]

extern double atof();
extern int atoi();
extern long int atol();
extern double strtod();
extern long int strtol();
extern unsigned long strtoul();

extern int rand();
extern void srand();

extern char *calloc();
extern void free();
extern char *malloc();
extern char *realloc();

extern void abort();
extern int atexit();
extern void exit();
extern char *getenv();
extern int system();

extern char *bsearch();
extern void qsort();

extern int abs();
extern div_t div();
extern long int labs();
extern ldiv_t ldiv();

extern int mbtowc();
extern int mblen();
extern int wctomb();

extern size_t mbstowcs();
extern size_t wcstombs();

extern long a64l();
extern int dup2();
extern char *ecvt();
extern char *fcvt();
extern char *getcwd();
extern char *getlogin();
extern int getopt();
extern int getsubopt();
extern char *optarg;
extern int optind, opterr, optopt;
extern char *getpass();
extern int getpw();
extern char *gcvt();
extern int isatty();
extern void l3tol();
extern char *l64a();
extern void ltol3();
extern char *memalign();
extern char *mktemp();
extern int putenv();
extern char *realpath();
extern void swab();
extern char *ttyname();
extern int ttyslot();
extern char *valloc();

extern double drand48();
extern double erand48();
extern long jrand48();
extern void lcong48();
extern long lrand48();
extern long mrand48();
extern long nrand48();
extern unsigned short *seed48();
extern void srand48();
#endif	/* __STDC__ */

#define mblen(s, n)	mbtowc((wchar_t *)0, s, n)

#endif 	/* _STDLIB_H */
