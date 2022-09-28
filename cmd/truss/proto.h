/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:proto.h	1.6.3.1"

/*
 * Function prototypes for most external functions.
 */

#if	defined(__STDC__)

extern	int	requested( process_t * , int );
extern	int	jobcontrol( process_t * );
extern	int	signalled( process_t * );
extern	void	faulted( process_t * );
extern	int	sysentry( process_t * );
extern	void	sysexit( process_t * );
extern	void	showbuffer( process_t * , long , int );
extern	void	showbytes( CONST char * , int , char * );
extern	void	accumulate( timestruc_t * , timestruc_t * , timestruc_t * );

extern	CONST char *	ioctlname( int );
extern	CONST char *	fcntlname( int );
extern	CONST char *	sfsname( int );
extern	CONST char *	plockname( int );
extern	CONST char *	rfsysname( int );
#ifdef i386
extern	CONST char *	si86name( int );
#else
extern	CONST char *	s3bname( int );
#endif
extern	CONST char *	utscode( int );
extern	CONST char *	sigarg( int );
extern	CONST char *	openarg( int );
extern	CONST char *	msgflags( int );
extern	CONST char *	semflags( int );
extern	CONST char *	shmflags( int );
extern	CONST char *	msgcmd( int );
extern	CONST char *	semcmd( int );
extern	CONST char *	shmcmd( int );
extern	CONST char *	strrdopt( int );
extern	CONST char *	strevents( int );
extern	CONST char *	strflush( int );
extern	CONST char *	mountflags( int );
extern	CONST char *	svfsflags( int );
extern	CONST char *	sconfname( int );
extern	CONST char *	pathconfname( int );
extern	CONST char *	fuiname( int );
extern	CONST char *	fuflags( int );

extern	void	expound( process_t * , int , int );
extern	void	prtime( CONST char * , time_t );
extern	void	prtimestruc( CONST char * , timestruc_t );
extern	void	print_siginfo( siginfo_t *sip );

extern	void	increment( int * );
extern	void	decrement( int * );

extern	void	Flush(void);
extern	void	Eserialize(void);
extern	void	Xserialize(void);
extern	void	procadd( pid_t );
extern	void	procdel(void);
extern	int	checkproc( process_t * , char * , int );

extern	int	syslist( char * , sysset_t * , int * );
extern	int	siglist( char * , sigset_t * , int * );
extern	int	fltlist( char * , fltset_t * , int * );
extern	int	fdlist( char * , fileset_t * );

extern	char *	fetchstring( long , int );
extern	void	show_cred( process_t * , int );
extern	void	errmsg( CONST char * , CONST char * );
extern	void	abend( CONST char * , CONST char * );
extern	int	isprocdir( process_t * , CONST char * );

extern	void	outstring( CONST char * s );

extern	void	show_procset( process_t * , long );
extern	void	show_statloc( process_t * , long );
extern	CONST char *	woptions( int );

extern	CONST char *	errname( int );
extern	CONST char *	sysname( int , int );
extern	CONST char *	rawsigname( int );
extern	CONST char *	signame( int );
extern	CONST char *	rawfltname( int );
extern	CONST char *	fltname( int );

extern	void	show_xstat( process_t * , long );
extern	void	show_stat( process_t * , long );

#else	/* defined(__STDC__) */

extern	int	requested();
extern	int	jobcontrol();
extern	int	signalled();
extern	void	faulted();
extern	int	sysentry();
extern	void	sysexit();
extern	void	showbuffer();
extern	void	showbytes();
extern	void	accumulate();

extern	CONST char *	ioctlname();
extern	CONST char *	fcntlname();
extern	CONST char *	sfsname();
extern	CONST char *	plockname();
extern	CONST char *	rfsysname();
#ifdef i386
extern	CONST char *	si86name();
#else
extern	CONST char *	s3bname();
#endif
extern	CONST char *	utscode();
extern	CONST char *	sigarg();
extern	CONST char *	openarg();
extern	CONST char *	msgflags();
extern	CONST char *	semflags();
extern	CONST char *	shmflags();
extern	CONST char *	msgcmd();
extern	CONST char *	semcmd();
extern	CONST char *	shmcmd();
extern	CONST char *	strrdopt();
extern	CONST char *	strevents();
extern	CONST char *	strflush();
extern	CONST char *	mountflags();
extern	CONST char *	svfsflags();
extern	CONST char *	sconfname();
extern	CONST char *	pathconfname();
extern	CONST char *	fuiname();
extern	CONST char *	fuflags();

extern	void	expound();
extern	void	prtime();
extern	void	prtimestruc();
extern	void	print_siginfo();

extern	void	increment();
extern	void	decrement();

extern	void	Flush();
extern	void	Eserialize();
extern	void	Xserialize();
extern	void	procadd();
extern	void	procdel();
extern	int	checkproc();

extern	int	syslist();
extern	int	siglist();
extern	int	fltlist();
extern	int	fdlist();

extern	char *	fetchstring();
extern	void	show_cred();
extern	void	errmsg();
extern	void	abend();
extern	int	isprocdir();

extern	void	outstring();

extern	void	show_procset();
extern	void	show_statloc();
extern	CONST char *	woptions();

extern	CONST char *	errname();
extern	CONST char *	sysname();
extern	CONST char *	rawsigname();
extern	CONST char *	signame();
extern	CONST char *	rawfltname();
extern	CONST char *	fltname();

extern	void	show_xstat();
extern	void	show_stat();

#endif	/* defined(__STDC__) */
