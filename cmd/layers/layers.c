/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:layers.c	2.25.11.2"

/*
 * Bring up the Blit layers universe.
 *
 * The layers command supports the following options :
 *
 * 	p 	Option passed to the loader. If a firmware patch has been 
 *		downloaded, print the down-loading protocol statistics and 
 *		trace on standard error   
 *
 *	d	Option passed to the loader. If a firmware patch has been      
 *	        downloaded, print the sizes of the text, data and bss portions 
 *		of the patch on standard error.				   
 *
 *	t	Turn XT tracing on and display the trace at the end of session
 *
 *	s	Display XT statistics at the end of the session
 *
 *	D	Debugging - log the activities of layers to stderr
 *
 *	h	Push the supplied modules on the MUX (XT) : default ldterm
 *		The following argument  is treated as a list of modules. The 
 *		modules in the list should be separted from each other by a 
 *		','. There should not be any spaces in the list. 
 *
 *	M	Push the dump modules on the XT before the MUX is created. In 
 *		essence, these modules will reside under ldterm or the modules
 *		pushed by the h option. THIS OPTION IS VALID ONLY WHEN 'DEBUG'
 *		IS SET. iT IS AN UNDOCUMENTED OPTION.
 *		The following argument  is treated as a list of modules. The 
 *		modules in the list should be separted from each other by a 
 *		','. There should not be any spaces in the list. 
 *
 *	f 	The following argument is treated like an atuo-configuration 
 *		startup file. This file should contain entries of the following
 *		type : 
 *
 *		origin.x origin.y corner.x corner.y <shell command list 1>
 *		origin.x origin.y corner.x corner.y <shell command list 2>
 *
 * 		The first window will be created at the rectangle coordinates
 *		given in the first line and 'shell command 1' will be executed.
 *
 *      	The second window will be created at the rectangle coordinates
 *  		given in the second line and 'shell command 2' will be executed 
 * 		and so on.
 *
 *	      **For a 630, the valid coordinates are 6,6(upper left corner of
 *	 	the screen) to 1018,1018(lower right corner of the screen).
 *	
 *	        The coordinates are in pixels. If the no. of rows is r and the 
 *		no. of columns is c, the following relationship holds :
 *
 *		jwin size in bits y = difference in y coordinates =
 *				    (r * font_height) + 34
 *		jwin size in bits y = difference in y coordinates =
 *				    (c * font_width) + 29
 *
 *		Thus corner x = origin x + jwin size in bits x
 *		     corner y = origin y + jwin size in bits y
 *
 *		For a 630, the font sizes are as follows :
 *
 *		small font   : font height = 14 pixels, font width = 7 pixels
 *		mdedium font : font height = 14 pixels, font width = 9 pixels
 *		large font   : font height = 16 pixels, font width = 11 pixels
 *	        
 *	      **Windows which have an origin outside the valid border will be 
 *		moved inside. (corners are truncated)
 *	
 *	      **If all the coordinates are 0, the user will be prompted to 
 *		create them through the mouse at run time.
 *
 *	      **If any line is illegal, it will be ignored. The window 
 *	        indicated by the last legal line is made current.
 *
 *	b	Ignore BREAK condition.
 *
 *   		The history of BREAK condition :
 *
 *		BREAK cannot be sent as a normal BREAK in layers because
 *		it is valid only for the window in which the BREAK key was 
 *		pressed. Initially, there was an idea of reporting the BREAK 
 *		condition as a separate packet in layers. This idea was thought
 *		to be viable but was dropped because it involved a lot of 
 *		changes on the terminal side for supporting it on all the 
 *		terminals.
 *
 *		Thus BREAK has never been supported in layers. The terminal 
 *		simply ignores BREAK key. Therefore, layers will never receive 
 *		the BREAK condition. This brings up the question of why an 
 *		ignore break option is needed when BREAK is not supported by 
 *		the terminal in the layers mode. The following is an 
 *		explanation of this paradox. 
 *
 *		Conditions arose when the host would be in layers and the 
 *		terminal would be in non-layers mode. To get out of this 
 *		condition, a suggestion was made that the terminal should 
 *		transmit the BREAK condition and that host layers should exit 
 *		on detection of the BREAK condition.
 *
 *		One opposition to the above suggestion was that noise on the 
 *		line could cause a spurious BREAK condition. To avoid layers 
 *		from exiting in such cases, it was suggested that an option be 
 *		added to layers to ignore the break key. This option was 
 *		implemented but was never rigorously tested. A decision was 
 *		later made to get rid of the option all together. 
 *
 *		IN THE CURRENT VERSION, THE PREVIOUS IMPLEMENTATION OF 
 *		IGNORING BREAK HAS BEEN PRESERVED THROUGH THE 'IGNBRK' DEFINE. 
 *		IT IS REMOVED FROM THE USAGE MESSAGE AND THE LAYERS DOCUMENTS 
 *		WON'T MENTION IT.
 *
 */

#ifdef SVR32
#include "sys/patch.h"
#endif /* SVR32 */
#include "sys/types.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "ctype.h"
#include "grp.h"
#include "sys/stat.h"
#include "sys/errno.h"
#include "signal.h"
#include "windows.h"
#include "sys/jioctl.h"
#include "sys/nxtproto.h"
#include "sys/termio.h"
#include "fcntl.h"
#include "stdio.h"
#include "sxtstat.h"
#include "sys/nxt.h"
#include "sys/tty.h"
#include "setjmp.h"
#include "sys/conf.h"	/* for I_LOOK ioctl (nonlayersld) */
#include "setjmp.h"

#ifdef i386
#define	XTIOCTYPE	('b'<<8)
#define	XTIOCLINK	(XTIOCTYPE|1)
#define	HXTIOCLINK	(XTIOCTYPE|6)

/** Link set up request **/
struct xtioclm
{
	char	fd;		/* File descriptor for 'real' 'tty' line */
	char	nchans;		/* Maximum channels that will be used */
};
#endif

#ifdef SVR32
/* Seems to be a new type from stat.h defined in SVR4.0 - missing
 * from SVR3.2.
 */
#define S_IAMB 0777
#endif /* SVR32 */

/* generic defines : typically should be a part of header file		     */

typedef	char		bool;	/* boolean type : only TRUE, FALSE to be used*/
#define TRUE		0xff
#define FALSE		0
#define SUCCEED		0	/* indicates success status in all functions */
#define FAIL		(-1)	/* indicates error status in local functions */
#define ERREXIT		1	/* exit program with error		     */
#define	SYSERROR 	(-1)	/* used for return from system functions     */ 
#define EOS		'\0'	/* End of string			     */

/* Arguments to error(). When called with DISP_ERR, the terminal is able
 * to display errors (for example, because it is not in xt mode yet), so just
 * print the error. When called with STORE_ERR, print the error if stderr
 * is redirected. Otherwise, store it for printing on layers exit. UNBOOT
 * is for errors after the terminal goes into xt mode but before the
 * I_UNLINK. UNBOOT will unboot the terminal and then display the error.
*/
#define DISP_ERR	0
#define STORE_ERR	1
#define UNBOOT		3


/* Currently no use is made of the difference between HEX_LOAD and ENC_ENABLE*/
/* In hex encoding mode, 3 bytes are transformed into 4 bytes; the characters*/
/* are still transmitted as 8 bits per character, but the most significant 2 */
/* bits are translated. 						     */	

#define BINARY_LOAD	0	/* hex encoding is not enabled: 8 bits per ch*/
#define HEX_LOAD	1	/* enable hex encoding : 6 bits per char     */
#define	ENC_ENABLE	2	/* value that enables terminal encoding      */


#define	LINKTIMO 	10	/* timeout value for setting up mux link     */
#define	JMSGSIZE 	2 	/* old IFDEFs ripped out of here 	     */
#define	MAXSTR		256	/* maximum size of strings in this module    */

/* defines for errors encountered in layers				     */

#define erNOTTTY	1	/* stdin or stdout are not tty's	     */
#define	erINVARG	2	/* invalid arguments			     */
#define erOPNLYR	3	/* layers already open			     */
#define erSHLAYR	4	/* shell layers active			     */
#define erSCROPN	5	/* error in opening script file 	     */
#define erCTLOPN	6	/* error in opening control channel	     */
#define erTRCOPN	7	/* error in opening the trace file	     */
#define erMUXOPN	8	/* error in setting up encoding		     */
#define erDMBOOT	9	/* error in booting : loading the DMD prog   */
#define erENCODE	10	/* error in setting up encoding		     */
#define erRLOGIN	11	/* error in changing the utmp entry          */
#define erSETPGR	12	/* error in setpgrp of control channel 	     */
#define erSTATFAIL	13	/* error in fstat in main()	 	     */
#define erJXTPROTO	14	/* error from JXTPROTO in switchproto()      */
#define erJTIMOM	15	/* error from JTIMOM in switchproto()	     */
#define erCONTROLIOCTL	16	/* error in initial ioctl of control channel */
#define erNOTSTREAMS	17	/* ran stream layers over non-streams device */
#define erTIMOUT	(-2)	/* Time out error			     */
 


#define CHAR(x)		((x) & 0x7f) /* if terminal is 7bpc and raw is 8bpc  */ 

/* flags used for command line options 					     */

char dumpon = 0; 		/* dump module to be pushed on the contr chan*/
char *pdmplist; 		/* pointer to dump module list		     */ 
char sld = 0;			/* -p option to loader : print down-load     */
				/* protocol statistics and trace 	     */
char dbg = 0;			/* -d option to loader : print text, data and*/
				/* bss sizes for the firmware patch 	     */
char ldlist = 0; 		/* list of modules to be pushed 	     */
char debug = 0;		 	/* flag to indicate -D debug is on   	     */
char ignbrk = 0;		/* is BREAK to be ignored ?  		     */


/* global variables initialised to default values			     */ 

/* usage message for command line errors          */ 
/* -M dump modules is used only in the debug mode */
char usage[] = "usage: %s [-Ddpst] [-h modlist] [-f start-prgm] [-m max-pkt ] [layersys-prgm]\n";

#ifdef SVR32
char cntlf[] = "/dev/nxt000";	/* control channel for 1st terminal	     */ 
#endif /* SVR32 */
#ifdef SVR40
char cntlf[] = "/dev/xt/000";	/* control channel for 1st terminal	     */ 
#endif /* SVR40 */

#ifdef i386
char stream = 1;		/* Assume device is STREAMS based */
char ocntlf[] = "/dev/xto000";	/* control channel for 1st terminal	     */ 
#endif

char dmdprog[MAXSTR] = "";	/* DMD program to be loaded on boot 	     */
char dmdenv[MAXSTR];		/* DMD environment used for loading purposes */
char shell[MAXSTR] = "sh";	/* shell to be execed in a new window 	     */
char iopt[] = "-i";		/* options to be passed to the execed shell  */
char icopt[] = "-ic";		/* options to be passed for window on which  */
				/* a command is to be executed; a shell      */
				/* should be the last command in this seq    */
				/* else the terminal will hang		     */ 
char *execarg[] = { shell, iopt, 0, 0 }; /* arguments to be passed to child  */
				/* on execing a new shell		     */
char relogin[] = "/usr/lib/layersys/relogin"; /* relogin command	     */
char *nonlayersld[FMNAMESZ+1];	/* save non-layers line discipline name	     */
int ttcompat_pushed;		/* set if ttcompat module is pushed   	     */
char *layersld = "ldterm";	/* STREAMS line discipline used by layers    */
char *dumpld = "dump";		/* default STREAMS dump module 		     */
struct jerqmesg Termesg	= { (char)JTERM }; /* command to be sent to the      */
				/* terminal on receiving a C_EXIT command    */ 

#define CHANIND		(sizeof(cntlf) - 2) /* index of the channel byte     */

/* assorted global variables 						     */

#ifdef INCLUDE
char *ttyfile;			/* full name of tty, e.g., "/dev/tty11"	     */
#endif
char ttyfile[80];		/* full name of tty, e.g., "/dev/tty11"	     */
struct termio ttysave; 		/* termio structure used to save the initial */
				/* values of the controlling tty.            */
struct termio ttyraw;  		/* termio structure  used to save the value  */
				/* values after setting it in the raw mode   */
struct stat ttystat;		/* output of fstat on the tty		     */
int mux_id;			/* mux identifier returned by I_LINK ioctl   */
char *pmodlist;			/* pointer to module list		     */
char *layerscmd;		/* name of the layers command		     */ 
int Loadtype = BINARY_LOAD;     /* Binary or hex load           	     */

bool use_errbuf = FALSE;	/* are error messages to be stored in errbuf?*/
long errtime;			/* time last error was stored in errbuf      */
char errbuf[512];		/* Buffer to store last error() if stderr not
				   redirected.				     */
char firstshell = 0;		/* is it the first window ? used to set the  */
				/* utmp entry to the new window device       */
pid_t shells[MAXPCHAN];		/* the pid of the parent shell of a window   */ 
				/* remember that this shell is a layers child*/
FILE *fpconf;			/* file pointer for layers -f file           */

bool	fSIGHUP = FALSE;	/* flag to indicate if SIGHUP was received   */
pid_t	sid;			/* session id required for hangup */ 
pid_t	myppid; 		/* parent id required for hangup */ 

int whichsig;			/* These are for communication between       */
int dolongjmp;			/*   doexec() and doexec_sigcatch().         */
jmp_buf doexecenv;

int ttyp;			/* terminal type			     */
int tver;			/* version of the terminal	     	     */

/* extern non-int C functions 						    */

extern FILE *fopen();
extern unsigned	alarm();
extern void exit();
extern char *strcpy();
extern char *strtok();
extern char *ttyname();
extern char *getenv();
extern char *ctime();
extern long time();

/* extern layers functions  						     */

extern int xtraces();
extern int xtstats();

/* forward declaration of non-int functions of this module		     */

void trace();
void control(); 
void doexec();
void doexec_sigcatch();
void error();
void raw();
void sigcatch(); 
void undoshell();
void termshells();
void dorun();
FILE *openffile();
FILE *opentrace();


/* extern system variables						     */

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

sigjmp_buf jmpenv;

got_hangup()
{
	fSIGHUP = TRUE;
	signal(SIGPOLL,SIG_IGN);
	siglongjmp(jmpenv,1);
	return;
}


/* There are two defines used throughout the code : 
   DEBUG  : for tracing purposes
   IGNBRK : To accept ignore break from the command line
	    By default break is not ignored.



*/

/* EXIT VALUES : as documented above under errors returned from layers       */

main(argc, argv, envp)
int argc;			/* no. of arguments 			     */
register char *argv[];		/* pointer to arguments			     */
register char *envp[];		/* pointer to environment variables 	     */
{
	char *ttytmp;		/* temp storage for return from ttyname()    */
	struct sxtblock sxtstat;/* dummy storage area for ioctl call         */ 
	register int option;	/* exit value				     */
	char statson = 0;	/* is statistics to be displayed on exit ?   */
	char traceon = 0;	/* is trace to be displayed on exit ?        */
	char ffile = 0;		/* is set-up file to be read before exec ?   */
	unsigned char maxpktarg = 0; /* hold -m option */
	register char *myshell; /* environment SHELL variable		     */
	register int cntlfd;    /* file descriptor for control window : win0 */ 
	extern int optind;	/* index of the next argument		     */
	extern char *optarg;	/* option argument			     */
	int ttyfd;		/* file descriptor of the tty device 	     */
	int exitval;		/* exit value from main			     */
	pid_t setpval;		/* value returned by setpgrp 		     */
	extern struct Tbuf Traces; /* defined in xtraces.c */
	extern Stats_t Stats[];    /* defined in xtstats.c */

#ifdef i386
	int rc = 0;
#endif

	layerscmd = *argv;	/* save our name */

	if( (ttytmp = ttyname(0)) == NULL) {
		error(DISP_ERR, 0, "stdin not associated with a terminal device"); 
		exit(erNOTTTY);
	}

	strcpy(ttyfile, ttytmp); /* save for later use */

	if( (ttytmp = ttyname(1)) == NULL) {
		error(DISP_ERR, 0, "stdout not associated with a terminal device"); 
		exit(erNOTTTY);
	}

	if( strcmp(ttyfile, ttytmp) != 0 ) {
		error(DISP_ERR, 0, "stdin and stdout must be attached to the same tty");
		exit(erNOTTTY);
	}

	/* Store error messages in a buffer for display on layers exit
	 * if stderr is not redirected. Stderr is not redirected if
	 * stdout and stderr are the same /dev file.
	 */
	if( (ttytmp = ttyname(2)) != NULL) {
		if( strcmp(ttyfile, ttytmp) == 0 )
			use_errbuf = TRUE;
	}

	/* check if layers is already active on the terminal 		     */

	if (ioctl(1, JMPX, NULL) != -1) {
		error(DISP_ERR, 0, "Cannot invoke layers from within a layer");
		exit(erOPNLYR);
	}

	/* check if shell layers is already active on the terminal	     */

	if (ioctl(1, SXTIOCSTAT, &sxtstat) != -1) {
		error(DISP_ERR, 0, "Cannot invoke layers from within shl");
		exit(erSHLAYR);
	}

	/* Process all the options. 					     */
	/* Only error messgages should be generated during the checking of   */
	/* the arguments. Any attempt to trace through calls to trace will   */
	/* fail. Remember that trace file is opened only after all the args  */
	/* are processed.						     */

	while ((option = getopt (argc, argv, "M:h:Df:m:dbpst")) != EOF)
		switch ( option ) {

		  case 'h':
			/* The following arg is the name of the module       */
		        /* to be pushed on the mux. The default module is    */
			/* ldterm. Letter h is chosen for consistency with   */
			/* other commands.				     */
  
			ldlist++;
			pmodlist = optarg;
			break;	


#ifdef DEBUG
		  case 'M':
			/* Dump modules to be pushed on the mux.	     */

			dumpon++;		
			pdmplist = optarg;
			break;	
#endif

#ifdef IGNBRK
		  case 'b':			/* ignore BREAK (SIGINT)     */
			ignbrk++;
			break;
#endif

		  case 'D':			/* debug option		     */
			debug++;		
			break;	

		  case 's':
			/* Display protocol statistics on standard error on  */
			/* exiting layers. 				     */

			statson++;
			break;

		  case 't':			
			/* Display trace dump on standard error on exiting   */
			/* layers. 				      	     */

			traceon++;
			break;

		  case 'p':
			/* Passed as -p option to the loader. If a firmware  */
			/* patch has been downloaded, print the down-loading */
			/* protocol statistics and trace on standard error   */
			
			sld++;
			break;

		  case 'd':
			/* Passed as -d option to the loader. If a firmware  */
			/* patch has been downloaded, print the sizes of the */ 
			/* text, data and bss portions of the patch on       */
			/* standard error.				     */
			
			dbg++;
			break;
			
		  case 'f':			/* take the following arg as */
						/* file with auto config     */
			if ((fpconf = openffile(optarg)) == (FILE *) NULL)
				exit(erSCROPN);
			ffile++;
			break;

		case 'm':
			if( isdigit(optarg[0]) ) {
				int temp; /* because maxpktarg is a char */
				temp = atoi(optarg);
				if(temp >= 32 && temp <= 252) {
					maxpktarg = temp;
					break;
				}
				else {
					fprintf(stderr,
					   "Legal max-pkt values are 32 to 252\n");
				}
			}
			/* no break - fall through */

		default:			/* invalid argument	     */
			fprintf(stderr, usage, layerscmd);
			exit(erINVARG);
		}

	trace("ttyfile = %s", ttyfile);
#ifdef i386
	{
		char *ttynm;
		int temp_fd;

		ttynm=ttyname(0);
		if ((temp_fd=open(ttynm,O_RDWR)) == -1) {
			perror("open:");
			exit(1);
		}
		if (isastream(temp_fd) != 1)
			stream=0;
	}
#endif

	/* Get the name of the streams line discipline being used so it can
	 * be restored on exit. Also, since this is the first streams
	 * specific ioctl, failure implies an attempt to execute over a
	 * non-streams device, so print an error message.
	 */
    if (stream)
	if (ioctl(1, I_LOOK, nonlayersld) != -1) {
		if( strcmp(nonlayersld, "ttcompat") == 0 ) {
			/* Have to wait until multiplex() to I_LOOK at
			 * the line discipline because shouldn't I_POP
			 * ttcompat yet.
			 */
			ttcompat_pushed = 1;
			trace("main: ttcompat was pushed");
		} else {
			trace("main: nonlayersld = %s", nonlayersld);
		}
	}
	else {
		error(DISP_ERR, errno, "main: I_LOOK ioctl failed");
		fprintf(stderr, "Attempt to execute streams xt/layers over a non-streams device failed.\n");
#ifdef SVR32
		fprintf(stderr, "Use character layers over RS232.\n");
#endif /* SVR32 */
		exit(erNOTSTREAMS);
	}

	if (ffile)
		trace("startup file %s , fd %d", fpconf, fileno(fpconf));

	/* Optind points to the next argument, if any. The last argument     */
	/* index is argc - 1. 						     */
	/* When all arguments are exhausted by getopt, optind is set to argc.*/ 

	if (optind < argc-1) {
		error(DISP_ERR, 0, "extra arguments at end of command line");
		fprintf(stderr, usage, layerscmd);
		exit(erINVARG);
	}
	if (optind == (argc - 1)) 
		strcpy(dmdprog,argv[optind]);
	

	/* If the environment variable SHELL is not defined, the shell used  */
	/* is the BOURNE SHELL : sh					     */

	if ((myshell = getenv("SHELL")) != NULL && myshell[0] != '\0')
		strcpy(shell, myshell);


	/* Set all the signal catchers.
	 *
	 * Note that a SIGTERM or SIGHUP will cause read() in control()
	 * to return error. This will cause control() to return, which
	 * leads to layers cleaning up and then exiting.
	 */
	(void)signal(SIGTERM, sigcatch);
	(void)signal(SIGHUP, sigcatch);
	(void)signal(SIGALRM, sigcatch);
	if (!ignbrk)
		(void)signal(SIGINT, sigcatch);

	raw(1, &ttysave);		/* set stdout in raw mode 	     */

	/* save the current permissions on stdout and change it to read,write*/
	/* only by the current user					     */

	if( fstat(1, &ttystat) == SYSERROR ) {
		error(DISP_ERR, errno, "main: cannot fstat stdout");
		(void)ioctl(1, TCSETAW, &ttysave);
		exit(erSTATFAIL);
	}
	(void)chmod(ttyfile, S_IRUSR|S_IWUSR);

	/* load the dmd program indicated by the user			     */ 
	trace("main: loading the boot program %s", dmdprog);
	if ( boot(dmdprog) == FAIL ) {
		(void)ioctl(1, TCSETAW, &ttysave);
		exit(erDMBOOT);
	}

	(void)alarm(3);
	(void)pause();

	/* 
	   Fix added to take care of the following problem :

	   Stream Head sends a signal to the process group leader only if 
	   the signal is received on a cotrolling tty.
	   A controlling tty is the stream that is first opened by the process
	   group leader. 
	   Since the control channel is not a controlling tty, SIGHUP sent
	   on the control channel does not get sent to the process group 
	   leader.
	   The following code makes the layers process the process group 
	   leader before opening the control channel. 
	   SIGHUP is caught by layers and is then sent to the process group 
	   leader.
	*/

#ifdef SVR32
	sid = getpgrp();
	trace("Layers parent pgrp = %d", sid);
#endif /* SVR32 */
#ifdef SVR40
	sid = getsid(0);
  	myppid = getppid(); /* get parent id for hangup hmj */
	trace("Layers session sid = %d gid = %d", sid, myppid);
#endif /* SVR40 */

/* setsid() should have been the call for SVR4.0, but because of
 * a POSIX definition - a group leader cannot become a session
 * leader (which may be fixed in the future), doing a setsid() will
 * fail on job control shells (sh and ksh). Hence we are stuck with
 * using setprgrp() for SVR4.0. Note that getsid(0) has to be done
 * to get the session leader and not getpgrp().
 */
	if (stream) {    /* bl90-11502 */
		if ((setpval = setpgrp()) < 0) {
			error(UNBOOT, errno, "setpgrp failed - pid returned by setpgrp = %d", setpval);
			(void)ioctl(1, TCSETAW, &ttysave);
			exit(erSETPGR);
		}
	}

	trace("Layers pid returned by setpgrp/setsid setpval= %d", setpval);

	if ( (cntlfd = opencntlf()) == FAIL ) {
		(void)ioctl(1, TCSETAW, &ttysave);
		exit(erCTLOPN);
	}

	if (stream) {     /* bl90-08840  */
		/* Set the control channel to raw mode. This is necessary
		 * even though no LDTERM is pushed on the control channel,
		 * because it is necessary to inform xt of baud rate.
		 */
		if( ioctl(cntlfd, TCSETAW, &ttyraw) == SYSERROR ) {
			error(UNBOOT, errno, "main: initial ioctl of control chan failed");
			(void)ioctl(1, TCSETAW, &ttysave);
			exit(erCONTROLIOCTL);
		}

		/* Set encoding if required.
		 */
		if (Loadtype != BINARY_LOAD) {
			if (ioctl(cntlfd, XTIOCHEX, 0) == SYSERROR) {
				error(UNBOOT, errno, "Set encoding ioctl failed");
				(void)ioctl(1, TCSETAW, &ttysave);
				exit(erENCODE);
			}
			else {
				trace("LAN encoding enabled");
			}
		}
		rc=multiplex(cntlfd, 1);
	} else {
		if( ioctl(1, TCSETAW, &ttyraw) == SYSERROR ) {
			error(UNBOOT, errno, "main: initial ioctl of control chan failed");
			(void)ioctl(1, TCSETAW, &ttysave);
			exit(erCONTROLIOCTL);
		}
		rc=omultiplex(cntlfd, 1);
	}


#ifdef i386

	if (rc == SUCCEED) { 	/* set up the mux	     */
				/* link stdout under cntlfd 	     */
		/* no zombie processes can be created			     */
		if ( !stream ) { /* not sure we need this here - ams */
			ttyraw.c_cc[VMIN] = JMSGSIZE;
			(void) ioctl(cntlfd, TCSETAW, &ttyraw);
			if (!ignbrk) {
	                        struct termio ttybrk;
	
	                        (void)ioctl(1,TCGETA,&ttybrk);
	                        ttybrk.c_iflag = BRKINT;
	                        ttybrk.c_cc[VMIN] = 4;
	                        (void)ioctl(1,TCSETAW,&ttybrk);
			}
		}
#else
	if( multiplex(cntlfd, 1) == SUCCEED ) { /* set up the mux	     */
					/* link stdout under cntlfd 	     */
#endif

		if( traceon )
			if( ioctl(cntlfd, XTIOCTRACE, &Traces) == -1 ) {
				error(STORE_ERR, errno,
				    "initial XTIOCTRACE ioctl failed");
			}

		/* no zombie processes can be created			     */
		signal(SIGCLD, SIG_IGN);

 		if (tver != 5 || ttyp != 5)
 			switchproto(cntlfd, maxpktarg);

		/* execute the start-up script file			     */

		if( ffile ) {
			trace("main: reading layers -f file");
			readf(cntlfd);
		}

		/* keep reading the control channel till error or exit       */

		trace("main: reading the control channel");
		if (stream) {
			if (ioctl(cntlfd,I_SETSIG,S_HANGUP) < 0) {
				perror("I_SETSIG ioctl:");
				goto cleanup;
			}
			(void) signal (SIGPOLL,got_hangup);
		}
		control(cntlfd);

cleanup:
		/* Since I am already terminating, I have no use for these
		 * signals. Besides, if one gets received now it can
		 * confuse the termination process.
		 */
		(void)signal(SIGTERM, SIG_IGN);
		(void)signal(SIGHUP, SIG_IGN);

		/* For some reason, when running ksh, the "kill -1" in
		 * killcheck() for the first ksh termshells() attempts to
		 * undo will fail if the kill is sent too quickly. This
		 * seems to happen on the faster 3B2s like the 600 and
		 * 700, but not on the slower 3B2/400. Possibly it is some
		 * interaction with a stream message from xt. I am not
		 * sure, but I do know that I am gladly willing to wait one
		 * extra second for layers to exit to get rid of this
		 * stupid problem.
		 */
		(void)alarm(1);
		(void)pause();

		trace("main: terminating all windows");
		termshells();		/* terminate all parent shells       */

		/* send JTERM to the XT driver : XT driver returns an*/
		/* ACK and passes this down to the terminal.	     */

		(void)write(cntlfd, (char *) & Termesg, JMSGSIZE);

		(void)alarm(2);
		(void)pause();

		/* Get the final xtrace and xtstats if necessary.
		** Save them for display after the mux is dismantled.
		*/
		if( traceon ) {
			if( ioctl(cntlfd, XTIOCTRACE, &Traces) == SYSERROR )
				error(STORE_ERR, errno, "XTIOCTRACE ioctl failed");
		}
		if( statson ) {
			if ( ioctl(cntlfd, XTIOCSTATS, Stats) == SYSERROR )
				error(STORE_ERR, errno, "XTIOCSTATS ioctl failed");
		}

		/* Dismantle the mux.
		 */
		trace("main: unlinking mux with mux_id : %d", mux_id);

#ifdef i386
		if(stream && (ioctl(cntlfd, I_UNLINK, mux_id) == SYSERROR))
#else
		if( ioctl(cntlfd, I_UNLINK, mux_id) == SYSERROR )
#endif

			error(STORE_ERR, errno, "I_UNLINK ioctl failed");

		exitval = 0;
	}
	else {
		exitval = erMUXOPN;
	}
	
	/* SIGCLD is previously set to SIG_IGN so that zombie processes
	 * will not be created by children of shells forked by doexec()
	 * when windows are deleted. Resetlogin(), however, requires
	 * SIGCLD be set to SIG_DFL. At this point we are not going to
	 * call doexec() anymore, so we can just set SIGCLD back to
	 * SIG_DFL.
	 */
	signal(SIGCLD, SIG_DFL);

	/* Change utmp entry back to tty.
	 */
	cntlf[CHANIND] = EOS;
	(void)resetlogin(cntlf, ttyfile);

	/* Reset the line.
	 */

#ifdef i386
	if (stream)
		if (ioctl(1, I_PUSH, layersld) == SYSERROR)
			error(DISP_ERR, errno, "I_PUSH of ldterm failed");
#else
	if (ioctl(1, I_PUSH, layersld) == SYSERROR)
			error(DISP_ERR, errno, "I_PUSH of ldterm failed");
#endif

	if (ttcompat_pushed == 1) {
		if (ioctl(1, I_PUSH, "ttcompat") == SYSERROR)
			error(DISP_ERR, errno, "I_PUSH of ttcompat failed");
	}

	/* change terminal parameters to what they were       		     */
	(void)ioctl(1, TCSETAW, &ttysave);

	/* print the errbuf if necessary */ 
	if (use_errbuf && errbuf[0] != '\0') {
		printf("The following error was encountered by the layers\n");
		printf("command at time %s", ctime(&errtime) );
		printf("during the layers session which just ended:\n\n");
	 	printf("%s\n", errbuf);	
	}

	if( traceon )
		xtraces(stderr);

	if( statson )
		xtstats(stderr);

	(void)close(cntlfd);	

	/* change permissions on the tty device back to what they were	     */
	(void)chmod(ttyfile, ttystat.st_mode & S_IAMB);
	trace("main: layers returns");

	if (fSIGHUP) {
		trace("sending kill to session sid = %d", sid);
		kill(-sid, SIGHUP);
 		kill(myppid, SIGHUP); 
	}

	exit(exitval);
}


/* Open the layers -f file.
 * Returns the file pointer of the file opened or NULL in case of failure.
 */
FILE *
openffile(ffile)
char *ffile;			/* name of the config file to be opened */
{
	FILE	*fp;		/* file pointer for the opened file */ 

	if( access(ffile, 04) != SYSERROR )
		if ((fp = fopen(ffile, "r")) != (FILE *)NULL )
			return(fp);

	error(DISP_ERR, 0, "Cannot open layers -f file '%s'", ffile);
	return(NULL);
}


/* open the control channel						     */
/* /dev/xt/ij0 is the file opened where ij follows the following sequence    */
/* 00-09 10-19. Different terminals will have different control channels.    */ 

/* RETURNS : The file descriptor of the opened control channel or FAIL in    */
/* 	     case of failure.						     */

int
opencntlf()
{
	register int fd;	/* file descriptor of the control channel    */
	register int i;		/* index into the control file name string   */
	struct stat statbuf;	/* buffer for keeping status of control file */
	struct group *gp;

	/* The effective user id must be that of root			     */

	if( geteuid() != 0 ) {
		error(UNBOOT, 0, "layers must be mode set user ID to root");
		return(FAIL);
	}

	/* Start with the index pointing at j. Note the subtle difference    */
	/* between sizeof and strlen : sizeof is strlen + 1. Also note that  */
	/* sizeof is used here to avoid dynamic calculation of strlen.       */
 
#ifdef i386 
	if (! stream)
		strcpy(cntlf, "/dev/xto000");
#endif

	i = sizeof(cntlf) - 3;		

	for (;;)
		if( (fd = open(cntlf, O_RDWR | O_EXCL)) == SYSERROR ) {

			trace("opencntlf: couldn't open channel %s errno %d", cntlf, errno);

			if(errno == ENOENT) {
				/* ENOENT means no such file or directory.
				 * In other words, we tried every existing
				 * control channel.
				 */
				error(UNBOOT, 0, "No spare xt/layers channels");
				return(FAIL);
			}
			else {
				/* Increment the channel number.
				 */
				if( cntlf[i] == '9' ) {
					cntlf[i-1]++;
					cntlf[i] = '0';
				} else
					cntlf[i]++;

				/* And try the next one.
				 */
				continue;
			}
		}
		else { /* open succeeded */

			uid_t uid = getuid();	
#ifdef SVR32
			gid_t gid = getgid();
#endif /* SVR32 */
#ifdef SVR40
		gid_t gid_user = getgid();
		/* 
		 * Set the group of all the channel (except control) to "tty". 
		 * First try the decent way to get tty_gid, otherwise use brute
		 * force (hard coded value!).
		 */

		gid_t gid = 7; /* "tty" group id */

		if ((gp = getgrnam("tty")) == NULL) {
                	trace("no group entry for <tty>, default is used");
		}
        	else {
                	gid = gp->gr_gid;
			trace("tty group id gid=%d",gid);
		}
		

#endif /* SVR40 */

			trace("opencntlf: Opened channel %s", cntlf);

			i++;		/* index now points to the window no.*/

			/* Make the caller the owner of the new channels
			 * including the control channel, and set the proper
			 * modes. The control channel is always set to
			 * mode 0600 since there is no reason that another
			 * users should have to write to the control channel
			 * and this could be a security hole. All the other
			 * channels are changed to the mode of the original
			 * tty as saved in the ttystat structure in main().
			 */
			while ( chown(cntlf, uid, gid) != SYSERROR ) {
				if(cntlf[i] == '0') /* control channel */
					(void)chmod(cntlf, S_IRUSR|S_IWUSR);
					else	 {	    /* other channels */
#ifdef SVR32
						(void)chmod(cntlf, ttystat.st_mode&S_IAMB);
#endif /*SVR32*/
#ifdef SVR40
						(void)chmod(cntlf, 0620);
#endif /* SVR40 */
					}

				if( cntlf[i]++ == '7' )
					break;
			}
			if( (cntlf[i] != '8') && (errno != ENOENT) ) {
				trace("opencntlf: can't chown '%s' errno %d", cntlf, errno);
				(void)close(fd);
				fd = FAIL;
			} else {
				/* reset the last byte to indicate control ch*/ 
				cntlf[i] = '0'; 

				/* set the real and effective uid and gid    */
#ifdef SVR32
				if( (setgid(gid) == SYSERROR)||
#endif /* SVR32 */
#ifdef SVR40
				if( (setgid(gid_user) == SYSERROR)||
#endif /* SVR40 */
				    (setuid(uid) == SYSERROR) ) {
					trace("opencntlf: can't setgid/setuid errno %d", errno);
					(void)close(fd);
					fd = FAIL;
				}
				break; /* we made it */
			}
		}

	/* push a list of module specified by -M option 	      */
	/* on the control channel for debugging 		      */

	/* meant to push dump module which is an undocumented 	     */
	/* STREAMS tool 					     */
			
#ifdef i386
	if(stream && dumpon) {
#else
	if(dumpon) {
#endif

		for( dumpld = strtok(pdmplist,","); dumpld != NULL;
		     dumpld = strtok(NULL,",") ) {
			if( ioctl(fd, I_PUSH, dumpld) == SYSERROR )
				trace("opencntlf: PUSH of %s on new chan failed: errno %d\n", dumpld, errno);
		}
	}

	return(fd);
}


/* 
   Loads the dmd program onto the terminal. The action taken depends on the 
   various variables. Since there are too many variations, the logic is laid
   out beforehand.

   1. If the program was not specified as the last parameter of the layers 
      command, the following program is used :

      if DMD is set
      $DMD/lib/layers/lsys.8;<type>;<version> 
      else
      /usr/lib/layers/lsys.8;<type>;<version> 
   
      <type> and <version> are obtained by querying the terminal.


   The following steps are taken if the dmd program does not exist :

   2. Returns error if the program was specified and does not exist.

   3. Type 7, version <= 4 requires a non-zero load. Therefore, error is 
      returned if the program does not exist (default or specified dmd prog).

   4. If the program was not specified and does not exist, the following action
      is taken :

	If DMD is not set, encoding is set in the terminal using :

	ESC[2;2v for encoding set
	ESC[2;0v for no encoding 

	and control is returned

	If DMD is set, the program is changed to the following :

	$DMD/lib/layers/lsys.8;<type>;?

	If this program cannot be read, error is returned.

   The following steps are taken if the dmd program exists :
	

   5. If DMD is set, the loader program used is the following :

	$DMD/bin/wtinit

      If access is not allowed on this program, the following program is used :

        $DMD/bin/32ld

      If access is not allowed on this program, error is returned.

   6. If DMD is not set, the loader program used is the following :

         /usr/lib/layersys/wtinit

   7. If access is allowed for the version mentioned in 3 above, the following
         program is downloaded first :

	 /usr/lib/layersys/set_enc.j

      This program sets encoding for the terminal.

   8. In the end, the dmd program is downloaded using the following command :

	loader [-p] [-d] <dmd prog> 


   Return Values : SUCCEED/FAIL
*/


int
boot(prog)
char *prog;			/* booting progaram to be downloaded	     */
{
	char *dmdenv;		/* preserves the environment variable : DMD  */
	char command[MAXSTR];   /* command string for downloading	     */ 
	char loader[MAXSTR];	/* loader used to download the booting prog  */
	bool supplied;		/* is the dmd program name supplied(Non-Null)*/
	bool v1;		/* version 1 terminal : type = 7,version <= 4*/ 
	bool DMDset;		/* is the DMD environ variable set ?	     */
	bool exists;		/* does the dmd program exist?		     */
	bool empty;		/* is the file size  of dmd prog 0 ?         */

	struct stat statbuf;

	if( check_term(&ttyp, &tver) == FAIL )
		return(FAIL);

	trace("terminal type = %d, version = %d", ttyp, tver);

	v1 = ((ttyp == 7) && (tver <= 4));
	supplied = (prog[0] != '\0');
	dmdenv = getenv("DMD");
	DMDset = ((dmdenv != NULL) && (dmdenv[0] != '\0'));
	if (!supplied) {
		sprintf(prog, "%s/lib/layersys/lsys.8;%d;%d",
			DMDset ? dmdenv : "/usr", ttyp,tver);
	}
	exists = ( stat(prog, &statbuf) != -1 );
	empty = ( statbuf.st_size == 0 );

	trace("DMD %s set; boot prog %s supplied",
		DMDset ? dmdenv : "not", supplied ? "" : "not");
	trace("dmd prog %s", prog);
	trace("dmd prog %s exists, %s empty", 
		exists ? "" : "not" ,  empty ? "" : "not");

	if( supplied && !exists ) {
		error(DISP_ERR, errno, "can't load terminal program %s", prog);
		return(FAIL);
	}
	if( v1 && (!exists || empty) ) {
		error(DISP_ERR, 0,
		    "5620 version 1 firmware requires non-zero download: %s",prog);
		return(FAIL);
	}
	if( !DMDset ) {
		/*
		 *  Use Generic Windowing package
		 */
		if( !exists || empty ) {  /* && not v1 */
			/* default init */
			write(1, Loadtype ? "\033[2;2v" : "\033[2;0v", 6);
			return(SUCCEED);
		} else {
			(void)strcpy(loader, "/usr/lib/layersys/wtinit");
		}
	} else {
		/*
		 *  Use Terminal Feature Package
		 */
		if( !exists ) {  /* && not supplied */
			prog[strlen(prog) - 1] = '?';
			if( access(prog, 4) == SYSERROR ) {
				error(DISP_ERR, 0, "terminal id unknown to $DMD software %s",prog);
				fprintf(stderr, "\r\n\
    Your $DMD variable is pointing to a terminal specific host\r\n\
    software package that does not know about your terminal.\r\n\
    Either change your $DMD variable to point to the proper host\r\n\
    software package or unset your $DMD variable with the\r\n\
    command 'unset DMD'.\r\n");
				return(FAIL);
			}
		}
		(void)sprintf(loader,"%s/bin/wtinit", dmdenv);
		if( access(loader, 1) == SYSERROR ) {
			trace("boot: loader changed to %s/bin/32ld", dmdenv);
			(void)sprintf(loader, "%s/bin/32ld", dmdenv);
		}
	}

	if( access(loader,1) == SYSERROR ) {
		error(DISP_ERR, errno, "executing file: '%s'", loader);
		return(FAIL);
	}

	if( v1 ) { 	/* 1.1 or 1.2 5620 firmware */
		trace("boot: v1 set");
		(void)sprintf(command, "%s /usr/lib/layersys/set_enc.j", 
				loader);
		trace("boot: command for setting encoding : %s", command);
		if( system(command) ) {
			error(DISP_ERR, errno,
			    "can't download /usr/lib/layersys/set_enc.j");
			return(FAIL);
		}
		fflush(stdout);
		sleep(2);
		printf("\n\n\n\n\t\tPlease stand by.  Downloading layers...\r\n");
		printf("\n\t\t(May not be visible for several seconds) ");
		fflush(stdout);
		sleep(3);
	}

	(void)sprintf(command,"%s -l %s %s '%s'", loader, sld ? "-p":"", 
			dbg ? "-d" : "", prog);
	trace("boot: loader command : %s", command);
	if( system(command) ) {
		error(DISP_ERR, errno, "can't load '%s'", prog);
		return(FAIL);
	}
	return(SUCCEED);
}

/*
   Finds the encoding mode to be used. It sends the following string to the 
   terminal :
   
   ESC[c		: Request terminal type and version 

   and expects the following response :

   ESC[?8;<type>;<version>c 

   For type != 7 and version >= 5, the following action is taken :

   Send   : ESC[F		 : Request encoding

   Expect : ESC[<encoding flag>F : encoding flag is non-zero for enabling 
				   encoding

   RETURNS : SUCCEED/FAIL
*/


int
check_term(type, vers)
int *type;				/* pointer to terminal type 	     */
int *vers;				/* pointer to version of terminal    */
{
	char *enc_env;			/* encoding environment variable     */	
	int enc_flg;			/* encoding response from terminal   */

	if( ((enc_env = getenv("DMDLOAD")) != NULL) && 
		(strcmp(enc_env,"hex") == 0) )
		Loadtype = HEX_LOAD;

	if (enq_term("\033[c", "\033[?8;%d;%dc", type, vers) != 2) {
		error(DISP_ERR, 0,
		    "The terminal failed to respond to a request for terminal ID.\
 Either the terminal is not an XT windowing terminal or input was garbled.");
		return(FAIL);
	}
	if( (*type == 7) && (*vers <= 2) ) {
		error(DISP_ERR, 0, "obsolete 5620 firmware version");
		return(FAIL);
	}
	if( (*type != 7) || (*vers >= 5) ) {
		if (enq_term("\033[F", "\033[%dF", &enc_flg) != 1) {
			error(DISP_ERR, 0, "invalid encoding response from terminal = %x",
				enc_flg);
			return(FAIL);
		}
		if( enc_flg || Loadtype )
			Loadtype = ENC_ENABLE;
	}
	return(SUCCEED);
}

/* Send the enq string to the terminal. Get the response as indicated by resp*/
/* The function uses variable no. of arguments as indicated by va_alist	     */

/* Assumptions : first character in response is an ESC character 	     */

/* RETURNS : erTIMOUT or the output of fscanf. This output, in case of	     */ 
/* 	     success, will be the no. of matching variables before EOF is    */
/*	     reached.                                                 	     */

/* NOTE    : The code will have to be modified if more than two variables are*/
/*		needed							     */

int
enq_term(enq, resp, val1, val2)
char *enq;				/* the enq string sent to terminal   */
char *resp;				/* the response string from terminal */
int  *val1;
int  *val2;
{
	register int retval = 0;	/* return value for function	     */
	int	 c;			/* The first character of response   */
	int	*pval;			/* pointer to val array		     */	
	int	ival = 0;		/* val1 or val2 ? 		     */ 

	/* assume raw(1) executed before this routine called 		     */
	write(1, enq, strlen(enq));
	/* assume sigcatch(SIGALRM) executed before this routine 	     */

	alarm(15);	/* value increased from 7 to 15 after customer report*/

	/* The terminal may be set for 7 bits per character with parity      */
	/* enabled. In such cases, the character is received with a parity   */
	/* bit. This parity bit should be stripped off.			     */

	/* Skip all characters till an ESC is recognised            	     */

	while ( (c = getchar()) != SYSERROR )  
		if (CHAR(c) == *resp)
			break;

	alarm(0);
	if (c == SYSERROR) 
		return(erTIMOUT);
	
	resp++;

	for (;;) {
		alarm(15);	

		switch(*resp) {

		  case EOS :
			break;


		  case '%' :
			resp += 2;		/* assume "%d" and skip d    */
			pval = (ival++ == 0) ? val1 : val2;
			switch (*pval = srchint()) {

			  case erTIMOUT :
			  case FAIL :
				retval = *pval; 
				break;

			  default : 		/* a valid int was recognised*/
				break;
			}
			retval++;		/* no. of matching variables */
			alarm(0);
			continue;

		  default :
			if ((c = getchar()) == SYSERROR) {
				retval = erTIMOUT;
				break;
			}
			if (CHAR(c) != *resp)
				/* ignore control characters in response     */
				if (isprint(CHAR(c))) {
					error(DISP_ERR, 0,
					    "terminal response mismatch. Expected  %c, Received %c",
					    *resp, CHAR(c));
					retval = FAIL;
					break;
				} else
					trace("enq_term: control char in response - %x", CHAR(c));
			else {	/* a match was found			     */
				resp++;
				alarm(0);
				continue;
			}
				
		}
		break;
	}
			
	alarm(0);
	return(retval);
}


				
int
srchint()
{
	int	c;		/* character received from the terminal      */
	char	digit[5];	/* expected maximum size 		     */
	char	*pvar;		/* pointer to ascii value of integer         */
	char	*plimit;	/* limit of character storage 		     */
	int	retval = SUCCEED; /* return value			     */

	pvar = digit;
	plimit = pvar + sizeof(digit);

	alarm(0);
	alarm(15);	
	while ((c = getchar()) != SYSERROR) {
		if (!isdigit(CHAR(c))) {
			/* ignore control characters			     */
			if (isprint(CHAR(c))) {
				if (ungetc(c, stdin) == EOF) {
				       error(DISP_ERR, 0, "failed to put char on stream");
					retval = FAIL;
				}
				break;
			} else
				trace("srchint: control char in response %x", CHAR(c));
			
		}
		if (pvar == plimit) {
			retval = FAIL; 
			break;
		}
		*pvar++ = CHAR(c);
		alarm(0);
		alarm(15);	
	}
	alarm(0);	
	*pvar = EOS;
	if (c == SYSERROR)
	   	return(erTIMOUT);
	else 
		return(retval == FAIL ? FAIL : atoi(digit));
}

/* pops the module existing over the tty driver(expected to be ldterm) and   */
/* links the tty driver under the streams XT driver(the control channel).    */

/* RETURN VALUES : SUCCEED/FAIL						     */
 

int
multiplex(xtfd, ttyfd)
int xtfd;			/* file descriptor of xt : the MUX	     */
int ttyfd;			/* file descriptor of the tty to be linked   */
				/* under the mux			     */
{
	register int i;				/* loop counter		     */
	int num_eagain = 0;

	trace("multiplex: xtfd = %d ttyfd = %d", xtfd, ttyfd);

	/* If the ttcompat ioctl compatibility module is pushed, we need
	 * to I_POP it and then remember what was under it.
	 */
	if(ttcompat_pushed == 1) {
		if( ioctl(ttyfd, I_POP, 0) == SYSERROR ) {
			error(UNBOOT, errno, "multiplex1: I_POP ioctl failed");
			return(FAIL);
		}
		if( ioctl(ttyfd, I_LOOK, nonlayersld) == SYSERROR ) {
			error(UNBOOT, errno, "multiplex: I_LOOK ioctl failed");
			exit(FAIL);
		}
		trace("multiplex: nonlayersld = %s", nonlayersld);
	}

	/* I_POP the line discipline.
	 */
	if( ioctl(ttyfd, I_POP, 0) == SYSERROR ) {
		error(UNBOOT, errno, "xtmultiplex2: I_POP ioctl failed");
		return(FAIL);
	}

	(void)alarm(LINKTIMO);

	for (;;) {
	    if ( (mux_id = ioctl(xtfd, I_LINK, ttyfd)) == SYSERROR ) {
		(void)alarm(0);
		switch( errno ) {

		  case EAGAIN: /* STREAMS buf alloc or NMUXLINK failure; go again*/
			if(++num_eagain == 5) {
				error(UNBOOT, 0,
				    "Out of STREAMS resources ");
				return(FAIL);
			}
		        trace("multiplex: Out of STREAMS resources: Trying again");
			(void)alarm(2);
			(void)pause();
		        continue; 

		  case EIO:

		        /* Error in I/O : try another ioctl : see if it works*/ 
		        for( i = 0; i < 2 ; i++ ) {
				(void)alarm(2);
				(void)pause();
				if( ioctl(xtfd, JTIMOM, 0) != SYSERROR )
					break;
			}
			if (i == 2) { /* JTIMOM also did not succeed          */ 
				error(UNBOOT, errno,
				    "JTIMOM failed after failed link : %s", cntlf);
				return(FAIL);
			}
			break;

		  default:	
			error(UNBOOT, errno, "linking MUX : %s", cntlf);
			return(FAIL);
		}	/* end of switch				     */
		break;  /* control passed only from EIO on success	     */
	     } else {  	/* succeeded in linking				     */ 
		trace("multiplex: mux_id : %d", mux_id);
		break;
	     }
	}		/* end of forever loop				     */

	(void)alarm(0);
	return(SUCCEED);
}

#ifdef i386
/* Multiplexor routine for non-STREAMS based devices */
int
omultiplex(xtfd, ttyfd)
int	xtfd;
int	ttyfd;
{
	struct xtioclm lm;
	register char	*	e1;
	register char	*	e2;
	register int	i;
	short	int cmd;
	struct	stat	s;

	trace("multiplex entered, xtfd=%d ttyfd=%d", xtfd, ttyfd);

	if (Loadtype != BINARY_LOAD)
		cmd = HXTIOCLINK;		/* "hex-mode" */
	else
            	cmd = XTIOCLINK;

        lm.fd = ttyfd;
	lm.nchans = MAXPCHAN;
	(void)alarm(LINKTIMO);

        if ( ioctl(xtfd, cmd, lm) == SYSERROR ) {
		(void)alarm(0);
		e2 = cntlf;

                switch ( errno ) {
		case EINTR:
                        e1 = "timeout - no response after %d seconds";
                        e2 = (char *)LINKTIMO;
                        errno = FAIL;
                        break;
		case EINVAL:
			e1 = "bad arg for link ioctl, call a DMD administrator";
			break;
		case ENOTTY:
			e1 = "not connected to a tty device";
			e2 = ttyfile;
			break;
		case ENXIO:
			e1 = "'linesw' not configured for 'xt' driver, call a DMD administrator";
			break;
		case ENOMEM:
			e1 = "no memory for kernel configuration, try again";
			break;
		case EBUSY:
			e1 = "multiplex pre-empted, call a DMD administrator";
			break;
		case EIO:
			for ( i = 0 ; i < 2 ; i++) {
                                (void)alarm(2);
                                (void)pause();
                                if ( ioctl(xtfd, JTIMOM, 0) != SYSERROR )
                                        break;
			}
			if ( i == 2 ) {
                                e1 = "system out of clists, try again";
                                break;
			}
			return SUCCEED;
		default:
			e1 = "unknown link ioctl error, call a DMD administrator";
			break;
		}
		error(e1, e2);
		return FAIL;
	}

	(void)alarm(0);
	return SUCCEED;
}
#endif 

/* switchproto: Ask the terminal what xt protocol it wants. Handle
** transition to network xt or larger packet regular xt if required.
*/
switchproto(cntlfd, maxpktarg)
int cntlfd;
unsigned char maxpktarg;
{
	unsigned char maxpkt;

	/* bbuf declaration from libwindows.c */
	extern union	bbuf	{
		struct	agentrect ar;
		char	buf[32];
	};
	extern union	bbuf	ret;

	/* ask terminal what protocol to use */
	if( ioctlagent(cntlfd, A_XTPROTO, 1,0,0,0, (short)0) ) {
		/* failure - firmware does not understand A_XTPROTO so
		** abort this attempt to switch protocols and just stay
		** with default 32 byte packets.
		*/
		trace("switchproto: Old firmware didn't understand A_XTPROTO");
		return;
	}

	maxpkt = ret.ar.r.origin.x;
	if(maxpkt != 1 && (maxpkt < 32 || maxpkt > 252)) {
		error(STORE_ERR, 0, "Illegal maxpkt from terminal");
		return;
	}

	trace("A_XTPROTO response is %d", maxpkt);

	/*
	** The following has several functions. First, it allows maxpktarg
	** (the -m option) to override network xt. This could be
	** necessary if a user starlans to a 3b2 and then cu's to
	** a remote machine over rs232. Second, it implements the
	** normal usage of maxpktarg where the user can change packet
	** size if the terminal can handle the size requested.
	**
	** This also adjusts maxpkt to baud rate if maxpktarg is not
	** specified. The default maxpkt at 9600 baud or higher is 128
	** bytes rather than the maximum possible size of 252 bytes. This
	** is because with 252 bytes, interactive response to a ^D or ^F in
	** vi appears a little slow because it takes a whole 252/960 of a
	** second for the entire first packet to arrive. Testing shows that
	** 128 bytes packets are about the same performance as 252 byte
	** packets, so a lower default was chosen to improve interactive
	** response.
	**
	** Packet size is lowered even further for slower baud rates. This
	** is necessary because at lower baud rates output gets too bursty
	** with larger packets. Also, this is necessary to keep
	** retransmission timeouts after errors reasonable at lower baud
	** rates. Besides, at lower baud rates the performance improvement
	** of larger packets is less important.
	*/
	if(maxpktarg || maxpkt != 1)  {
		if(maxpktarg) {
			if(maxpkt == 1 || maxpktarg <= maxpkt) {
				/* use maxpktarg if possible */
				maxpkt = maxpktarg;
			} /* else maxpktarg > maxpkt, so use maxpkt */
		}
		else {
			/* adjust maxpkt to baud rate */
			if(maxpkt >= 128)
				maxpkt = 128;
			if((int)(ttysave.c_cflag&CBAUD) < B9600 && maxpkt >= 64)
				maxpkt = 64;
			if((int)(ttysave.c_cflag&CBAUD) < B1200)
				maxpkt = 32;
		}
	}

	/* The following because it is difficult to dynamically switch
	** from encoding to network xt. If the user sets DMDLOAD=hex,
	** give them encoded regular xt, which is actually what they
	** asked for anyway.
	*/
	if(maxpkt == 1 && Loadtype != BINARY_LOAD) {
		trace("switchproto aborted: DMDLOAD=hex overrides network xt");
		return;
	}

	trace("JXTPROTO sending maxpkt = %d", maxpkt);

	/* Tell the driver the new packet size.
	*/
	if( ioctl(cntlfd, JXTPROTO, &maxpkt) == -1 ) {
		error(STORE_ERR, errno, "JXTPROTO ioctl failed");
		(void)ioctl(1, TCSETAW, &ttysave);
		exit(erJXTPROTO);
	}

	/* Must do JTIMOM again after changing packet size to adjust
	** both the host and terminal timeouts to new packet size.
	*/
	if( ioctl(cntlfd, JTIMOM, 0) == -1 ) {
		error(STORE_ERR, errno, "JTIMOM ioctl in switchproto failed");
		(void)ioctl(1, TCSETAW, &ttysave);
		exit(erJTIMOM);
	}
}


/*
 * readf - read the layers -f file.
 *
 * Each line of this file should have the following format:
 *
 * x1 y1 x2 y2 command 
 *
 * where  x1 is the x coordinate for the top left corner of the new window 
 *	  y1 is the y coordinate for the top left corner of the new window
 *        x2 is the x coordinate for the bottom right corner of the new window 
 *	  y2 is the y coordinate for the bottom right corner of the new window
 *
 *	  command is a string that is passed to shell. 
 *
 * e.g.
 *
 * 8 8 700 200 date ; pwd ; exec $SHELL
 *
 * NOTE :
 *
 * 1. Valid coordinates for 630 :
 *
 * 	6,6 to 1018, 1018
 *
 * 2. Coordinates outside the above range are clipped.
 *    If all coordinates are 0's, the user will be prompted to create the 
 *    windows at run time.
 * 
 * 3. Window must be at least as large as the minimun (32x32) window size. 
 *
 * 4. The command is executed by layers as follows :
 *
 *    $SHELL -i -c command
 *
 * 5. If the environment variable SHELL is not defined,  bourne sh is used.
 *
 *
 * Note that older versions of this function would create a window, run the
 * command for that window, create the second window, run the command for
 * the second window, etc.. Now the function first creates all windows and
 * then runs the commands for all windows. The reason for this change is
 * because there was a problem if the first window created finished
 * downloading a program which resulted in a local window and therefore
 * deleted the channel used to download the local window before the last
 * window was processed. The last window would re-use the channel of the
 * first window. Then when readf() returned and control() was called,
 * control() would find a C_DELETE sitting around for the channel which
 * created the local window, and control would end up deleting the process
 * in the last window by mistake. What a mess, but I prefer the user
 * interface of creating the windows first anyway.
*/
readf(fd)
int fd;					/* file descriptor of the config file */
{
	int x1, y1, x2, y2;		/* x,y coordinates for the window */
	int lastchan = EOF;		/* last channel	that was opened */
	int nmatch;			/* no. of matching items */
	char string[MAXSTR];		/* array for reading the input line */
	char *pstring;			/* temp pointer to string[] */
	int line;			/* line number in -f file */
	int savecmdnum=0;		/* current entry in savecmd structure */
	int i;

	/* Structure to save commands from -f file. Note that MAXPCHAN
	 * is defined to be 8 in nxt.h. There are MAXPCHAN-1 entries
	 * of this structure because the control channel has no window
	 * so the max number of windows is 7.
	 */
	struct savecmd {
		int chan;		/* channel for this command */
		char command[MAXSTR];	/* command portion of the line */ 
	} savecmd[MAXPCHAN-1];


	/* First, read the -f file and create the windows.
	 */
	for(line = 1 ; fgets(string, MAXSTR, fpconf) != NULL ; ++line) {

		if(savecmdnum == MAXPCHAN-1) {
			error(STORE_ERR, 0,
			    "Executing layers -f file - too many windows specified");
			break;
		}

		/* Skip blank lines and lines starting with the shell
		 * comment character '#'.
		 */
		pstring = string;
		while( isspace(*pstring) )
			++pstring;
		if(*pstring == '\0' || *pstring == '#')
			continue;
		
		/* Scan and process the string.
		 */
		nmatch = sscanf(pstring, "%d %d %d %d %[^\n]", &x1, &y1, 
				&x2, &y2, savecmd[savecmdnum].command);
		if( nmatch == 5 ) {
			if( (savecmd[savecmdnum].chan = Newlayer(fd, x1, y1, x2, y2)) == EOF ) {
				error(STORE_ERR, 0,
				    "Executing layers -f file - window creation failed");
				break; 
			}
			lastchan = savecmd[savecmdnum].chan;
			++savecmdnum;
		}
		else {
			/* Only this line is ignored. Rest of them could
			 * still be valid.
			 */
			error(STORE_ERR, 0,
			   "Bad format line %d in layers -f file: Line ignored", line);
		}
			
	}

	/* Run the specified commands in the created windows.
	 */
	for(i = 0 ; i < savecmdnum ; ++i)
		(void)doexec(savecmd[i].chan, savecmd[i].command);

	/* Make the last channel that was successfully opened the current
	 * windows.
	 */
	if (lastchan != EOF) {
		trace("readf: last chan = %d", lastchan);
		Current(fd, lastchan);
	}

	/* No more use for this file pointer.
	 */
	fclose(fpconf);
}


/* This function keeps reading messages from the control channel till it     */ 
/* receives an exit command (the user trying to exit layers) or an invalid   */
/* command. It will also exit if it is interrupted by a signal that is not   */
/* caught by layers (more generally, if the read returns with a failure).    */
/* When the terminal is switched off (DTR drops), an M_HANGUP message is sent*/
/* by the tty driver. This message is sent on the control channel. Since     */
/* layers is not the process group leader for the control channel, it does   */
/* not receive the SIGHUP signal. It is received instead by the parent shell.*/
/* The read, nevertheless, returns with a failure and this procedure exits.  */
void
control(fd)
int fd;				/* file descriptor of the control channel    */
{
	struct jerqmesg rbuf;	/* every command received has the following  */
				/* format : <command> <channel no.> data     */

	if (sigsetjmp(jmpenv,1))  /* use arg 1 to save signal mask */
		return; /* caught SIGPOLL ==> HANGUP on control chan */

	while( read(fd, (char *)&rbuf, JMSGSIZE) == JMSGSIZE ) {
		trace("control: control command %d on channel %d", rbuf.cmd,
				 rbuf.chan);
		switch( rbuf.cmd ) {
		  case C_NEW:		/* Create a new window 		     */
			doexec(rbuf.chan, shell);		
			continue;

		  case C_DELETE:	/* Delete a window		     */	
			undoshell(rbuf.chan);	
			continue;

		  case C_RUN:		/* Run a system command 	     */
			dorun(fd, rbuf.chan);
			continue;

		  case C_EXIT:		/* Exit from layers		     */	
			return;

		  default:		/* Bad control command 		     */	
			error(STORE_ERR, 0, "control: bad control command '%x'", rbuf.cmd );
			return;
		}
	}

	trace("control: returns. Control channel read %s", cntlf);
}


/* Saves the current value of the terminal parameters in ttyp and changes it */
/* to 8 bits per char, disable parity, 1 stop bit. It preserves the existing */
/* baud rate and line type (local or dial-up) and enables the receiver       */
/* Refer to termio(7).							     */

void
raw(fd,ttyp)
int fd;			/* file descriptor of the tty to be set in raw mode  */
struct termio *ttyp;	/* pointer where existing termio structure is saved  */
{
	(void)ioctl(fd, TCGETA, ttyp);

	if (!ignbrk)
	   ttyraw.c_iflag = BRKINT;	/* signal interrupt on break	     */
	ttyraw.c_lflag = 0;
	ttyraw.c_oflag = 0;
	ttyraw.c_cflag = (ttyp->c_cflag & (CBAUD|CLOCAL)) | CS8 | CREAD;

	ttyraw.c_cc[VMIN] = 1;		/* it is not meaningful right now    */
	(void)ioctl(fd, TCSETAW, &ttyraw); /* change device to raw mode	     */
}


/* This procedure just reads the string that follows the C_RUN command and   */
/* execs it. This string is intended as a shell command.		     */

void
dorun(fd,chan)
int fd;		/* file descriptor of channel in which the command is run    */ 
int chan;	/* channel in which the command is executed		     */
{
	register char *myshell;		/* the shell to be execed in chan    */
	char command[TTYHOG+1];		/* command to be executed  in cha    */
	char c;				/* character read from control chan  */
	char *pcmd;			/* pointer to command bytes 	     */

	pcmd = command;
	c = '\01';
	while ( c != '\0'){
		if (read(fd, &c, 1) != 1) {
			error(STORE_ERR, errno, "error in reading control channel");
			return;
		}
		*pcmd++ = c;
	}
 	*pcmd = '\0';
        
	/* The previous way of implementation will cause the system to hang  */
	/* because the process is run as sh -ic command. When that command   */
	/* execution is complete, there is no controlling shell left. The    */
	/* problem is similar to the startup file. The last command should   */
	/* typically be an exec $SHELL to allow a controlling shell to remain*/
	/* on the window.						     */

	strcat(command, ";exec ");
	if ((myshell = getenv("SHELL")) != NULL && myshell[0] != '\0') 
		strcat(command, myshell);
	else 
		strcat(command, "sh");
	trace("dorun: command '%s'", command);
	doexec(chan,command);
}



/* Execs sh -ic command if there is a command to be execed, else execs sh -i */
/* The child closes all inherited file descriptors and opens the new channel */
/* as stdin, stdout and stderr. It then pushes ldterm on the channel.        */

void
doexec(chan, command)
int chan;			/* channel in which the command is execed    */ 
char *command;			/* command to be executed		     */ 
{
 	register pid_t childpid;  /* pid of the child process		     */
	register int i;		/* loop counter				     */
	int  fd;		/* temporary file descriptor		     */
	char chanf[sizeof(cntlf)];/* name of the channel device		     */
	int layerspid;		/* pid of parent process		     */
	int it;			/* return code from I_FIND */


	trace("doexec: chan %d, command '%s'", chan, command);
	if( shells[chan] )  		/* make sure this guy is DEAD 	     */
		undoshell(chan);

	/* new channel device = /dev/xt/ijn where control channel for the    */
	/* terminal is /dev/xt/ij0 and the new channel no. is n		     */
	
	strcpy(chanf, cntlf);
	chanf[CHANIND] = '0' + chan;

	layerspid = getpid();

	/* These signals are used by the child process to signal whether the
	 * open() of the channel succeeds or fails. Note that it would be
	 * easier if we could just do the open() before the fork(), but the
	 * problem is that the open() must be after the setpgrp() and
	 * setpgrp() must be after the fork(). So, this becomes a situation
	 * of two process having to communicate with each other.
	 */
	signal(SIGUSR1, doexec_sigcatch);
	signal(SIGUSR2, doexec_sigcatch);

	whichsig = 0;
	dolongjmp = 0;

	switch( childpid = fork() ) {
	  case 0: /* child process */

		/* The process group id of the controlling shell is set      */
		/* to its existing pid value. Any signal sent to this process*/
		/* group id will kill the shell and all the processes that   */
		/* were forked by it, since the child process inherits the   */
		/* process group id of the parent process. 		     */	

		/* There is no point in returning from the child. All errors */
		/* result in exits.					     */

		(void)setpgrp();

		/* Close all the existing file descriptors : you don't want  */
		/* to share the parent's stdin, stdout and sterr since they  */
		/* are on a different window.		     		     */

		for (i = 0;  close(i++) != SYSERROR ;) ;

		/* It will be the duty of the execed process(sh) to change   */
		/* it to desired values if any.				     */

		signal(SIGCLD, SIG_DFL);

		/* set the stdin, stdout, stderr to the new channel          */

		/* open file descriptor: 0 = stdin
		 *
		 * If the open fails, signal the layers command with SIGUSR2
		 * so that it can print an error on layers exit. Otherwise,
		 * signal with SIGUSR1 so layers knows all is ok.
		 */
		for(i=0 ; open(chanf, O_RDWR) != 0 && i < 3 ; ++i) {
			if( errno == EAGAIN ) {
				continue;
			}
			else  {
				kill(layerspid, SIGUSR2);
				exit(ERREXIT); /* window will hang         */
			}
		}
		kill(layerspid, SIGUSR1);

		(void)dup(0);	/* file descriptor : 1	- stdout 	     */
		(void)dup(0);	/* file descriptor : 2  - stderr	     */

		/* Set these back to default. They were for use by the
		 * parent process.
		 */
		signal(SIGUSR1, SIG_DFL);
		signal(SIGUSR2, SIG_DFL);

		/* push a list of module specified by -h option or the 	     */
		/* default line discipline 				     */
		/* push the module only if they are not already on the stream*/

#ifdef i386
	    if (stream)
#endif

		if(ldlist)
			for( layersld = strtok(pmodlist,","); layersld != NULL;
			     layersld = strtok(NULL,",") ) {
				it = ioctl(1, I_FIND, layersld);
					if (it < 0) {
						error(DISP_ERR, errno,
						    "xtdoexec: module to PUSH %s not configured\n", layersld);
					}
				if((it == 0) && (ioctl(1, I_PUSH, layersld) == SYSERROR))
					error(DISP_ERR, errno,
					    "xtdoexec: PUSH of %s on new chan failed\n",
					    layersld);
			}
		else {
			it = ioctl(1, I_FIND, layersld);
			if (it < 0)
				error(DISP_ERR, errno,
				    "xtdoexec: module to PUSH %s not configured\n", layersld);
			if((it == 0) && (ioctl(1, I_PUSH, layersld) == SYSERROR))
				error(DISP_ERR, errno, "xtdoexec: PUSH of %s on new chan failed", 
				 layersld);
		}

		(void)ioctl(1, TCSETA, &ttysave);/* set the same values as of*/
						/* the original terminal     */
		if( firstshell == 0 ) {

	 	 /* Change the user terminal entry in utmp file to    */
	        /* indicate that s[he] is logged onto the window device.*/

			resetlogin(ttyfile, chanf);
		}

		/* if only shell is to be execed : i.e., no setup file was   */
		/* read, then exec shell -i, else exec shell -ic command     */

		if( strcmp(shell, command) == 0 ) {
			execarg[1] = iopt;
			execarg[2] = 0;
		} else {
			execarg[1] = icopt;
			execarg[2] = command;
		}
		(void)execvp(*execarg, execarg);

		/* Control should not return here unless exec failed         */
		error(DISP_ERR, errno, "exec of %s failed", *execarg);
		exit(ERREXIT);		/* terminal will hang		     */
		break;

	  case SYSERROR:		/* could not fork successfully	     */
		trace("doexec: fork failed - errno %d", errno);

		if( (fd = open(chanf, O_WRONLY)) != SYSERROR ) {
			if( write(fd, "Cannot fork, try again.", 23) == SYSERROR )
				trace("doexec: write of fork failed msg failed - errno %d", errno);
			sleep(2); /* allow output to drain */
			(void)close(fd);
		}
		else {
			trace("doexec: open to write fork failed msg failed - errno %d",
			    errno);
		}
		break;

	  default:			/* parent process		     */
		alarm(30);
		signal(SIGALRM, doexec_sigcatch);

		/* Nice little trick picked up from the sleep function. Use
		 * setjmp and longjmp to avoid infinite sleep if a signal
		 * occurs between the "whichsig == 0" test and the pause().
		 * Have the interupt handling routine return via a
		 * longjmp rather than return. The dolongjmp flag is in
		 * case the signal comes in before the setjmp() - if this
		 * happens whichsig will not equal 0 so pause will never
		 * be called.
		 */
		if( setjmp(doexecenv) == 0 ) {
			dolongjmp = 1;
			if( whichsig == 0 )
				pause();
		}

		alarm(0);
		/* reset original alarm handler */
		signal(SIGALRM, sigcatch);
			
		switch(whichsig) {
		case SIGUSR1:	/* signal from child that all is well */
			trace("doexec: child pid %d on channel %d", childpid, chan);
			shells[chan] = childpid; /* save the pid of parent shell */
			firstshell = 1;
			break;

		case SIGUSR2:	/* signal from child that open failed */
			/* Yea, I know it is ugly */
			error(STORE_ERR, 0,
			    "Open of %s failed, most likely because a process still \
had the channel open after it's window was deleted (look with ps). This caused a \
dead window.", chanf);
			break;

		default:
			/* Unexpected signal (probably a timeout). Just log
			 * it and ignore it. This is not serious because
			 * the worst that happens is an error message gets lost.
			 */
			trace("doexec: unexpected signal %d", whichsig);
			break;
		}

		break;
	}

	/* SIG_IGN these so that in case the timeout above expired
	 * and the signal comes in later, control() does not
	 * exit layers. In the obscure case of a process
	 * hanging on a channel, exit layers, go back into layers,
	 * kill the process and open a window, this can happen. The
	 * timeout was increased to the point where this does not
	 * normally happen, but it is still possible on a very
	 * slow system.
	 */
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
}

void
doexec_sigcatch(sig)
{
	whichsig = sig;
	if(dolongjmp)
		longjmp(doexecenv, 1);
}


/* kill the parent shell for the window indicated by chan 		     */

void
undoshell(chan)
register int chan;		/* channel whose controlling shell is killed */
{

	trace("undoshell: chan %d", chan);

	while ( shells[chan] ) {
		trace("undoshell: pid %d", shells[chan]);
		if( killcheck(chan, SIGHUP, 6) == FAIL ) {
			trace("undoshell: try calling killcheck again");
			killcheck(chan, SIGKILL, 2);	/* kill it for sure  */
		}
	}
}


/* Sends the supplied signal to the process group id of the parent shell for */
/* the specified channel, killing the parent shell and its children. It then */
/* waits for the shell to die. If another child shell dies during this wait, */
/* cleanup is done for that shell too.			                     */

/* RETURNS : SUCCEED/FAIL						     */ 

int
killcheck(chan,sig,timeout)
int chan;			/* channel on which kill signal is to be sent*/ 
int sig;			/* signal to be sent to the controlling shell*/
int timeout;			/* timeout for waiting for the shell to die  */
{
	register int  i;			/* loop counter		     */
	register pid_t pid;			/* pid of dying child	     */
	register int retval = FAIL;		/* function return value     */


	/* Setting SIGCLD to default so that waits can be individually 	     */
	/* performed. 							     */

	signal(SIGCLD, SIG_DFL);
	/*
	 *  Kill entire process group on channel
	 */
	if ( kill(-shells[chan],sig) == SYSERROR ) { /* process already gone */
		shells[chan] = 0;
		trace("killcheck: chan %d already gone", chan);
		retval = SUCCEED;
	} else {
		/* Wait for the children to die. This should take care of    */
		/* defunct processes.					     */

		(void)alarm(timeout);
		while ( (pid = wait((int *)0)) != SYSERROR ) {
			if ( pid == shells[chan] ) {
				(void)alarm(0);
				shells[chan] = 0;
				trace("killcheck: terminated on requested chan %d", chan);
				retval = SUCCEED;
				break;
			}
			for ( i = 0; i < MAXPCHAN; i++ )
				if ( pid == shells[i] ) {
					shells[i] = 0;
					trace("killcheck: shell terminated on chan %d", i);
				}
		}
	}
	(void)alarm(0);
	signal(SIGCLD, SIG_IGN); /* reset SIGCLD			     */
	return(retval);				
}



/* dummy signal catching routine					     */

void
sigcatch(sig)
int sig;					/* signal received 	     */
{
	(void)signal(sig, sigcatch);
	trace("sigcatch: sig = %d", sig);

	if (sig == SIGHUP)
	    fSIGHUP = TRUE;
}


/* terminate all the parent shells for the various channels		     */ 

void
termshells()
{
	register int i;				/* loop counter 	     */ 

	for( i = 0 ; i < MAXPCHAN ; i++ )
		while( shells[i] )
			undoshell(i);
} 


/* Execute the command "relogin - <entry to be changed>". The terminal line  */
/* field of the user's utmp entry will get changed to the name of the device */
/* now attached to the standard input. This is the same as the name of the   */
/* control channel for the windowing layer attached to the terminal.	     */ 

/* Note: SIGCLD must be set to SIG_DFL before calling resetlogin() or else   */
/* the wait() in this routine can hang.					     */

/* RETURNS : SUCCEED/FAIL                                          	     */

int
resetlogin(chgentry, newentry)
char *chgentry;			/* entry to be changed in /var/adm/utmp	     */
char *newentry;			/* what entry should be changed to	     */
{
	register int i;		/* loop counter				     */
	register int ret = SUCCEED;

	switch( fork() ) {
	  case 0:		/* child process			     */
		/* close all file descriptors above 2 			     */
		for (i = 3; close(i++) != SYSERROR ; ); 

#ifdef i386
		trace("ttyname = %s", ttyname(1));
#endif

		(void)execl(relogin, relogin, "-s", "-t", newentry, chgentry, 0);
		error(DISP_ERR, errno, "execl of %s failed", relogin);
		exit(ERREXIT);

	  case SYSERROR:	/* Failure in fork			     */	
		error(DISP_ERR, errno, "resetlogin child : fork failed.");
		ret = FAIL;
		break;

	  default :		/* parent process			     */
		/* layers should not exit leaving the child in the middle of */
		/* an edit because any following processes may fail in       */
		/* trying to update the utmp entry. 			     */

		if (wait((int *)0) == SYSERROR)
			ret = FAIL;
		break;
	}

	return(ret);
}

/* Used for time stamped trace.                      			     */

/* This subroutine may be called with variable no. of arguments. These       */
/* arguments are passed to fprintf.					     */
/* A max of two variables can be used (of regular size, i.e., max size = int)*/


void
trace(format, a, b) 
char *format;			/* pointer to format string		     */	
char *a;			/* a and b are two variables that are needed */ 
char *b;			/* by the format specified		     */
{
	long t;			/* used for logging time		     */ 

	/* if -D debugging option was not used or stderr is not
	 * redirected, forget tracing.
	 */
	if (!debug || use_errbuf == TRUE)
		return;

	(void)time(&t);
	(void)fprintf(stderr, "%.8s ", ctime(&t) + 11);
	if( format == (char *)NULL )
		return;
	(void)fprintf(stderr, format, a, b);
	(void)fprintf(stderr, "\n");
	fflush(stderr);
}

/* Prints error message in proper way for current conditions.
 *
 * The subroutine may be called with variable no. of arguments. The arguments
 * are just passed on to printf.
 * A max of two variables can be used (of regular size, i.e., max size = int)
 */
void
error(disp, myerrno, format, e1, e2)
int disp;			/* DISP_ERR, STORE_ERR or UNBOOT	     */
int myerrno;			/* error number if appropriate for this msg  */
char *format;			/* format string		     	     */
char *e1;			/* e1 and e2 are two variables that are      */
char *e2;			/* required by the format specified	     */	
{
	char *perrbuf = errbuf;

	time(&errtime);

	perrbuf += sprintf(perrbuf, "%s - error - ", layerscmd);
	perrbuf += sprintf(perrbuf, format, e1, e2);
	if( myerrno ) {
		char *ep;		/* string indicating errno expansion */

		if( myerrno < sys_nerr )
			ep = sys_errlist[myerrno];
		else
			ep = "Unknown error";
		perrbuf += sprintf(perrbuf," : errno %d : %s", myerrno, ep);
	}

	/* Unboot the terminal if necessary.
	 */
	if(disp == UNBOOT) {
		unboot_term();
		disp = DISP_ERR;
	}

	/* Display the error if possible.
	 */
	if(disp == DISP_ERR || use_errbuf == FALSE) {
		fprintf(stderr, "%s\r\n", errbuf);
		fflush(stderr);
		errbuf[0] = '\0';
	}

	/* Else the error will be displayed in main() on layers exit */
}


/* Once layers has called boot() the terminal is in xt mode waiting for
 * xt packets. If an error occurs and layers is not able to put the host
 * into xt mode, we must unboot the terminal before displaying an error
 * message. This routine sends the xt packets which tells the terminal
 * to initiate and then end the layers session. In other words, it is
 * simulating xt protocol enough to unboot the terminal. Pretty tricky,
 * huh?
 *
 * Note that this will only work for error messages between when boot()
 * returns and before multiplex does the I_LINK. After the I_LINK, we
 * can no longer do writes to stdout.
 */
unboot_term()
{
	/* JTIMOM xt packet which starts layers - unencoded and encoded */
	static char jtimom_pkt[] = {
		0x80, 0x05, 0x06, 0x06, 0x04, 0xB8, 0x0B, 0x7B, 0x13
	};
	static char jtimom_pkt_encoded[] = {
		0x20, 0x40, 0x45, 0x46, 0x42, 0x46, 0x44, 0x78,
		0x44, 0x4b, 0x7b, 0x53
	};

	/* JTERM on channel 0 xt packet which ends layers */
	static char jterm_pkt[] = {
		0x81, 0x02, 0x02, 0x00, 0x88, 0x9C
	};
	static char jterm_pkt_encoded[] = {
		0x20, 0x41, 0x42, 0x42, 0x4a, 0x40, 0x48, 0x5c
	};

	trace("Unbooting the terminal");

	clearerr(stdin);

	if( Loadtype == BINARY_LOAD )
		fwrite(jtimom_pkt, sizeof(jtimom_pkt), 1, stdout);
	   else
		fwrite(jtimom_pkt_encoded, sizeof(jtimom_pkt_encoded), 1, stdout);

	fflush(stdout);

	alarm(2);
	for(;;) { /* read and discard response from the terminal */
		getchar();
		if( ferror(stdin) ) {
			clearerr(stdin);
			break;
		}
	}

	if( Loadtype == BINARY_LOAD )
		fwrite(jterm_pkt, sizeof(jterm_pkt), 1, stdout);
	   else
		fwrite(jterm_pkt_encoded, sizeof(jterm_pkt_encoded), 1, stdout);

	fflush(stdout);

	alarm(5); /* 5 seconds because the 5620 takes a while to recover
		     from the JTERM */

	for(;;) { /* read and discard response from the terminal */
		getchar();
		if( ferror(stdin) ) {
			clearerr(stdin);
			break;
		}
	}
}
