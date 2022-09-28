/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)saf:extern.h	1.5.4.1"

extern	void	log();
extern	void	initialize();
extern	void	openlog();
extern	void	opendebug();
extern	void	debug();
extern	void	insert();
extern	void	startpms();
extern	void	startit();
extern	void	pollpms();
extern	void	pollfail();
extern	void	startpoll();
extern	void	sigpoll();
extern	void	sendpmmsg();
extern	void	purge();
extern	void	parse();
extern	void	read_table();
extern	void	readpipe();
extern	void	error();
extern	void	replace();
extern	void	reap();
extern	void	sendack();
extern	void	account();
extern	void	cleanut();
extern	void	readutmp();
extern	void	quit();
extern	void	usage();

extern	FILE	*open_temp();

extern	struct	sactab	*read_entry();
extern	struct	sactab	*findpm();

extern	char	*trim();
extern	char	*pstate();
extern	char	*nexttok();
extern	char	**mkargv();
extern	char	*make_tempname();
extern	char	**dump_table();

extern	long	time();
extern	long	atol();
extern	char	*ctime();
extern	char	*strchr();
extern	char	*strrchr();
extern	char	*strcat();
extern	char	*strpbrk();
extern	char	*strtok();
extern	char	*strcpy();
extern	char	*fgets();
extern	char	*malloc();
extern	char	*calloc();
extern	char	*mktemp();
extern	void	free();
extern	void	setutent();
extern	void	endutent();
extern	struct	utmp	*getutid();
extern	struct	utmp	*getutent();

extern	int	N_msgs;
extern	unsigned	Stime;
extern	int	Sfd;
extern	int	Cfd;
extern	int	Nentries;
extern	struct	sactab	*Sactab;
extern	struct	errmsg	Msgs[];
extern	char	Scratch[];

extern	char	Comment[];
extern	int	Saferrno;

extern	char	**environ;
extern	int	errno;
extern	char	*optarg;
extern	int	optind;
extern	int	opterr;
