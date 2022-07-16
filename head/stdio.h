/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
* User-visible pieces of the ANSI C standard I/O package.
*/
#ifndef _STDIO_H /* if not defined then stdio.h has not yet been included */
#define _STDIO_H

#ident	"@(#)head:stdio.h	2.34.1.2"

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int 	size_t;
#endif 

typedef long	fpos_t;

#ifndef NULL
#define NULL            0
#endif 

#if defined(__STDC__)

#if #machine(pdp11)
#   define BUFSIZ		512
#   define _STDIO_REVERSE
#elif #machine(u370)
#   define BUFSIZ		4096
#   define _STDIO_REVERSE
#   define _STDIO_ALLOCATE
#else
#   define BUFSIZ		1024
#endif

#if #machine(i386)
#define _NFILE	60	/* initial number of streams */
#else
#define _NFILE	20	/* initial number of streams */
#endif

#else	/* !defined(__STDC__) */

#if pdp11 || u370

#if pdp11
#   define BUFSIZ		512
#   define _STDIO_REVERSE
#else 	/* u370 */
#   define BUFSIZ		4096
#   define _STDIO_REVERSE
#   define _STDIO_ALLOCATE
#endif

#else
#   define BUFSIZ		1024
#endif

#ifdef i386
#define _NFILE	60	/* initial number of streams */
#else
#define _NFILE	20	/* initial number of streams */
#endif

#endif	/* __STDC__ */

#define _SBFSIZ	8	/* compatibility with shared libs */

#define _IOFBF		0000	/* full buffered */
#define _IOLBF		0100	/* line buffered */
#define _IONBF		0004	/* not buffered */
#define _IOEOF		0020	/* EOF reached on read */
#define _IOERR		0040	/* I/O error from system */

#define _IOREAD		0001	/* currently reading */
#define _IOWRT		0002	/* currently writing */
#define _IORW		0200	/* opened for reading and writing */
#define _IOMYBUF	0010	/* stdio malloc()'d buffer */

#ifndef EOF
#   define EOF	(-1)
#endif

#define FOPEN_MAX	_NFILE
#define FILENAME_MAX    1024	/* max # of characters in a path name */

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#define TMP_MAX		17576	/* 26 * 26 * 26 */

#if __STDC__ - 0 == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#define L_ctermid	9
#define L_cuserid	9
#endif

#if (__STDC__ - 0 == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)
#define P_tmpdir	"/var/tmp/"
#endif

#define L_tmpnam	25	/* (sizeof(P_tmpdir) + 15) */

#if defined(__STDC__)
#define stdin	(&__iob[0])
#define stdout	(&__iob[1])
#define stderr	(&__iob[2])
#else
#define stdin	(&_iob[0])
#define stdout	(&_iob[1])
#define stderr	(&_iob[2])
#endif

typedef struct	/* needs to be binary-compatible with old versions */
{
#ifdef _STDIO_REVERSE
	unsigned char	*_ptr;	/* next character from/to here in buffer */
	int		_cnt;	/* number of available characters in buffer */
#else
	int		_cnt;	/* number of available characters in buffer */
	unsigned char	*_ptr;	/* next character from/to here in buffer */
#endif
	unsigned char	*_base;	/* the buffer */
	unsigned char	_flag;	/* the state of the stream */
	unsigned char	_file;	/* UNIX System file descriptor */
} FILE;

#if defined(__STDC__)
extern FILE		__iob[_NFILE];
#else
extern FILE		_iob[_NFILE];
#endif
extern FILE		*_lastbuf;
extern unsigned char 	*_bufendtab[];
#ifndef _STDIO_ALLOCATE
extern unsigned char	 _sibuf[], _sobuf[];
#endif

#if defined(__STDC__)

extern int	remove(const char *);
extern int	rename(const char *, const char *);
extern FILE	*tmpfile(void);
extern char	*tmpnam(char *);
extern int	fclose(FILE *);
extern int	fflush(FILE *);
extern FILE	*fopen(const char *, const char *);
extern FILE	*freopen(const char *, const char *, FILE *);
extern void	setbuf(FILE *, char *);
extern int	setvbuf(FILE *, char *, int, size_t);
/* PRINTFLIKE2 */
extern int	fprintf(FILE *, const char *, ...);
/* SCANFLIKE2 */
extern int	fscanf(FILE *, const char *, ...);
/* PRINTFLIKE1 */
extern int	printf(const char *, ...);
/* SCANFLIKE1 */
extern int	scanf(const char *, ...);
/* PRINTFLIKE2 */
extern int	sprintf(char *, const char *, ...);
/* SCANFLIKE2 */
extern int	sscanf(const char *, const char *, ...);
extern int	vfprintf(FILE *, const char *, void *);
extern int	vprintf(const char *, void *);
extern int	vsprintf(char *, const char *, void *);
extern int	fgetc(FILE *);
extern char	*fgets(char *, int, FILE *); 
extern int	fputc(int, FILE *);
extern int	fputs(const char *, FILE *);
extern int	getc(FILE *);
extern int	getchar(void);
extern char	*gets(char *);
extern int	putc(int, FILE *);
extern int	putchar(int);
extern int	puts(const char *);
extern int	ungetc(int, FILE *);
extern size_t	fread(void *, size_t, size_t, FILE *);
	#pragma int_to_unsigned fread
extern size_t	fwrite(const void *, size_t, size_t, FILE *);
	#pragma int_to_unsigned fwrite
extern int	fgetpos(FILE *, fpos_t *);
extern int	fseek(FILE *, long, int);
extern int	fsetpos(FILE *, const fpos_t *);
extern long	ftell(FILE *);
extern void	rewind(FILE *);
extern void	clearerr(FILE *);
extern int	feof(FILE *);
extern int	ferror(FILE *);
extern void	perror(const char *);

extern int	__filbuf(FILE *);
extern int	__flsbuf(int, FILE *);

#if !#lint(on)
#define getc(p)		(--(p)->_cnt < 0 ? __filbuf(p) : (int)*(p)->_ptr++)
#define putc(x, p)	(--(p)->_cnt < 0 ? __flsbuf((x), (p)) \
				: (int)(*(p)->_ptr++ = (x)))
#define getchar()	getc(stdin)
#define putchar(x)	putc((x), stdout)
#define clearerr(p)	((void)((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p)		((p)->_flag & _IOEOF)
#define ferror(p)	((p)->_flag & _IOERR)
#endif

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) /* non-ANSI standard compilation */

extern FILE    *fdopen(int, const char *);
extern FILE    *popen(const char *, const char *);
extern char    *ctermid(char *);
extern char    *cuserid(char *);
extern char    *tempnam(const char *, const char *);
extern int     getw(FILE *);
extern int     putw(int, FILE *);
extern int     pclose(FILE *);
extern int     system(const char *);
extern int	fileno(FILE *);

#if !#lint(on)
#define fileno(p)	(p)->_file
#endif

#endif	/* __STDC__ == 0 */

#else	/* !defined __STDC__ */
#define _bufend(p)      _bufendtab[(p)->_file]
#define _bufsiz(p)      (_bufend(p) - (p)->_base)

#ifndef lint
#define getc(p)         (--(p)->_cnt < 0 ? _filbuf(p) : (int) *(p)->_ptr++)
#define putc(x, p)      (--(p)->_cnt < 0 ? \
                        _flsbuf((unsigned char) (x), (p)) : \
                        (int) (*(p)->_ptr++ = (unsigned char) (x)))
#define getchar()       getc(stdin)
#define putchar(x)      putc((x), stdout)
#define clearerr(p)     ((void) ((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p)         ((p)->_flag & _IOEOF)
#define ferror(p)       ((p)->_flag & _IOERR)
#define fileno(p)       (p)->_file
#endif	/* lint */

extern FILE     *fopen(), *fdopen(), *freopen(), *popen(), *tmpfile();
extern long     ftell();
extern void     rewind(), setbuf();
extern char     *ctermid(), *cuserid(), *fgets(), *gets(), *tempnam(), *tmpnam();
extern int      fclose(), fflush(), fread(), fwrite(), fseek(), fgetc(),
                getw(), pclose(), printf(), fprintf(), sprintf(),
                vprintf(), vfprintf(), vsprintf(), fputc(), putw(),
                puts(), fputs(), scanf(), fscanf(), sscanf(),
                setvbuf(), system(), ungetc();

#endif	/* __STDC__ */

#endif  /* _STDIO_H */
