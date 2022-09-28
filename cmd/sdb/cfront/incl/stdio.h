/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/stdio.h	1.1"
/*ident	"@(#)ctrans:incl/stdio.h	1.5.7.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/

#ifndef STDIOH
#define STDIOH

#ifdef uts
#define BUFSIZ  4096
#else
#  ifdef pyr
#    define BUFSIZ 2048
#    include        "sys/SYS_INC_UNIV"
#    ifdef UCB_SYS_INC
#      ifndef BSD
#       define BSD 1
#      endif
#    endif
#  else
#    define BUFSIZ  1024
#  endif
#endif
   
#ifdef pyr
#define _NFILE 62
#else
#ifdef hpux
#define _NFILE  60
#else
#define	_NFILE 20
#endif
#endif

#ifdef sun
#  define _SBFSIZ  8
#endif

#ifndef FILE
extern	struct	_iobuf {
#ifdef hpux
	int             _cnt;
        unsigned char   *_ptr;
        unsigned char   *_base;
        short           _flag;
        char            _file;
} _iob[_NFILE]; 
#else
#if defined(vax) || defined(u3b) || defined(u3b2) || defined(u3b5) \
	|| defined(u3b15) || defined(mc68k) || defined(pyr) || defined(i386)
	int	_cnt;
	char	*_ptr;
#else
#  ifdef sun
        int     _cnt;
        unsigned char   *_ptr;
#  else
        char    *_ptr;
        int     _cnt;
#  endif
#endif
#ifdef sun
        unsigned char   *_base;
#else
        char    *_base;
#endif
#   ifdef BSD
	int	_bufsiz;
	short	_flag;
#   else
	char	_flag;
#   endif
	char	_file;
#ifdef sun
} _iob[];
#else
} _iob[_NFILE];
#endif
#endif
#endif

#define _IOFBF    0000
#define	_IOREAD   0001
#define	_IOWRT    0002
#define	_IONBF    0004
#define	_IOMYBUF  0010
#define	_IOEOF    0020
#define	_IOERR    0040
#ifdef sun
# define        _IOSTRG   0100
# define        _IOLBF    0200
# define        _IORW     0400
#else
#ifdef hpux
# define        _IOLBF    0200
# define        _IORW     0400
#else
#  define       _IOLBF    0100
#  define       _IORW     0200
#endif
#endif

#define	NULL 0
#define	FILE struct _iobuf
#define	EOF (-1)

extern int _flsbuf(unsigned,FILE*);
extern int _filbuf(FILE*);

#define	stdin (&_iob[0])
#define	stdout (&_iob[1])
#define	stderr (&_iob[2])

#define	getc(p) (--(p)->_cnt>=0? *(p)->_ptr++&0377:_filbuf(p))
#define putc(x,p) (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p))

#define	getchar() getc(stdin)
#define	putchar(x) putc(x,stdout)
#define	feof(p) (((p)->_flag&_IOEOF)!=0)
#define	ferror(p) (((p)->_flag&_IOERR)!=0)
#define	fileno(p) ((p)->_file)

extern FILE* fopen(const char*, const char*);
extern FILE* fdopen(int, const char*);
extern FILE* freopen(const char*, const char*, FILE*);
extern long ftell(const FILE*);
extern char* fgets(char*, int, FILE*);

#define L_ctermid	9
#define L_cuserid	9
#ifdef sun
#define P_tmpdir "/usr/tmp/"
#else
#define P_tmpdir "/tmp/"
#endif
#define	L_tmpnam (sizeof(P_tmpdir) + 15)

extern char* gets(char*);
extern int puts(const char*);
extern int fputs(const char*, FILE*);
extern int printf(const char* ...);
extern int fprintf(FILE*, const char* ...);
extern int sprintf(char*, const char* ...);
extern int scanf(const char* ...);
extern int fscanf(FILE*, const char* ...);
extern int sscanf(char*, const char* ...);
extern int fread(char*, unsigned int, int, FILE*);
extern int fwrite(const char*, unsigned int, int, FILE*);
extern int fclose(FILE*);
extern int fflush(FILE*);
extern void clearerr(FILE*);
extern int fseek(FILE*, long, int);
extern void rewind(FILE*);
extern int getw(FILE*);
extern int fgetc(FILE*);
extern FILE* popen(const char*, const char*);
extern int pclose(FILE*);
extern int putw(int, FILE*);
extern int fputc(int, FILE*);
extern void setbuf(FILE*, char*);
extern int ungetc(int, FILE*);

extern void exit(int);
extern int abort();

extern int atoi(const char*);
extern double atof(const char*);
extern long atol(const char*);

#define _bufend(p)	_bufendtab[(p)->_file]
#define _bufsiz(p)	(_bufend(p) - (p)->_base)

extern FILE     *tmpfile ();
extern char	*ctermid(char*),
                *cuserid(char*),
                *tempnam(char*, char*),
                *tmpnam(char*);

extern void perror (const char*);

extern int errno;
extern char* sys_errlist[];
extern int sys_nerr;
extern unsigned char *_bufendtab[];

#endif /* STDIOH */
