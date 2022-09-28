/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)tset:tset.c	1.1.1.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) tset.c 1.1 88/03/29 tset:tset.c
 */
/*
**  TSET -- set terminal modes
**
**	This program does sophisticated terminal initialization.
**	I recommend that you include it in your .start_up or .login
**	file to initialize whatever terminal you are on.
**
**	There are several features:
**
**	A special file or sequence (as controlled by the ttycap file)
**	is sent to the terminal.
**
**	Mode bits are set on a per-terminal_type basis (much better
**	than UNIX itself).  This allows special delays, automatic
**	tabs, etc.
**
**	Erase and Kill characters can be set to whatever you want.
**	Default is to change erase to control-H on a terminal which
**	can overstrike, and leave it alone on anything else.  Kill
**	is always left alone unless specifically requested.  These
**	characters can be represented as "^X" meaning control-X;
**	X is any character.
**
**	Terminals which are dialups or plugboard types can be aliased
**	to whatever type you may have in your home or office.  Thus,
**	if you know that when you dial up you will always be on a
**	TI 733, you can specify that fact to tset.  You can represent
**	a type as "?type".  This will ask you what type you want it
**	to be -- if you reply with just a newline, it will default
**	to the type given.
**
**	The current terminal type can be queried.
**
**	Usage:
**		tset [-] [-EC] [-eC] [-kC] [-s] [-h] [-u] [-r]
**			[-m [ident] [test baudrate] :type]
**			[-Q] [-I] [-S] [type]
**
**		In systems with environments, use:
**			`tset -s ...`
**		Actually, this doesn't work because of a shell bug.
**		Instead, use:
**			tset -s ... > tset.tmp
**			source tset.tmp
**			rm tset.tmp
**		or:
**			set noglob
**			set term=(`tset -S ....`)
**			setenv TERM $term[1]
**			setenv TERMCAP "$term[2]"
**			unset term
**			unset noglob
**
**	Positional Parameters:
**		type -- the terminal type to force.  If this is
**			specified, initialization is for this
**			terminal type.
**
**	Flags:
**		- -- report terminal type.  Whatever type is
**			decided on is reported.  If no other flags
**			are stated, the only affect is to write
**			the terminal type on the standard output.
**		-r -- report to user in addition to other flags.
**		-EC -- set the erase character to C on all terminals
**			except those which cannot backspace (e.g.,
**			a TTY 33).  C defaults to control-H.
**		-eC -- set the erase character to C on all terminals.
**			C defaults to control-H.  If neither -E or -e
**			are specified, the erase character is set to
**			control-H if the terminal can both backspace
**			and not overstrike (e.g., a CRT).  If the erase
**			character is NULL (zero byte), it will be reset
**			to '#' if nothing else is specified.
**		-kC -- set the kill character to C on all terminals.
**			Default for C is control-X.  If not specified,
**			the kill character is untouched; however, if
**			not specified and the kill character is NULL
**			(zero byte), the kill character is set to '@'.
**		-iC -- reserved for setable interrupt character.
**		-qC -- reserved for setable quit character.
**		-m -- map the system identified type to some user
**			specified type. The mapping can be baud rate
**			dependent. This replaces the old -d, -p flags.
**			(-d type  ->  -m dialup:type)
**			(-p type  ->  -m plug:type)
**			Syntax:	-m identifier [test baudrate] :type
**			where: ``identifier'' is whatever is found in
**			/etc/ttytype for this port, (abscence of an identifier
**			matches any identifier); ``test'' may be any combination
**			of  >  =  <  !  @; ``baudrate'' is as with stty(1);
**			``type'' is the actual terminal type to use if the
**			mapping condition is met. Multiple maps are scanned
**			in order and the first match prevails.
**		-h -- don't read htmp file.  Normally the terminal type
**			is determined by reading the htmp file or the
**			environment (unless some mapping is specified).
**			This forces a read of the ttytype file -- useful
**			when htmp is somehow wrong.
**		-s -- output setenv commands for TERM.  This can be
**			used with
**				`tset -s ...`
**			and is to be prefered to:
**				setenv TERM `tset - ...`
**		-S -- Similar to -s but outputs a string suitable for
**			use in csh .login files as follows:
**				set noglob
**				set term=(`tset -S .....`)
**				setenv TERM $term[1]
**				unset term
**				unset noglob
**		-Q -- be quiet.  don't output 'Erase set to' etc.
**		-I -- don't do terminal initialization (is & if
**			strings).
**
**	Files:
**		/etc/ttytype
**			contains a terminal id -> terminal type
**			mapping; used when any user mapping is specified,
**			or the environment doesn't have TERM set.
**
**	Return Codes:
**		-1 -- couldn't open ttycap.
**		1 -- bad terminal type, or standard output not tty.
**		0 -- ok.
**
**	Defined Constants:
**		DIALUP -- the type code for a dialup port
**		PLUGBOARD -- the code for a plugboard port.
**		ARPANET -- the code for an arpanet port.
**		BACKSPACE -- control-H, the default for -e.
**		CONTROLU -- control-U, the default for -k.	M000
**		OLDERASE -- the system default erase character.
**		OLDKILL -- the system default kill character.
**		FILEDES -- the file descriptor to do the operation
**			on, nominally 1 or 2.
**		UIDMASK -- the bit pattern to mask with the getuid()
**			call to get just the user id.
**		GTTYN -- defines file containing generalized ttynames
**			and compiles code to look there.
**
**	Requires:
**		Routines to handle ttytype, and ttycap.
**
**	Compilation Flags:
**		OLDDIALUP -- accept the -d flag. Map "sd" to "dialup".
**		OLDPLUGBOARD -- accept the -p flag. Map "sp" to "plugboard".
**		OLDARPANET -- accept the -a flag. Map "sa" to "arpanet".
**		OLDFLAGS -- must be defined to compile code for any of
**			the -d, -p, or -a flags.
**		GTTYN -- if set, compiles code to look at /etc/ttytype.
**
**	Trace Flags:
**		none
**
**	Diagnostics:
**		Bad flag
**			An incorrect option was specified.
**		Too few args
**			more command line arguments are required.
**		Unexpected arg
**			wrong type of argument was encountered.
**		Cannot open ...
**			The specified file could not be openned.
**		Type ... unknown
**			An unknown terminal type was specified.
**		Cannot update htmp
**			Cannot update htmp file when the standard
**			output is not a terminal.
**		Erase set to ...
**			Telling that the erase character has been
**			set to the specified character.
**		Kill set to ...
**			Ditto for kill
**		Erase is ...    Kill is ...
**			Tells that the erase/kill characters were
**			wierd before, but they are being left as-is.
**		Not a terminal
**			Set if FILEDES is not a terminal.
**
**		where 'bin' should be whoever owns the 'htmp' file.
**		If 'htmp' is 666, then tset need not be setuid.
**
**	Author:
**		Eric Allman
**		Electronics Research Labs
**		U.C. Berkeley
**
**	History:
**		7/80 -- '-S' added. -m mapping added. TERMCAP string
**			cleaned up.
**		3/80 -- Changed to use tputs.  Prc & flush added.
**		10/79 -- '-s' option extended to handle TERMCAP
**			variable, set noglob, quote the entry,
**			and know about the Bourne shell.  Terminal
**			initialization moved to before any information
**			output so screen clears would not screw you.
**			'-Q' option added.
**		8/79 -- '-' option alone changed to only output
**			type.  '-s' option added.  'VERSION7'
**			changed to 'V6' for compatibility.
**		12/78 -- modified for eventual migration to VAX/UNIX,
**			so the '-' option is changed to output only
**			the terminal type to STDOUT instead of
**			FILEDES.  FULLLOGIN flag added.
**		9/78 -- '-' and '-p' options added (now fully
**			compatible with ttytype!), and spaces are
**			permitted between the -d and the type.
**		8/78 -- The sense of -h and -u were reversed, and the
**			-f flag is dropped -- same effect is available
**			by just stating the terminal type.
**		10/77 -- Written.
**
**	MODIFICATION HISTORY
**	M000	03 Aug 83	andyp	3.0 upgrade
**	- Changed system erase and kill characters.
**	- Changed default kill char to control-u (was control-x).
**
**	M001	20 Apr 84	ats
**	- Deleted -u flag if we're not UNIX V6
**	M002	20 Feb 88	davidby
**	- near complete rewrite.  use terminfo, not termcap 
**	- spawn tput to set delays, tabs, initialization of terminal
**	- use stdio instead of home-built buffering.
**	- remove V6 ifdef code
*/

/*
# define	FULLLOGIN	1
*/
# define	GTTYN		"/etc/ttytype"

#include	<ctype.h>
#include	<memory.h>
#include	<string.h>
#include	"delays.i"
#ifdef M_SYS3
#include	<sys/types.h>
#include	<curses.h>
#include	<unistd.h>
#include	<stdlib.h>
#define		stty(fd, termio)	ioctl(fd, TCSETAF, termio)
#define		gtty(fd, termio)	ioctl(fd, TCGETA, termio)
#else
#include	<sgtty.h>
#include	<stdio.h>
#endif


# define	BACKSPACE	('H' & 037)
/*# define	CONTROLX	('X' & 037)
/* */
# define	CONTROLU	('U' & 037)	/* M000 */
/* M000
/*# define	OLDERASE	'#'
/*# define	OLDKILL		'@'
/* */
# define	OLDERASE	BACKSPACE
# define	OLDKILL		CONTROLU

# define	FILEDES		2

# define	UIDMASK		-1

# define	DEFTYPE		"unknown"
#define		TTYPELEN	50

#define		USAGE\
"usage: tset [ - ] [ -hrsIQS ] [ -e[C] ] [ -E[C] ] [ -k[C] ]\n\t[-m [ident][test speed]:type] [type]\n"

# define	DIALUP		"dialup"
# define	OLDDIALUP	"sd"
# define	PLUGBOARD	"plugboard"
# define	OLDPLUGBOARD	"sp"
/***
# define	ARPANET		"arpanet"
# define	OLDARPANET	"sa"
***/
# define	OLDFLAGS



# ifdef GTTYN
typedef char	*ttyid_t;
# define	NOTTY		0
# else
typedef char	ttyid_t;
# define	NOTTY		'x'
# endif

/*
 * Baud Rate Conditionals
 */
# define	ANY		0
# define	GT		1
# define	EQ		2
# define	LT		4
# define	GE		(GT|EQ)
# define	LE		(LT|EQ)
# define	NE		(GT|LT)
# define	ALL		(GT|EQ|LT)



# define	NMAP		10

struct	map {
	char *Ident;
	char Test;
	char Speed;
	char *Type;
} map[NMAP];

struct map *Map = map;

struct
{
	char	*string;
	int	speed;
} speeds[] = {
	"0",	B0,
	"50",	B50,
	"75",	B75,
	"110",	B110,
	"134",	B134,
	"134.5",B134,
	"150",	B150,
	"200",	B200,
	"300",	B300,
	"600",	B600,
	"1200",	B1200,
	"1800",	B1800,
	"2400",	B2400,
	"4800",	B4800,
	"9600",	B9600,
	"exta",	EXTA,
	"extb",	EXTB,
	0,
};

char	Erase_char;		/* new erase character */
char	Kill_char;		/* new kill character */
char	Specialerase;		/* set => Erase_char only on terminals with backspace */

ttyid_t	Ttyid = NOTTY;		/* terminal identifier */
char	*TtyType;		/* type of terminal */
char	*DefType;		/* default type if none other computed */
char	*NewType;		/* mapping identifier based on old flags */
int	Dash_h;			/* don't read htmp */
int	DoSetenv;		/* output setenv commands */
int	BeQuiet;		/* be quiet */
int	NoInit;			/* don't output initialization string */
int	Report;			/* report current type */
int	Ureport;		/* report to user */
int	RepOnly;		/* report only */
int	CmndLine;		/* output full command lines (-s option) */
int	Ask;			/* ask user for termtype */

#ifdef M_SYS3
	struct termio	mode,oldmode;
#else
	struct sgttyb	mode;
	struct sgttyb	oldmode;
#endif


main(argc, argv)
int	argc;
char	*argv[];
{
	char		typebuf[TTYPELEN];
	register char	*p;
	char		*command;
	register int	i;
	int		Break;
	int		Not;
	int		Mapped = 0;
	extern char	*basename();
	extern char	*rem_delay();
	extern char	*nextarg();
	extern char	*mapped();
# ifdef GTTYN
	extern char	*stypeof();
	extern char	*tigetstr();
# endif
	char		bs_char;
	int		bs_len;
	int		bs_flag;
	char		*bs_str;
	int		csh = 0;
	extern short	ospeed;

	if (gtty(FILEDES, &mode) < 0)
	{
		fputs("Not a terminal\n", stderr);
		exit(1);
	}
	oldmode = mode;				/* structure copy */
	ospeed = mode.c_cflag & CBAUD;
		
	/* scan argument list and collect flags */
	command = argv[0];
	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == '\0')
	{
		RepOnly++;
	}
	argc--;
	while (--argc >= 0)
	{
		p = *++argv;
		if (*p == '-')
		{
			if (*++p == NULL)
				Report++; /* report current terminal type */
			else while (*p) switch (*p++)
			{

			  case 'r':	/* report to user */
				Ureport++;
				continue;

			  case 'E':	/* special erase: operate on all but TTY33 */
				Specialerase++;
				/* explicit fall-through to -e case */

			  case 'e':	/* erase character */
				if (*p == NULL)
					Erase_char = -1;
				else
				{
					if (*p == '^' && p[1] != NULL)
						Erase_char = *++p & 037;
					else
						Erase_char = *p;
					p++;
				}
				continue;

			  case 'k':	/* kill character */
				if (*p == NULL)
					Kill_char = CONTROLU;	/* M000 was X */
				else
				{
					if (*p == '^' && p[1] != NULL)
						Kill_char = *++p & 037;
					else
						Kill_char = *p;
					p++;
				}
				continue;

# ifdef OLDFLAGS
# ifdef	OLDDIALUP
			  case 'd':	/* dialup type */
				NewType = DIALUP;
				goto mapold;
# endif

# ifdef OLDPLUGBOARD
			  case 'p':	/* plugboard type */
				NewType = PLUGBOARD;
				goto mapold;
# endif

# ifdef OLDARPANET
			  case 'a':	/* arpanet type */
				Newtype = ARPANET;
				goto mapold;
# endif

mapold:				Map->Ident = NewType;
				Map->Test = ALL;
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				Map->Type = p;
				Map++;
				Mapped++;
				p = "";
				continue;
# endif

			  case 'm':	/* map identifier to type */
				/* This code is very loose. Almost no
				** syntax checking is done!! However,
				** illegal syntax will only produce
				** weird results.
				*/
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				if (isalnum(*p))
				{
					Map->Ident = p;	/* identifier */
					while (isalnum(*p)) p++;
				}
				else
					Map->Ident = "";
				Break = 0;
				Not = 0;
				while (!Break) switch (*p)
				{
					case NULL:
						p = nextarg(argc--, argv++);
						continue;

					case ':':	/* mapped type */
						*p++ = NULL;
						Break++;
						continue;

					case '>':	/* conditional */
						Map->Test |= GT;
						*p++ = NULL;
						continue;

					case '<':	/* conditional */
						Map->Test |= LT;
						*p++ = NULL;
						continue;

					case '=':	/* conditional */
					case '@':
						Map->Test |= EQ;
						*p++ = NULL;
						continue;
					
					case '!':	/* invert conditions */
						Not = ~Not;
						*p++ = NULL;
						continue;

					case 'B':	/* Baud rate */
						p++;
						/* intentional fallthru */
					default:
						if (isdigit(*p) || *p == 'e')
						{
							Map->Speed = baudratex(p);
							while (isalnum(*p) || *p == '.')
								p++;
						}
						else
							Break++;
						continue;
				}
				if (Not)	/* invert sense of test */
				{
					Map->Test = (~(Map->Test))&ALL;
				}
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				Map->Type = p;
				p = "";
				Map++;
				Mapped++;
				continue;

			  case 'h':	/* don't get type from env */
				Dash_h++;
				continue;

			  case 's':	/* output setenv commands */
				DoSetenv++;
				CmndLine++;
				continue;

			  case 'S':	/* output setenv strings */
				DoSetenv++;
				CmndLine=0;
				continue;

			  case 'Q':	/* be quiet */
				BeQuiet++;
				continue;

			  case 'I':	/* no initialization */
				NoInit++;
				continue;

			  case 'A':	/* Ask user */
				Ask++;
				continue;

			  default:
				*p-- = NULL;
				fatal("Bad flag -", p);
			}
		}
		else
		{
			/* terminal type */
			DefType = p;
		}
	}
	if (DefType)
	{
		if (Mapped)
		{
			Map->Ident = "";	/* means "map any type" */
			Map->Test = ALL;	/* at all baud rates */
			Map->Type = DefType;	/* to the default type */
		}
		else
			TtyType = DefType;
	}

	/* get current idea of terminal type from environment */
	if (!Dash_h && !Mapped && TtyType == 0)
		TtyType = getenv("TERM");

	/* determine terminal id if needed */
	if (!RepOnly && Ttyid == NOTTY && (TtyType == 0 || !Dash_h))
		Ttyid = ttyname(FILEDES);
# ifdef GTTYN
	/* If still undefined, look at /etc/ttytype */
	if (TtyType == 0)
		TtyType = stypeof(Ttyid);
# endif

	/* If still undefined, use DEFTYPE */
	if (TtyType == 0)
		TtyType = DEFTYPE;

	/* check for dialup or other mapping */
	if (Mapped)
		TtyType = mapped(TtyType, ospeed);

	/* TtyType now contains a pointer to the type of the terminal */
	/* If the first character is '?', ask the user */
	if (TtyType[0] == '?')
	{
		Ask++;
		TtyType++;
		if (TtyType[0] == '\0')
			TtyType = DEFTYPE;
	}
	if (Ask)
	{
		fprintf(stderr, "TERM = (%s) ", TtyType);
		fflush(stderr);

		/* read the terminal.  If not empty, set type */
		i = read(2, typebuf, TTYPELEN - 1);
		if (i >= 0)
		{
			if (typebuf[i - 1] == '\n')
				i--;
			typebuf[i] = '\0';
			if (typebuf[0] != '\0')
				TtyType = typebuf;
		}
	}

	if (!RepOnly)
	{
		pid_t chpid;
		int statloc = 0;

		switch (chpid = fork())
		{
			case 0:
			{
				char *arg1 = NoInit ? "reset" : "init";
				char envstr[TTYPELEN + sizeof("TERM=")];
				
				sprintf(envstr, "TERM=%s", TtyType);
				if ((putenv(envstr) == 0) &&
				    (dup2(fileno(stderr), fileno(stdout)) >= 0))
					{
					execlp("tput", "tput", arg1, NULL);
					/* no return from execlp */
					perror("tput");
					}
				else
					perror("tset");
				exit(-1);
			}
			default:
				if (chpid != wait(&statloc))
					{
					perror("tset");
					exit(-1);
					}
				else if (statloc != 0)
					exit(-1);
				else
					break;
			case -1:
				perror("tset");
				exit(-1);
		};
		statloc = 0;
		setupterm(TtyType, fileno(stderr), &statloc);
		switch (statloc)
		{
			case -1:
				fprintf(stderr,
					"Cannot open terminfo database\n");
				exit(-1);
			case 0:
				fprintf(stderr,"Type %s unknown", TtyType);
				exit(1);
			case 1:
				if (reset_shell_mode() != ERR)
					break;
				/* else fallthrough */
			default:
				fprintf(stderr, "Terminfo error\n");
				exit(-1);
		};
		
		if (((p = tigetstr("kbs")) != NULL) &&
		    ((bs_str = strdup(p)) != NULL))
			{
			p = rem_delay(bs_str, p);
			bs_len = strlen(p);
			bs_flag = (bs_len == 1) && ((*p) == BACKSPACE);
			}
		else
			bs_len = bs_flag = 0;
		/* determine erase and kill characters */
		if (Specialerase && !bs_flag)
			Erase_char = 0;
		if (bs_len == 1)
			bs_char = p[0];
		else
			bs_char = 0;
		if (Erase_char == 0 && !tigetflag("os") && mode.c_cc[VERASE] == OLDERASE)
		{
			if (bs_flag || bs_char != 0)
				Erase_char = -1;
		}
		if (Erase_char < 0)
			Erase_char = (bs_char != 0) ? bs_char : BACKSPACE;

		if (mode.c_cc[VERASE] == 0)
			mode.c_cc[VERASE] = OLDERASE;
		if (Erase_char != 0)
			mode.c_cc[VERASE] = Erase_char;
	
		if (mode.c_cc[VKILL] == 0)
			mode.c_cc[VKILL] = OLDKILL;
		if (Kill_char != 0)
			mode.c_cc[VKILL] = Kill_char;

		/* set modes */  
		mode.c_lflag |= ISIG | ICANON;
		mode.c_iflag |= ICRNL;
		mode.c_lflag |=  ECHO;
		if (memcmp((char *) &mode, (char *) &oldmode, sizeof mode))
			stty(FILEDES, &mode);

		/* set up environment for the shell we are using */
		/* (this code is rather heuristic) */
		if (DoSetenv)
		{
			char *sh;

			if ((sh = getenv("SHELL")) != NULL)
			{
				if ((csh = !strcmp(basename(sh), "csh")) &&
				    CmndLine)
					puts("set noglob;");
			}
			if (!csh)			/* running "sh" */
				puts("export TERM;");
		}
	}

	/* report type if appropriate */
	if (DoSetenv)
	{
		if (csh)
		{
			if (CmndLine)
				printf("setenv TERM ");
			printf("%s ", TtyType);
			if (CmndLine)
				puts(";\nunset noglob;");
		}
		else
			printf("TERM=%s;\n", TtyType);
	}
	else if (Report)
		printf("%s\n", TtyType);
	if (Ureport)
	{
		fprintf(stderr, "Terminal type is %s\n", TtyType);
		fflush(stderr);
	}
	if (RepOnly)
		exit(0);

	/* tell about changing erase and kill characters */
	reportek("Erase", mode.c_cc[VERASE], oldmode.c_cc[VERASE], OLDERASE);
	reportek("Kill", mode.c_cc[VKILL], oldmode.c_cc[VKILL], OLDKILL);
	exit(0);
}

reportek(name, new, old, def)
char	*name;
char	old;
char	new;
char	def;
{
	register char	o;
	register char	n;
	register char	*p;

	if (BeQuiet)
		return;
	o = old;
	n = new;

	if (o == n && n == def)
		return;
	fprintf(stderr, "%s %s ", name, (o == n) ? "is" : "set to");
	if (n < 040)
	{
		fputs("control-", stderr);
		n = (n & 037) | 0100;
	}
	fprintf(stderr, "%c\n", (char) n);
	fflush(stderr);
}

# ifdef GTTYN
char *
stypeof(ttyid)
char	*ttyid;
{
	extern void	getterm();
	extern char	*basename();
	static char	typebuf[TTYPELEN];
	register char	*PortType;

	if (ttyid == NOTTY)
		return (DEFTYPE);
	getterm(basename(ttyid), typebuf, GTTYN, DEFTYPE);
	PortType = typebuf;
# ifdef OLDDIALUP
	if (!strcmp(PortType, OLDDIALUP))
		PortType = DIALUP;
# endif

# ifdef OLDPLUGBOARD
	if (!strcmp(PortType, OLDPLUGBOARD))
		PortType = PLUGBOARD;
# endif

# ifdef OLDARPANET
	if (!strcmp(PortType, OLDARPANET))
		PortType = ARPANET;
# endif
	return(PortType);
}
# endif

#define	YES	1
#define	NO	0

baudratex(p)
char	*p;
{
	char buf[8];
	int i = 0;

	while (i < 7 && (isalnum(*p) || *p == '.'))
		buf[i++] = *p++;
	buf[i] = NULL;
	for (i=0; speeds[i].string; i++)
		if (!strcmp(speeds[i].string, buf))
			return (speeds[i].speed);
	return (-1);
}

char *
mapped(type, speed)
char	*type;
short	speed;
{
	int	match;

# ifdef DEB
	printf ("spd:%d\n", speed);
	prmap();
# endif
	Map = map;
	while (Map->Ident)
	{
		if (*(Map->Ident) == NULL || !strncmp(Map->Ident, type, 4))
		{
			match = NO;
			switch (Map->Test)
			{
				case ANY:	/* no test specified */
				case ALL:
					match = YES;
					break;
				
				case GT:
					match = (speed > Map->Speed);
					break;

				case GE:
					match = (speed >= Map->Speed);
					break;

				case EQ:
					match = (speed == Map->Speed);
					break;

				case LE:
					match = (speed <= Map->Speed);
					break;

				case LT:
					match = (speed < Map->Speed);
					break;

				case NE:
					match = (speed != Map->Speed);
					break;
			}
			if (match)
				return (Map->Type);
		}
		Map++;
	}
	/* no match found; return given type */
	return (type);
}

# ifdef DEB
prmap()
{
	Map = map;
	while (Map->Ident)
	{
	printf ("%s t:%d s:%d %s\n",
		Map->Ident, Map->Test, Map->Speed, Map->Type);
	Map++;
	}
}
# endif

char *
nextarg(argc, argv)
int	argc;
char	*argv[];
{
	if (argc <= 0)
		fatal ("Too few args: ", *argv);
	if (*(*++argv) == '-')
		fatal ("Unexpected arg: ", *argv);
	return (*argv);
}

fatal (mesg, obj)
char	*mesg;
char	*obj;
{
	fprintf(stderr, "%s%s\n%s", mesg, obj, USAGE);
	exit(1);
}

/* rem_delay -- remove delay strings from terminfo entries
 *	the array delay_exp[] contains the compiled
 *	regular expression which closely approximates the
 *	syntax of terminfo delay strings -- roughly
 *	$<..> where ".." is any number with at most one
 *	digit following the optional decimal point, followed by
 *	by either '/', '*', both, or neither.  The expression
 *	actually accepts a slightly wider range of strings,
 *	but not significantly so.
 */

char *
rem_delay(buf, str)
register char *str, *buf;
	{
	extern char *regex(), *__loc1, delay_exp[];
	char *end, *bufhead;
	int tmp;

	if ((str == NULL) || (buf == NULL))
		return NULL;
	bufhead = buf;
	while ((end = regex(delay_exp, str)) != NULL)
		{
		strncpy(buf, str, tmp = __loc1 - str);
		buf += tmp;
		str = end;
		}
	strcpy(buf, str);
	return bufhead;
	}
