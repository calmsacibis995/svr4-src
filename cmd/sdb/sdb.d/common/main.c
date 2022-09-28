/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:sdb/common/main.c	1.21.3.1"
/*
 *	UNIX debugger
 */

#include <setjmp.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>
#include <termio.h>
#include <fcntl.h>
#include <sgs.h>
#include "prioctl.h"
#include "utility.h"

#define FDTTY   1       /*  file desc for saving/restoring tty state */
#define FDIN    0       /*  for fcntl save/restore */

#ifndef SGTTY
static  struct termio sdbttym, usrttym;	/* tty structure(s) */
static  int sdbttyf, usrttyf;			/* fcntl flags */
#else
static  struct sgttyb sdbttym, usrttym;	/* tty modes for sdb and user */
#endif

/* #define OPTS		"VWds:we?"	*/
#define OPTS		"VWs:we?"

char			*corfil = "core";	/* core filename */
char			*symfil = "a.out";	/* executable filename */
time_t			SymFilTime;		/* modification time of symfil */
int				Wflag = 0;		/* -W option */
static int		Vflag;			/* -V option */
static int		wtflag;			/* -w option */
extern int		debugflag;		/* -d option or 'Y' command */
static int		sflag;			/* -s option */
static int		mauflag;		/* MAU existence flag */
static int		flatmap;		/* addresses are offsets */

static jmp_buf env;

static
show_options( s )
char *	s;
{
	printf("usage: %s -VWs:we [objfil [corfil [dir-list]]]\n",s);
	printf("\t\t[-V print version information]\n");
	printf("\t\t[-W suppress warning messages]\n");
/*	printf("\t\t[-d turn on internal debugging]\n");	*/
	printf("\t\t[-s don't stop process on signal (argument is sig num)]\n");
	printf("\t\t[-w write access to objfil and corfil]\n");
	printf("\t\t[-e use addresses as file offsets]\n");
	printf("\t\t[-? display options and quit]\n");
}

char	sdbpath[128];		/* Directory lookup path array for files. */
int	sdbpsz=sizeof(sdbpath);	/* Size of the lookup array. */

sigset_t	e_sigset;

SDBmain(argc,argv)
int argc;
char *argv[];
{
	FILE		*fp;
	void		fault();
	void		fpe();
	register char 	*p;
	struct stat 	stbuf;
	int 		i ,c;
	extern int 	optind;
	extern char *	optarg;
	int		sig;
	SIG_PF sigint;
	SIG_PF sigqit;
	int argcount=0;

	/*
	 * use unbuffered output
	 */
	(void) setbuf(stdout, NULL);	
	(void) setbuf(stderr, NULL);

	pushoutfile(stdout);

	/*
	 * process command line arguments
	 */
	Vflag = 0;
	flatmap = 0;
	wtflag = O_RDONLY;
	prfillset( &e_sigset );
	while (optind < argc) {
		c = getopt(argc, argv, OPTS);
		switch(c) {
			case 's':
				sig = strtol( optarg, 0, 0 );
				prdelset( &e_sigset, sig );
				break;
			case 'w':
				wtflag = O_RDWR;
				break;
/*
			case 'd':
				debugflag++;
				break;
*/
			case 'W':
				Wflag++;
				break;
			case 'V':
				++Vflag;
				printf("%s: %s%s.\n",argv[0],SGU_PKG,SGU_REL);
				break;
			case 'e':
				flatmap++;
				break;
			case '?':
				show_options( argv[0] );
				exit(0);
				break;
			case EOF:
				switch(argcount) {
				case 0: 		/* symbol table file */
					symfil = argv[optind];
					if ( symfil[0] != '-' &&
						    !strchr(symfil,'/') ) {
						symfil =
						    (char *)malloc(
							    strlen(symfil)+3);
						strcpy(symfil,"./");
						strcat(symfil,argv[optind]);
					}
					break;
				case 1: 		/* core file */
					corfil = argv[optind];
					break;
				case 2: 		/* source path */
					strncpy( sdbpath, argv[ optind ], sdbpsz );
					if( isfile( sdbpath ) ) {
						if( ( fp = fopen( sdbpath, "r" ) ) 
									!= NULL ) {
							fgets( sdbpath, sdbpsz, fp );
							sdbpath[strlen(sdbpath)-1] = 0;
							fclose( fp );
							}
						else
						{
							printf( 
						"Can't open pathfile: %s\n",sdbpath);
							exit( 2 );
						
						}
					}
				}
				argcount++;
				optind++;
				break;	
			default:
				exit(4);
		}
	}

	if ( Vflag && (argc == 2) ) exit(0);

	/*
	 * show source path
	 */
	if( sdbpath[ 0 ] ) {
		printf( "Source path: " );
		for(p=sdbpath;*p;p++) {
			printchar(*p);	/* user will see if junk is in file */
		}
		printf("\n");
	}

	if(sdbpath[0])
		set_path( sdbpath );

	/*
	 * check executable
	 */
	if(symfil[0] == '-')		/* name "-" ==> ignore a.out */
		printf("Warning: `%s' does not exist\n",symfil);
	else if(stat(symfil,&stbuf) == -1) {
		printf("`%s' does not exist\n",symfil);
		exit(4);
	}

	/*
	 * Check that core file is newer than symbol file 
	 */
	SymFilTime = stbuf.st_mtime;
	if(stat(corfil,&stbuf) == -1) {
		if ((argc > optind) && (corfil[0] != '-') && (corfil[1] != '\0'))
			printf( "Warning: can't access `%s'\n",corfil);
		corfil = 0;		/* so grab_core() is happy */
	}
	else if(!Wflag && SymFilTime > stbuf.st_mtime) {
		printf("Warning: `%s' newer than `%s'\n",symfil,corfil);
	}

	/*
	 * initialize sdb data structures
	 */
	if( corfil && corfil[0] == '-' && corfil[1] == '\0')	/* - means ignore core */
		corfil = 0;
	if(grab_core(symfil,corfil,wtflag,e_sigset,flatmap) == 0) {
		printf( "grab_core failed\n");
		exit(7);
	}
	if((corfil == 0) && !grabbed(current_process)) {
		char *	s;
		long	nl;

		printf("no process\n");
		s = find_function( current_process, "main", &nl );
		set_current_src( current_process, s, nl );
	}

	/* set up variables for user */
	ioctl( FDTTY, TCGETA, &sdbttym );   /* save initial state of terminal */
	sdbttyf = fcntl( FDIN, F_GETFL, 0 ); /* save initial status bits */

	/* if user was ignoring interrupts, fine; else trap to fault() */
	if ( (sigint =  signal( SIGINT, SIG_IGN ) ) !=  SIG_IGN) {
		sigint = (SIG_PF) fault;
		signal( SIGINT, fault );
	}
	sigqit =  signal( SIGQUIT, SIG_IGN ); /* ignore "quit" signal */
	signal( SIGILL, fpe );	/* fpe() handles illegal instructions */
#if u3b || u3b5 || u3b15 || vax
	signal( SIGFPE, fpe );	/* fpe() handles floating point exceptions */
#endif
	setjmp(env);

	sdbtty();	/* save user tty modes and restore sdb tty modes */

	docommands();

	exit(0);
}

void
fault(a)
int a;
{
	signal(a,fault);	/* cancel pending signal "a" */
	interrupted = 1;	/* reset in inform_processes() */
}

void
fpe(i)
int i;
{
	signal(SIGILL, fpe);	/* cancel pending signal SIGILL */
#if u3b || u3b5 || u3b15 || vax
	signal( SIGFPE, fpe );	/* cancel pending signal SIGFPE */
#endif
	printf("Illegal floating constant\n");
	longjmp(env, 0);
}

int isfile(s)
char * s;
{
	struct stat	sbuf;

	if(stat(s,&sbuf)==-1) {
		return( 0 );
	}
	if((sbuf.st_mode&S_IFMT)==S_IFREG) {
		return( 1 );
	}
	return( 0 );
}

static void
do_nothing_at_all()
{	/* make sure the following get loaded */
	extern int regtype();
	regtype();
}

printchar(c)
char c; {
	if ((c & 0177) < ' ') 
		printf("^%c", c + ('A' - 1));
	else if ((c & 0177) == 0177)
		printf("^?");
	else
		printf("%c", c);
}

/* added sdbtty() and also ifndef SGTTY
 * Save current user tty modes
 * and restore original sdb tty modes
 */

sdbtty()
{
#ifndef SGTTY
	usrttyf = fcntl(FDIN, F_GETFL, 0);
	ioctl(FDTTY, TCGETA, &usrttym);
	ioctl(FDTTY, TCSETAW, &sdbttym);
	fcntl(FDIN, F_SETFL, sdbttyf);
#else
	gtty(FDTTY, &usrttym);
	if (sdbttym.sg_flags != usrttym.sg_flags) {
		/* Bug in USG tty driver, put out a DEL as a patch. */
		if (sdbttym.sg_ospeed >= B1200)
			write(FDTTY, "\377", 1);
		stty(FDTTY, &sdbttym);
	}
#endif
}
