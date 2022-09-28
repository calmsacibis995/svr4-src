/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cpp:common/cpp.c	1.45.1.1"


#ifdef FLEXNAMES
#	define NCPS	128
#else
#	define NCPS	8
#endif
int ncps = NCPS;		/* default name length */

# include "stdio.h"
# include "ctype.h"
/* C command - C preprocessor */

	/*
	* See if necessary defines are given
	*/
#if !defined( PD_MACH ) || !defined( PD_SYS )
#	include "*** Must define PD_MACH and PD_SYS ***"
#endif
	/*
	* These defines are used to enter new machines and/or systems
	* to the two lists.  To create a new instance of cpp in a cross
	* compilation environment, set PD_MY_MACH=\"your-new-machine\"
	* and/or PD_MY_SYS=\"your-new-os\" and set PD_MACH=MAX_PD_MACH
	* and PD_SYS=MAX_PD_SYS.
	*/
#ifndef PD_MY_MACH
#	define PD_MY_MACH	0
#endif
#ifndef PD_MY_SYS
#	define PD_MY_SYS	0
#endif

#define STATIC

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define CLASSCODE	27	/* exit status if #class seen */

#define READ	"r"
#define WRITE	"w"
#define SALT	'#'

#ifndef BUFSIZ
#	define BUFSIZ 512
#endif

char *pbeg, *pbuf, *pend;
char *outp, *inp;
char *newp;
char cinit;

#ifdef CXREF
char *xcopy();
FILE *outfp;
static int ready = 0;
int xline;
#endif

/* some code depends on whether characters are sign or zero extended */
/*	#if '\377' < 0		not used here, old cpp doesn't understand */
#if pdp11 | vax | i386
#	define COFF 128
#else
#	define COFF 0
#endif

# if gcos
#	define ALFSIZ 512	/* alphabet size */
# else
#	define ALFSIZ 256	/* alphabet size */
# endif
char macbit[ ALFSIZ + 11 ];
char toktyp[ ALFSIZ ];
#define BLANK 1
#define IDENT 2
#define NUMBR 3

/* a superimposed code is used to reduce the number of calls to the
/* symbol table lookup routine.  (if the kth character of an identifier
/* is 'a' and there are no macro names whose kth character is 'a'
/* then the identifier cannot be a macro name, hence there is no need
/* to look in the symbol table.)  'scw1' enables the test based on
/* single characters and their position in the identifier.  'scw2'
/* enables the test based on adjacent pairs of characters and their
/* position in the identifier.  scw1 typically costs 1 indexed fetch,
/* an AND, and a jump per character of identifier, until the identifier
/* is known as a non-macro name or until the end of the identifier.
/* scw1 is inexpensive.  scw2 typically costs 4 indexed fetches,
/* an add, an AND, and a jump per character of identifier, but it is also
/* slightly more effective at reducing symbol table searches.
/* scw2 usually costs too much because the symbol table search is
/* usually short; but if symbol table search should become expensive,
/* the code is here.
/* using both scw1 and scw2 is of dubious value.
*/
#define scw1 1
#define scw2 0

#if scw2
char t21[ ALFSIZ ], t22[ ALFSIZ ], t23[ ALFSIZ + NCPS ];
#endif

#if scw1
#	define b0 1
#	define b1 2
#	define b2 4
#	define b3 8
#	define b4 16
#	define b5 32
#	define b6 64
#	define b7 128
#endif

#define IB 1
#define SB 2
#define NB 4
#define CB 8
#define QB 16
#define WB 32
char fastab[ ALFSIZ ];
char slotab[ ALFSIZ ];
char *ptrtab;
#define isslo		( ptrtab == ( slotab + COFF ) )
#define isid(a)		( ( fastab + COFF )[a] & IB )
#define isspc(a)	( ptrtab[a] & SB )
#define isnum(a)	( ( fastab + COFF )[a] & NB )
#define iscom(a)	( ( fastab + COFF )[a] & CB )
#define isquo(a)	( ( fastab + COFF )[a] & QB )
#define iswarn(a)	( ( fastab + COFF )[a] & WB )

#define eob(a)		( ( a ) >= pend )
#define bob(a)		( pbeg >= ( a ) )

char buffer[ NCPS + BUFSIZ + BUFSIZ + NCPS ];

# define MAXNEST 12	/* max number of nested #include's */
# define MAXFRE 14	/* max buffers of macro pushback */

#ifdef pdp11
#	define SBSIZE 24000
#else
/* The idea here is to define the buffer to have a certain size
** (the literal number) plus enough room for slop and taking side buffers
** out to hold processing interrupted by #include's.  The cost of side
** buffers is particularly noticable on an Amdahl, where BUFSIZ is 4096.
*/
#	define SBSIZE (244000 + (MAXFRE+1)*BUFSIZ)

#endif

char	sbf[ SBSIZE ];
char	*savch	= sbf;

# define DROP '\376' /* special character not legal ASCII or EBCDIC */
# define WARN DROP
# define SAME 0
# define MAXINC 100	/* max number of directories for -I options */
# define MAXFRM 31	/* max number of formals/actuals to a macro */

static char warnc = WARN;

int mactop, fretop;
char *instack[ MAXFRE ], *bufstack[ MAXFRE ], *endbuf[ MAXFRE ];

int plvl;	/* parenthesis level during scan for macro actuals */
int maclin;	/* line number of macro call requiring actuals */
char *macfil;	/* file name of macro call requiring actuals */
char *macnam;	/* name of macro requiring actuals */
int maclvl;	/* # calls since last decrease in nesting level */
char *macforw;	/* pointer which must be exceeded to decrease nesting level */
int macdam;	/* offset to macforw due to buffer shifting */

#if tgp
	int tgpscan;	/* flag for dump(); */
#endif

STATIC	int	inctop[ MAXNEST ];
STATIC	char	*fnames[ MAXNEST ];
STATIC	char	*dirnams[ MAXNEST ];	/* actual directory of #include files */
STATIC	FILE	*fins[ MAXNEST ];
STATIC	int	lineno[ MAXNEST ];

STATIC	char	*dirs[ MAXINC ];	/* -I and <> directories */
STATIC	char	*dfltdir = (char *) 0;	/* user-supplied default (like /usr/include) */
char *copy(), *subst(), *trmdir(), *strchr();
char *chkend();
struct symtab *stsym();
STATIC	FILE	*fin	= stdin;
STATIC	FILE	*fout	= stdout;
STATIC	int	nd	= 1;
STATIC	int	pflag;	/* don't put out lines "# 12 foo.c" */
STATIC	int	passcom;	/* don't delete comments */
STATIC	int	incomm;	/* set while in comment so that EOF can be detected */
STATIC	int	rflag;	/* allow macro recursion */
STATIC	int	print_incs;	/* set to print out included filenames */
STATIC	int	ifno;
# define NPREDEF 20
STATIC	char *prespc[ NPREDEF ];
STATIC	char **predef = prespc;
STATIC	char *punspc[ NPREDEF ];
STATIC	char **prund = punspc;
STATIC	int	exfail;
struct symtab
{
	char	*name;
	char	*value;
} *lastsym, *lookup(), *slookup();

# if gcos
#	include <setjmp.h>
	static jmp_buf env;
#	define main	mainpp
#	undef exit
#	define exit(S)	longjmp(env, 1)
#	define symsiz 500
#	define LINEFORM	"# %d %s\n"
#	define ERRFORM	"*%c*   %s, line "
# else
#	define symsiz 4000
#	define LINEFORM	"# %d \"%s\"\n"
#	define ERRFORM	"*%c*   \"%s\", line "
# endif
STATIC	struct symtab stab[ symsiz ];

STATIC	struct symtab *defloc;
STATIC	struct symtab *udfloc;
STATIC	struct symtab *incloc;
STATIC	struct symtab *ifloc;
STATIC	struct symtab *elsloc;
STATIC	struct symtab *eifloc;
STATIC	struct symtab *elifloc;
STATIC	struct symtab *ifdloc;
STATIC	struct symtab *ifnloc;
STATIC	struct symtab *ysysloc;
STATIC	struct symtab *varloc;
STATIC	struct symtab *lneloc;
STATIC	struct symtab *ulnloc;
STATIC	struct symtab *uflloc;
STATIC	struct symtab *clsloc;
STATIC	struct symtab *idtloc;
STATIC	struct symtab *pragmaloc;
STATIC	int	trulvl;
STATIC	int	flslvl;
#define MAX_DEPTH	500		/* max of trulvl + flslvl */
#define SEEN_ELSE	0x1
#define TRUE_ELIF	0x2
STATIC	char	ifelstk[MAX_DEPTH];

	/*
	* To facilitate the handling of different cpp's and cross cpp's,
	* the sets of mutually exclusive predefined are placed into lists.
	* The value used from these lists are set when building from the
	* command line.
	*/
#define MAX_PD_MACH	14	/* set so it is where PD_MY_MACH is at */
#define D_tss		0	/*define table indexes */
#define D_os		1
#define D_mert		2
#define D_RT		3
#define D_RES		4
#define D_interdata	5
#define D_pdp11		6
#define D_vax		7
#define D_u370		8
#define D_u3b		9
#define D_u3b5		10
#define D_u3b2		11
#define D_u3b20d	12
#define D_i386		13
#define D_newmach	MAX_PD_MACH
#define D_nomach	MAX_PD_MACH

char *pdef_mach[MAX_PD_MACH+2] =	/* choose using PD_MACH */
{
	/*
	* The following are ``machines'' for historical reasons only.
	*/
	"tss",		"os",		"mert",
	"RT",		"RES",		
	/* These are the currently supported machines */
	"interdata",	"pdp11",	"vax",
	"u370",		"u3b",		"u3b5",
	"u3b2",		"u3b20d",	"i386",
	PD_MY_MACH,	/* for adding "own" machine */
	0
};

#define MAX_PD_SYS	4	/* set so that it is where PD_MY_SYS is at */
#define D_unix		0
#define D_gcos		1
#define D_ibm		2
#define D_DMERT		3
#define D_newsys	MAX_PD_SYS
#define D_nosys		MAX_PD_SYS

char *pdef_sys[MAX_PD_SYS+2] =	/* choose using PD_SYS */
{
	"unix",
	"gcos",
	"ibm",
	"DMERT",
	PD_MY_SYS,	/* for adding "own" system */
	0
};
#if PD_MACH < 0 || PD_MACH > MAX_PD_MACH
#	include "*** PD_MACH set illegally ***"
#endif
#if PD_SYS < 0 || PD_SYS > MAX_PD_SYS
#	include "*** PD_SYS set illegally ***"
#endif



sayline()
{
	if ( pflag == 0 )
		fprintf( fout, LINEFORM, lineno[ifno], fnames[ifno] );
}

/* data structure guide
/*
/* most of the scanning takes place in the buffer:
/*
/*  (low address)                                             (high address)
/*  pbeg                           pbuf                                 pend
/*  |      <-- BUFSIZ chars -->      |         <-- BUFSIZ chars -->        |
/*  _______________________________________________________________________
/* |_______________________________________________________________________|
/*          |               |               |
/*          |<-- waiting -->|               |<-- waiting -->
/*          |    to be      |<-- current -->|    to be
/*          |    written    |    token      |    scanned
/*          |               |               |
/*          outp            inp             p
/*
/*  *outp   first char not yet written to output file
/*  *inp    first char of current token
/*  *p      first char not yet scanned
/*
/* macro expansion: write from *outp to *inp (chars waiting to be written),
/* ignore from *inp to *p (chars of the macro call), place generated
/* characters in front of *p (in reverse order), update pointers,
/* resume scanning.
/*
/* symbol table pointers point to just beyond the end of macro definitions;
/* the first preceding character is the number of formal parameters.
/* the appearance of a formal in the body of a definition is marked by
/* 2 chars: the char WARN, and a char containing the parameter number.
/* the first char of a definition is preceded by a zero character.
/*
/* when macro expansion attempts to back up over the beginning of the
/* buffer, some characters preceding *pend are saved in a side buffer,
/* the address of the side buffer is put on 'instack', and the rest
/* of the main buffer is moved to the right.  the end of the saved buffer
/* is kept in 'endbuf' since there may be nulls in the saved buffer.
/*
/* similar action is taken when an 'include' statement is processed,
/* except that the main buffer must be completely emptied.  the array
/* element 'inctop[ifno]' records the last side buffer saved when
/* file 'ifno' was included.  these buffers remain dormant while
/* the file is being read, and are reactivated at end-of-file.
/*
/* instack[0 : mactop] holds the addresses of all pending side buffers.
/* instack[inctop[ifno]+1 : mactop-1] holds the addresses of the side
/* buffers which are "live"; the side buffers instack[0 : inctop[ifno]]
/* are dormant, waiting for end-of-file on the current file.
/*
/* space for side buffers is obtained from 'savch' and is never returned.
/* bufstack[0:fretop-1] holds addresses of side buffers which
/* are available for use.
*/

dump()
{
	/* write part of buffer which lies between  outp  and  inp .
	/* this should be a direct call to 'write', but the system slows
	/* to a crawl if it has to do an unaligned copy.  thus we buffer.
	/*? this silly loop is 15% of the total time, thus even the 'putc'
	/*? macro is too slow.
	*/
	register char *p1;
	register FILE *f;
#if tgp
	register char *p2;
#endif

	if ( ( p1 = outp ) == inp || flslvl != 0 )
		return;
#if tgp
#	define MAXOUT 80
	if ( !tgpscan )		/* scan again to insure <= MAXOUT
	{			/* chars between linefeeds */
		register char c, *pblank;
		char savc, stopc, brk;

		tgpscan = 1;
		brk = stopc = pblank = 0;
		p2 = inp;
		savc = *p2;
		*p2 = '\0';
		while ( c = *p1++ )
		{
			if ( c == '\\' )
				c = *p1++;
			if ( stopc == c )
				stopc = 0;
			else if ( c == '"' || c == '\'' )
				stopc = c;
			if ( p1 - outp > MAXOUT && pblank != 0 )
			{
				*pblank++ = '\n';
				inp = pblank;
				dump();
				brk = 1;
				pblank = 0;
			}
			if ( c == ' ' && stopc == 0 )
				pblank = p1 - 1;
		}
		if ( brk )
			sayline();
		*p2 = savc;
		inp = p2;
		p1 = outp;
		tgpscan = 0;
	}
#endif
	f = fout;
# if gcos
	/* filter out "$ program c" card if first line of input */
	/* gmatch is a simple pattern matcher in the GCOS Standard Library */
	{
		static int gmfirst = 0;

		if ( !gmfirst )
		{
			++gmfirst;
			if ( gmatch( p1, "^$*program[ \t]*c*") )
				p1 = strchr( p1, '\n' );
		}
	}
# endif
	if ( p1 < inp )
		p1 += fwrite( p1, sizeof( char ), inp - p1, f );
	outp = p1;
}

char *
refill( p )
	register char *p;
{
	/* dump buffer.  save chars from inp to p.  read into buffer at pbuf,
	/* contiguous with p.  update pointers, return new p.
	*/
	register char *np, *op;
	register int ninbuf;

	dump();
	np = pbuf - ( p - inp );
	op = inp;
	if ( bob( np + 1 ) )
	{
		pperror( "token too long" );
		np = pbeg;
		p = inp + BUFSIZ;
	}
	macdam += np - inp;
	outp = inp = np;
	while ( op < p )
		*np++ = *op++;
	p = np;
	for ( ;; )
	{
			/* retrieve hunk of pushed-back macro text */
		if ( mactop > inctop[ifno] )
		{
			op = instack[ --mactop ];
			np = pbuf;
			do
			{
				while ( *np++ = *op++ )
					;
			}
			while ( op < endbuf[mactop] );
			pend = np - 1;
			/* make buffer space avail for 'include' processing */
			if ( fretop < MAXFRE )
				bufstack[fretop++] = instack[mactop];
			return( p );
		}
		else 	/* get more text from file(s) */
		{
			maclvl = 0;
			if ( 0 < ( ninbuf = fread( pbuf, sizeof( char ), BUFSIZ, fin ) ) )
			{
				pend = pbuf + ninbuf;
				*pend = '\0';
				return( p );
			}
			/* end of #include file */
			if ( ifno == 0 )	/* end of input */
			{
				if ( plvl != 0 )
				{
					int n = plvl, tlin = lineno[ifno];
					char *tfil = fnames[ifno];

					lineno[ifno] = maclin;
					fnames[ifno] = macfil;
					pperror( "%s: unterminated macro call",
						macnam );
					lineno[ifno] = tlin;
					fnames[ifno] = tfil;
					np = p;
					/*shut off unterminated quoted string*/
					*np++ = '\n';
						/* supply missing parens */
					while ( --n >= 0 )
						*np++ = ')';
					pend = np;
					*np = '\0';
					if ( plvl < 0 )
						plvl = 0;
					return( p );
				}
				if ( incomm ) {
					pperror( "Unexpected EOF in comment" );
					/* undo flslvl bump when comment started */
					if (flslvl) --flslvl;
				}
				if ( trulvl +flslvl > 0 )
					pperror( "Unexpected EOF within #if, #ifdef or #ifndef" );
				inp = p;
				dump();
				if ( fout && ferror( fout ) )
					pperror( "Problems with writing output file; probably out of temp space" );
				exit( exfail ? ( exfail ==
					CLASSCODE ? CLASSCODE : 2 ) : 0 );
			}
			fclose( fin );
			fin = fins[--ifno];
			dirs[0] = dirnams[ifno];
			sayline();
#ifdef CXREF
 			fprintf(outfp, "\"%s\"\n", fnames[ifno]);
#endif
		}
	}
}

#define BEG 0
#define LF 1

char *
cotoken( p )
	register char *p;
{
	register int c, i;
	char quoc;
	static int state = BEG;
	static int speakup = 0;

	if ( state != BEG )
		goto prevlf;
	for ( ;; )
	{
	again:
		while ( !isspc( *p++ ) )
			;
		switch ( *( inp = p - 1 ) )
		{
		case 0:
			if ( eob( --p ) )
			{
				p = refill( p );
				goto again;
			}
			else
				++p;	/* ignore null byte */
			break;
		case '|':
		case '&':
			for ( ;; )	/* sloscan only */
			{
				if ( *p++ == *inp )
					break;
				if ( eob( --p ) )
					p = refill( p );
				else
					break;
			}
			break;
		case '=':
		case '!':
			for ( ;; )	/* sloscan only */
			{
				if ( *p++ == '=' )
					break;
				if ( eob( --p ) )
					p = refill( p );
				else
					break;
			}
			break;
		case '<':
		case '>':
			for ( ;; )	/* sloscan only */
			{
				if ( *p++ == '=' || p[-2] == p[-1] )
					break;
				if ( eob( --p ) )
					p = refill( p );
				else
					break;
			}
			break;
		case '\\':
			for ( ;; )
			{
				if ( *p++ == '\n' )
				{
					++lineno[ifno];
					break;
				}
				if ( eob( --p ) )
					p = refill( p );
				else
				{
					++p;
					break;
				}
			}
			break;
		case '/':
			for ( ;; )
			{
				if ( *p++ == '*' )	/* comment */
				{
					incomm = 1;
					if ( !passcom )
					{
						inp = p - 2;
						dump();
						++flslvl;
					}
					for ( ;; )
					{
						while ( !iscom( *p++ ) )
							;
						if ( p[-1] == '*' )
							for ( ;; )
							{
								if ( *p++ == '/' )
									goto endcom;
								if ( eob( --p ) )
								{
									if ( !passcom )
									{
										inp = p;
										p = refill( p );
									}
									/* split long comment */
									else if ( ( p - inp ) >= BUFSIZ )
									{
										*p++ = '*';
										*p++ = '/';
										inp = p;
										p = refill( p );
										outp = inp = p -= 2;
										*p++ = '/';
										*p++ = '*';
									}
									else
										p = refill( p );
								}
								else
									break;
							}
						else if ( p[-1] == '\n' )
						{
							++lineno[ifno];
							if ( !passcom && flslvl <= 1 )
								putc( '\n', fout );
						}
						else if ( eob( --p ) )
						{
							if ( !passcom )
							{
								inp = p;
								p = refill( p );
							}
							/* split long comment */
							else if ( ( p - inp ) >= BUFSIZ )
							{
								*p++ = '*';
								*p++ = '/';
								inp = p;
								p = refill( p );
								outp = inp = p -= 2;
								*p++ = '/';
								*p++ = '*';
							}
							else
								p = refill( p );
						}
						else
							++p;	/* ignore null byte */
					}
				endcom:
					incomm = 0;
					if ( !passcom )
					{
						outp = inp = p;
						--flslvl;
						goto again;
					}
					break;
				}
				if ( eob( --p ) )
					p = refill( p );
				else
					break;
			}
			break;
# if gcos
		case '`':
# endif
		case '"':
		case '\'':
			quoc = p[-1];
			for ( ;; )
			{
				while ( !isquo( *p++ ) )
					;
				if ( p[-1] == quoc )
					break;
				if ( p[-1] == '\n' )	/* bare \n terminates quotation */
				{
					--p;
					break;
				}
				if ( p[-1] == '\\' )
					for ( ;; )
					{
						if ( *p++ == '\n' )	/* escaped \n ignored */
						{
							++lineno[ifno];
							break;
						}
						if ( eob( --p ) )
							p = refill( p );
						else
						{
							++p;
							break;
						}
					}
				else if ( eob( --p ) )
					p = refill( p );
				else
					++p;	/* it was a different quote character */
			}
			break;
		case WARN:
			{
				int ii;

				dump();
				speakup = 0;
				for ( ii = sizeof(int) / sizeof(char); --ii >= 0; )
				{
					if ( eob( p ) )
						p = refill( p );
					speakup |= ( *p++ & 0xFF ) << ( ii * 8 );
				}
				inp = outp = p;
				break;
			}
		case '\n':
			++lineno[ifno];
			if ( isslo )
			{
				state=LF;
				return( p );
			}
		prevlf:
			if ( speakup )
			{
				inp = p;
				dump();
				lineno[ifno] = speakup + 1;
				sayline();
				speakup = 0;
			}
			state = BEG;
			for ( ;; )
			{
				/*
				* ignore formfeeds and vertical tabs
				* which may be just before the SALT
				*/
				if ( *p == '\f' || *p == '\v' )
				{
					register char *s = p;

					while ( *++s == '\f' || *s == '\v' )
						;
					if ( *s == SALT )
					{
						/*
						* get the SALT to the front!
						*/
						*s = *p;
						*p = SALT;
					}
				}
				if ( *p++ == SALT )
					return( p );
				if ( eob( inp = --p ) )
					p = refill( p );
				else
					goto again;
			}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for ( ;; )
			{
				while ( isnum( *p++ ) )
					;
				if ( eob( --p ) )
					p = refill( p );
				else
					break;
			}
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z': case '_':
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
#if scw1
#	define tmac1( c, bit )		if ( !xmac1( c, bit, & ) ) \
						goto nomac
#	define xmac1( c, bit, op )	( ( macbit + COFF )[c] op ( bit ) )
#else
#	define tmac1( c, bit )
#	define xmac1( c, bit, op )
#endif

#if scw2
#	define tmac2( c0, c1, cpos )	if ( !xmac2( c0, c1, cpos, & ) ) \
						goto nomac
#	define xmac2( c0, c1, cpos, op ) \
		( ( macbit + COFF )[ ( t21 + COFF )[c0] + \
		( t22 + COFF )[c1]] op ( t23 + COFF + cpos )[c0] )
#else
#	define tmac2( c0, c1, cpos )
#	define xmac2( c0, c1, cpos, op )
#endif

			if ( flslvl )
				goto nomac;
			for ( ;; )
			{
				c = p[-1];
				tmac1( c, b0 );
				i = *p++;
				if ( !isid( i ) )
					goto endid;
				tmac1( i, b1 );
				tmac2( c, i, 0 );
				c = *p++;
				if ( !isid( c ) )
					goto endid;
				tmac1( c, b2 );
				tmac2( i, c, 1 );
				i = *p++;
				if ( !isid( i ) )
					goto endid;
				tmac1( i, b3 );
				tmac2( c, i, 2 );
				c = *p++;
				if ( !isid( c ) )
					goto endid;
				tmac1( c, b4 );
				tmac2( i, c, 3 );
				i = *p++;
				if ( !isid( i ) )
					goto endid;
				tmac1( i, b5 );
				tmac2( c, i, 4 );
				c = *p++;
				if ( !isid( c ) )
					goto endid;
				tmac1( c, b6 );
				tmac2( i, c, 5 );
				i = *p++;
				if ( !isid( i ) )
					goto endid;
				tmac1( i, b7 );
				tmac2( c, i, 6 );
				tmac2( i, 0, 7 );
				while ( isid( *p++ ) )
					;
				if ( eob( --p ) )
				{
					refill( p );
					p = inp + 1;
					continue;
				}
				goto lokid;
			endid:
				if ( eob( --p ) )
				{
					refill( p );
					p = inp + 1;
					continue;
				}
				tmac2( p[-1], 0, -1 + ( p - inp ) );
			lokid:
				slookup( inp, p, 0 );
				if ( newp )
				{
					p = newp;
					goto again;
				}
				else
					break;
			nomac:
				while ( isid( *p++ ) )
					;
				if ( eob( --p ) )
				{
					p = refill( p );
					goto nomac;
				}
				else
					break;
			}
			break;
		} /* end of switch */
	
		if ( isslo )
			return( p );
	} /* end of infinite loop */
}

char *
skipbl( p )		/* get next non-blank token */
	register char *p;
{
	do
	{
		outp = inp = p;
		p = cotoken( p );
	}
	while ( ( toktyp + COFF )[*inp] == BLANK );
	return( p );
}

char *
unfill( p )
	register char *p;
{
	/* take <= BUFSIZ chars from right end of buffer and put
	/* them on instack.  slide rest of buffer to the right,
	/* update pointers, return new p.
	*/
	register char *np, *op;
	register int d;

	if ( mactop >= MAXFRE )
	{
		pperror( "%s: too much pushback", macnam );
		p = inp = pend;		 /* begin flushing pushback */
		dump();
		while ( mactop > inctop[ifno] )
		{
			p = refill( p );
			p = inp = pend;
			dump();
		}
	}
	if ( fretop > 0 )
		np = bufstack[--fretop];
	else
	{
		np = savch;
		savch += BUFSIZ;
		if ( savch >= sbf + SBSIZE )
		{
			pperror( "no space" );
			exit( exfail ? ( exfail == CLASSCODE ? CLASSCODE : 2 )
				: 0 );
		}
		*savch++ = '\0';
	}
	instack[mactop] = np;
	op = pend - BUFSIZ;
	if ( op < p )
		op = p;
	for ( ;; )		/* out with old */
	{
		while ( *np++ = *op++ )
			;
		if ( eob( op ) )
			break;
	}
	endbuf[mactop++] = np;	/* mark end of saved text */
	np = pbuf + BUFSIZ;
	op = pend - BUFSIZ;
	pend = np;
	if ( op < p )
		op = p;
	while ( outp < op )		/* slide over new */
		*--np = *--op;
	if ( bob( np ) )
		pperror( "token too long" );
	d = np - outp;
	outp += d;
	inp += d;
	macdam += d;
	return( p + d );
}

char *
doincl( p )
	register char *p;
{
	int filok, inctype;
	register char *cp;
	char **dirp, *nfil;
	char filname[BUFSIZ];

	p = skipbl( p );
	cp = filname;
	if ( *inp++ == '<' )	/* special <> syntax */
	{
		inctype = 1;
		++flslvl;	/* prevent macro expansion */
		for ( ;; )
		{
			outp = inp = p;
			p = cotoken( p );
			if ( *inp == '\n' )
			{
				--p;
				*cp = '\0';
				break;
			}
			if ( *inp == '>' )
			{
				*cp = '\0';
				break;
			}
# ifdef gimpel
			if ( *inp == '.' && !intss() )
				*inp = '#';
# endif
			while ( inp < p )
				*cp++ = *inp++;
		}
		--flslvl;	/* reenable macro expansion */
	}
	else if ( inp[-1] == '"' )	/* regular "" syntax */
	{
		inctype = 0;
# ifdef gimpel
		while ( inp < p )
		{
			if ( *inp == '.' && !intss() )
				*inp = '#';
				*cp++ = *inp++;
		}
# else
		while ( inp < p )
			*cp++ = *inp++;
# endif
		if ( *--cp == '"' )
			*cp = '\0';
	}
	else
	{
		int ppwarn(), pperror();
		/* This is a warning if we're skipping, error if not. */
		(*(flslvl ? ppwarn : pperror))( "bad include syntax", 0 );
		inctype = 2;
	}
	/* flush current file to \n , then write \n */

	p = chkend( p, 1 );

	outp = inp = p;

	dump();
	if ( inctype == 2 || flslvl != 0 )
		return( p );
	/* look for included file */
	if ( ifno + 1 >= MAXNEST )
	{
		pperror( "Unreasonable include nesting", 0 );
		return( p );
	}
	if ( ( nfil = savch ) > sbf + SBSIZE - BUFSIZ )
	{
		pperror( "no space" );
		exit( exfail ? ( exfail == CLASSCODE ? CLASSCODE : 2 ) : 0 );
	}

	/* check for #include "/usr/include/..." */
	if (strncmp(filname, "/usr/include/", 13) == 0)
	    ppwarn("#include of /usr/include/... may be non-portable");

	filok = 0;
	for ( dirp = dirs + inctype; *dirp; ++dirp )
	{
		if (
# if gcos
			strchr( filname, '/' )
# else
			filname[0] == '/' 
# endif
				|| **dirp == '\0' )
		{
			strcpy( nfil, filname );
		}
		else
		{
			strcpy( nfil, *dirp );
# if unix || gcos || DMERT
			strcat( nfil, "/" );
# endif
#ifdef ibm
#	ifndef gimpel
			strcat( nfil, "." );
#	endif
#endif
			strcat( nfil, filname );
		}
		if ( NULL != ( fins[ifno + 1] = fopen( nfil, READ ) ) )
		{
			filok = 1;
			fin = fins[++ifno];
			if ( print_incs )
				fprintf( stderr, "%s\n", nfil );
			break;
		}
	}
	if ( filok == 0 )
		pperror( "Can't find include file %s", filname );
	else
	{
		lineno[ifno] = 1;
		fnames[ifno] = cp = nfil;
		while ( *cp++)
			;
		savch = cp;
		dirnams[ifno] = dirs[0] = trmdir( copy( nfil ) );
		sayline();
#ifdef CXREF
 		fprintf(outfp, "\"%s\"\n", fnames[ifno]);
#endif
		/* save current contents of buffer */
		while ( !eob( p ) )
			p = unfill( p );
		inctop[ifno] = mactop;
	}
	return( p );
}

equfrm( a, p1, p2 )
	register char *a, *p1, *p2;
{
	register char c;
	int flag;

	c = *p2;
	*p2 = '\0';
	flag = strcmp( a, p1 );
	*p2 = c;
	return( flag == SAME );
}

char *
dodef( p )		/* process '#define' */
	char *p;
{
	register char *pin, *psav, *cf;
	char **pf, **qf;
	int b, c, params;
	int ex_blank;	/* used to ignore extra blanks in token-string */
	int sav_passcom = passcom;	/* saved passcom, used to reset it */
	struct symtab *np;
	char *oldval, *oldsavch;
	char *formal[MAXFRM]; /* formal[n] is name of nth formal */
	char formtxt[BUFSIZ]; /* space for formal names */

	if ( savch > sbf + SBSIZE - BUFSIZ )
	{
		pperror( "too much defining" );
		return( p );
	}
	oldsavch = savch; /* to reclaim space if redefinition */
	++flslvl;	/* prevent macro expansion during 'define' */
	p = skipbl( p );
	pin = inp;
	if ( ( toktyp + COFF )[*pin] != IDENT )
	{
		if (*pin == '\n') --lineno[ifno];
		ppwarn( "illegal/missing macro name" );
		if (*pin == '\n') ++lineno[ifno];
		while ( *inp != '\n' )
			p = skipbl( p );
		--flslvl;		/* restore expansion */
		return( p );
	}
	np = slookup( pin, p, 1 );
	if ( oldval = np->value )	/* was previously defined */
		savch = oldsavch;
#ifdef CXREF
 	def(np->name, lineno[ifno]);
#endif
	b = 1;
	cf = pin;
	while ( cf < p )		/* update macbit */
	{
		c = *cf++;
		xmac1( c, b, |= );
		b = ( b + b ) & 0xFF;
		if ( cf != p )
			xmac2( c, *cf, -1 + ( cf - pin ), |= );
		else
			xmac2( c, 0, -1 + ( cf - pin ), |= );
	}
	params = 0;
	outp = inp = p;
	p = cotoken( p );
	pin = inp;
	if ( *pin == '(' )	/* with parameters; identify the formals */
	{
#ifdef CXREF
 		newf(np->name, lineno[ifno]);
#endif
		cf = formtxt;
		pf = formal;
		for ( ;; )
		{
			p = skipbl( p );
			pin = inp;
			if ( *pin == '\n' )
			{
				--lineno[ifno];
				--p;
				pperror( "%s: missing )", np->name );
				break;
			}
			if ( *pin == ')' )
				break;
			if ( *pin == ',' )
				continue;
			if ( ( toktyp + COFF )[*pin] != IDENT )
			{
				c = *p;
				*p = '\0';
				pperror( "bad formal: %s", pin );
				*p = c;
			}
			else if ( pf >= &formal[MAXFRM] )
			{
				c = *p;
				*p = '\0';
				pperror( "too many formals: %s", pin );
				*p = c;
			}
			else
			{
				*pf++ = cf;
				while ( pin < p )
					*cf++ = *pin++;
				*cf++ = '\0';
				++params;
#ifdef CXREF
 				def(*(pf-1), lineno[ifno]);
#endif
			}
		}
		if ( params == 0 )	/* #define foo() ... */
			--params;
	}
	else if ( *pin == '\n' )
	{
		--lineno[ifno];
		--p;
	}
	else
	    p = inp;			/* back up to scan non-blank next token */
	/*
	* remember beginning of macro body, so that we can
	* warn if a redefinition is different from old value.
	*/
	oldsavch = psav = savch;
	passcom = 1;	/* make cotoken() return comments as tokens */
	ex_blank = 1;	/* must have some delimiter - might as well be blank */
	for ( ;; )	/* accumulate definition until linefeed */
	{
		outp = inp = p;
		p = cotoken( p );
		pin = inp;
		if ( *pin == '\\' && pin[1] == '\n' )	/* ignore escaped lf */
		{
			if ( !ex_blank )	/* replace it with a blank */
			{
				*psav++ = ' ';
				ex_blank = 1;
			}
			putc( '\n', fout );
			continue;
		}
		if ( *pin == '\n' )
			break;
		if ( ( toktyp + COFF )[*pin] == BLANK )	/* skip extra blanks */
		{
			if ( ex_blank )
				continue;
			*pin = ' ';	/* force it to be a "real" blank */
			ex_blank = 1;
		}
		else
			ex_blank = 0;
		if ( *pin == '/' && pin[1] == '*' )	/* skip comment */
		{					/* except for \n's */
			while ( pin < p )
				if ( *pin++ == '\n' )
					putc( '\n', fout );
			continue;
		}
		if ( params )	/* mark the appearance of formals in the definiton */
		{
			if ( ( toktyp + COFF )[*pin] == IDENT )
			{
				for ( qf = pf; --qf >= formal; )
				{
					if ( equfrm( *qf, pin, p ) )
					{
#ifdef CXREF
#ifndef NO_MACRO_FORMAL
 						ref(*qf, lineno[ifno]);
#endif
#endif
						*psav++ = qf - formal + 1;
						*psav++ = WARN;
						pin = p;
						break;
					}
				}
			}
			/* inside quotation marks, too */
			else if ( *pin == '"' || *pin == '\''
# if gcos
					|| *pin == '`'
# endif
						)
			{
				char quoc = *pin;

				for ( *psav++ = *pin++; pin < p && *pin != quoc; )
				{
					while ( pin < p && !isid( *pin ) )
					{
						if ( *pin == '\n'
							&& pin[-1] == '\\' )
						{
							putc( '\n', fout );
							psav--;	/* no \ */
							pin++;	/* no \n */
						}
						else
							*psav++ = *pin++;
					}
					cf = pin;
					while ( cf < p && isid( *cf ) )
						++cf;
					for ( qf = pf; --qf >= formal; )
					{
						if ( equfrm( *qf, pin, cf ) )
						{
							*psav++ = qf - formal + 1;
							*psav++ = WARN;
							pin = cf;
							break;
						}
					}
					while ( pin < cf )
						*psav++ = *pin++;
				}
			}
		}
		while ( pin < p )
			if ( *pin == '\n' && pin[-1] == '\\' )
			{
				putc( '\n', fout );
				psav--;	/* no \ */
				pin++;	/* no \n */
			}
			else
				*psav++ = *pin++;
	}
	passcom = sav_passcom;	/* restore to "real" value */
	if ( psav[-1] == ' ' )	/* if token-string ended with a blank */
		psav--;		/* then it is unnecessary - throw away */
	*psav++ = params;
	*psav++ = '\0';
	if ( ( cf = oldval ) != NULL )		/* redefinition */
	{
		--cf;	/* skip no. of params, which may be zero */
		while ( *--cf )		/* go back to the beginning */
			;
		if ( 0 != strcmp( ++cf, oldsavch ) )	/* redefinition different from old */
		{
			--lineno[ifno];
			ppwarn( "%s redefined", np->name );
			++lineno[ifno];
			np->value = psav - 1;
		}
		else
			psav = oldsavch; /* identical redef.; reclaim space */
	}
	else
		np->value = psav - 1;
	--flslvl;
	inp = pin;
	savch = psav;
	return( p );
}

#define fasscan()	ptrtab = fastab + COFF
#define sloscan()	ptrtab = slotab + COFF
/* this macro manages the lookup of a macro name and produces
** a warning if the name is missing
*/
#define LOOKUP(skipblank, flag) \
	++flslvl; if (skipblank) p = skipbl(p);	\
	if ( ( toktyp + COFF )[*inp] != IDENT )	\
	{					\
		if (*inp == '\n') --lineno[ifno];\
		ppwarn( "illegal/missing macro name" );	\
		if (*inp == '\n') ++lineno[ifno];\
		while ( *inp != '\n' )		\
			p = skipbl( p );	\
		--flslvl;			\
		continue;			\
	}					\
	np = slookup(inp,p,flag); --flslvl

char *
control( p )		/* find and handle preprocessor control lines */
	register char *p;
{
	register struct symtab *np;

	for ( ;; )
	{
		fasscan();
		p = cotoken( p );
		if ( *inp == '\n' )
			++inp;
		dump();
		sloscan();
		p = skipbl( p );
		*--inp = SALT;
		outp = inp;
		++flslvl;
		np = slookup( inp, p, 0 );
		--flslvl;
		if ( np == defloc )	/* define */
		{
			if ( flslvl == 0 )
			{
				p = dodef( p ); 
				continue;
			}
		}
		else if ( np == incloc )	/* include */
		{
			p = doincl( p );
			continue;
		}
		else if ( np == ifnloc )	/* ifndef */
		{
			LOOKUP(1/*skipbl*/, 0/*flag*/);	/* sets "np" */
			if ( flslvl == 0 && np->value == 0 )
				++trulvl;
			else
				++flslvl;
			if ( trulvl + flslvl >= MAX_DEPTH )
				pperror( "#if,ifdef,ifndef nesting too deep" );
			else
				ifelstk[trulvl + flslvl] =
					flslvl ? 0 : TRUE_ELIF;
#ifdef CXREF
			ref(xcopy(inp, p), lineno[ifno]);
#endif
			p = chkend( p, 1 );
		}
		else if ( np == ifdloc )	/* ifdef */
		{
			LOOKUP(1/*skipbl*/, 0/*flag*/);	/* sets "np" */
			if ( flslvl == 0 && np->value != 0 )
				++trulvl;
			else
				++flslvl;
			if ( trulvl + flslvl >= MAX_DEPTH )
				pperror( "#if,ifdef,ifndef nesting too deep" );
			else
				ifelstk[trulvl + flslvl] =
					flslvl ? 0 : TRUE_ELIF;
#ifdef CXREF
			ref(xcopy(inp, p), lineno[ifno]);
#endif
			p = chkend( p, 1 );
		}
		else if ( np == eifloc )	/* endif */
		{
			if ( flslvl )
			{
				if ( --flslvl == 0 )
					sayline();
			}
			else if ( trulvl )
				--trulvl;
			else
				pperror( "If-less endif", 0 );
			p = chkend( p, 1 );
		}
		else if ( np == elifloc )	/* elif */
		{
			if ( ifelstk[trulvl + flslvl] & SEEN_ELSE )
				pperror( "#elif following #else", 0 );
			if ( flslvl )
			{
				if ( --flslvl != 0 )
					++flslvl;
				else
				{
					newp = p;
#ifdef CXREF
					xline = lineno[ifno];
#endif
					if ( ifelstk[trulvl + flslvl + 1] == 0
						&& yyparse() )
					{
						++trulvl;
						ifelstk[trulvl + flslvl] =
							TRUE_ELIF;
						sayline();
					}
					else
						++flslvl;
					p = newp;
				}
			}
			else if ( trulvl )
			{
				++flslvl;
				--trulvl;
			}
			else
				pperror( "If-less elif", 0 );
		}
		else if ( np == elsloc )	/* else */
		{
			if ( ifelstk[trulvl + flslvl] & SEEN_ELSE )
				pperror( "too many #else's", 0 );
			if ( flslvl )
			{
				if ( --flslvl != 0 )
					++flslvl;
				else if ( ifelstk[trulvl + flslvl + 1] == 0 )
				{
					++trulvl;
					sayline();
				}
				else
					++flslvl;
			}
			else if ( trulvl )
			{
				++flslvl;
				--trulvl;
			}
			else
				pperror( "If-less else", 0 );
			ifelstk[trulvl + flslvl] |= SEEN_ELSE;
			p = chkend( p, 1 );
		}
		else if ( np == udfloc )	/* undefine */
		{
			if ( flslvl == 0 )
			{
				LOOKUP(1/*skipbl*/, DROP); /* sets "np" */
				p = chkend( p, 1 );
			}
			else
				p = chkend( p, 2 );
#ifdef CXREF
			ref(xcopy(inp, p), lineno[ifno]);
#endif
		}
		else if ( np == ifloc )		/* if */
		{
#if tgp
			pperror( " IF not implemented, true assumed", 0 );
			if ( flslvl == 0 )
				++trulvl;
			else
				++flslvl;
#else
			newp = p;
#ifdef CXREF
			xline = lineno[ifno];
#endif
			if ( flslvl == 0 && yyparse() )
				++trulvl;
			else
				++flslvl;
			p = newp;
			if ( trulvl + flslvl >= MAX_DEPTH )
				pperror( "#if,ifdef,ifndef nesting too deep" );
			else
				ifelstk[trulvl + flslvl] =
					flslvl ? 0 : TRUE_ELIF;
#endif
		}
		else if ( np == lneloc )	/* line */
		{
			if ( flslvl == 0 && pflag == 0 )
			{
				register char *s;
				register int ln;

				outp = inp = p;
			do_line:
				*--outp = '#';
				/*
				* make sure that the whole
				* directive has been read
				*/
				s = p;
				while ( *s && *s != '\n' )
					s++;
				if ( eob( s ) )
					p = refill( s );
				/*
				* eat the line number
				*/
				s = inp;
				while ( ( toktyp + COFF )[*s] == BLANK )
					s++;
				ln = 0;
				while ( isdigit( *s ) )
					ln = ln * 10 + *s++ - '0';
				if ( ln )
					lineno[ifno] = ln - 1;
				else
					pperror( "bad number for #line" );
				/*
				* eat the optional "filename"
				*/
				while ( ( toktyp + COFF )[*s] == BLANK )
					s++;
				if ( *s != '\n' )
				{
					if ( *s != '"' )
						pperror( "bad file for #line" );
					else
					{
						register char *t = savch;

						for ( ;; )
						{
							if ( *++s == '"' )
								break;
							else if ( *s == '\n' ||
								*s == '\0' )
							{
								pperror( "bad file for #line" );
								break;
							}
							*t++ = *s;
						}
						*t++ = '\0';
						if ( strcmp( savch, fnames[ifno] ) )
						{
							fnames[ifno] = savch;
							savch = t;
						}
					}
				}
				/*
				* push it all along to be eventually printed
				*/
				while ( *inp != '\n' )
					p = cotoken( p );
				continue;
			}
		}
		else if ( np == clsloc )	/* class */
			exfail = CLASSCODE;		/* return value */
		else if ( np == idtloc )	/* ident */
		{
			if ( pflag == 0 )
				while ( *inp != '\n' )	/* just pass it along */
					p = cotoken( p );
		}
#ifdef	PRAGMA
		else if ( np == pragmaloc ) {	/* pragma */
			while ( *inp != '\n' )	/* pass it all along */
				p = cotoken( p );
		}
#endif
		else if ( *++inp == '\n' )	/* allows blank line after # */
			outp = inp;
		else if ( isdigit( *inp ) )	/* pass thru line directives */
		{
			outp = p = inp;
			goto do_line;
		}
		else
			pperror( "undefined control", 0 );
		/* flush to lf */
		++flslvl;
		while ( *inp != '\n' )
		{
			outp = inp = p;
			p = cotoken( p );
		}
		--flslvl;
	}
}

struct symtab *
stsym( s )
	register char *s;
{
	char buf[BUFSIZ];
	register char *p;

	/* make definition look exactly like end of #define line */
	/* copy to avoid running off end of world when param list is at end */
	p = buf;
	while ( *p++ = *s++ )
		;
	p = buf;
	while ( isid( *p++ ) )		/* skip first identifier */
		;
	if ( *--p == '=' )
	{
		*p++ = ' '; 
		while ( *p++ )
			;
	}
	else
	{
		s = " 1";
		while ( *p++ = *s++ )
			;
	}
	pend = p;
	*--p = '\n';
	sloscan();
	dodef( buf );
	return( lastsym );
}

struct symtab *
ppsym( s )		/* kludge */
	char *s;
{
	register struct symtab *sp;

	cinit = SALT;
	*savch++ = SALT;
	sp = stsym( s );
	--sp->name;
	cinit = 0;
	return( sp );
}


int yy_errflag;		/* TRUE when pperror called by yyerror() */

/* VARARGS1 */
pperror( s, x, y )
	char *s;
{
	if ( fnames[ifno][0] )
# if gcos
		fprintf( stderr, ERRFORM, exfail >= 0 ? 'F' : 'W',fnames[ifno]);
# else
		fprintf( stderr, "%s: ", fnames[ifno] );
# endif
	fprintf( stderr, "%d: ", lineno[ifno] );
	fprintf( stderr, s, x, y );
	if ( yy_errflag )
		fprintf( stderr, " (in preprocessor if)\n" );
	else
		fprintf( stderr, "\n" );
	if ( exfail < CLASSCODE - 1 )
		++exfail;
}

yyerror( s, a, b )
	char *s;
{
	yy_errflag = 1;
	pperror( s, a, b );
	yy_errflag = 0;
}

ppwarn( s, x )
	char *s;
{
	int fail = exfail;

	exfail = -1;
	pperror( s, x );
	exfail = fail;
}

struct symtab *
lookup( namep, enterf )
	char *namep;
{
	register char *np, *snp;
	register int c, i;
	int around;
	register struct symtab *sp;

	/* namep had better not be too long (currently, <=ncps chars) */
	np = namep;
	around = 0;
	i = cinit;
	while ( c = *np++ )
		i += i + c;
	c = i;	/* c=i for register usage on pdp11 */
	c %= symsiz;
	if ( c < 0 )
		c += symsiz;
	sp = &stab[c];
	while ( snp = sp->name )
	{
		np = namep;
		while ( *snp++ == *np )
			if ( *np++ == '\0' )
			{
				if ( enterf == DROP )
				{
					sp->name[0] = DROP;
					sp->value = 0;
				}
				return( lastsym = sp );
			}
		if ( --sp < &stab[0] )
			if ( around )
			{
				pperror( "too many defines", 0 );
				exit( exfail ? ( exfail ==
					CLASSCODE ? CLASSCODE : 2 ) : 0 );
			}
			else
			{
				++around;
				sp = &stab[symsiz - 1];
			}
	}
	if ( enterf == 1 )
		sp->name = namep;
	return( lastsym = sp );
}

struct symtab *
slookup( p1, p2, enterf )
	register char *p1, *p2;
	int enterf;
{
	register char *p3;
	char c2, c3;
	struct symtab *np;

	c2 = *p2;	/* mark end of token */
	*p2 ='\0';
	if ( ( p2 - p1 ) > ncps )
		p3 = p1 + ncps;
	else
		p3 = p2;
	c3 = *p3;	/* truncate to ncps chars or less */
	*p3 ='\0';
	if ( enterf == 1 )
		p1 = copy( p1 );
	np = lookup( p1, enterf );
	*p3 = c3;
	*p2 = c2;
	if ( np->value != 0 && flslvl == 0 )
		newp = subst( p2, np );
	else
		newp = 0;
	return( np );
}

char *
subst( p, sp )
	register char *p;
	struct symtab *sp;
{
	static char match[] = "%s: argument mismatch";
	register char *ca, *vp;
	int params;
	char *actual[MAXFRM]; /* actual[n] is text of nth actual */
	char acttxt[BUFSIZ]; /* space for actuals */

	if ( 0 == ( vp = sp->value ) )
		return( p );
	if ( ( p - macforw ) <= macdam )
	{
		if ( ++maclvl > symsiz && !rflag )
		{
			pperror( "%s: macro recursion", sp->name );
			return( p );
		}
	}
	else
		maclvl = 0;	/* level decreased */
	macforw = p;	/* new target for decrease in level */
	macdam = 0;
	macnam = sp->name;
#ifdef CXREF
	ref(macnam, lineno[ifno]);
#endif
	dump();
	if ( sp == ulnloc )
	{
		vp = acttxt;
		*vp++ = '\0';
		sprintf( vp, "%d", lineno[ifno] );
		while ( *vp++ )
			;
	}
	else if ( sp == uflloc )
	{
		vp = acttxt;
		*vp++ = '\0';
		sprintf( vp, "\"%s\"", fnames[ifno] );
		while ( *vp++ )
			;
	}
	if ( 0 != ( params = *--vp & 0xFF ) )	/*definition calls for params */
	{
		register char **pa;
		int dparams;		/* parameters in definition */

		ca = acttxt;
		pa = actual;
		if ( params == 0xFF )	/* #define foo() ... */
			params = 0;
		dparams = params;
		sloscan();
		++flslvl; /* no expansion during search for actuals */
		plvl = -1;
		maclin = lineno[ifno];
		macfil = fnames[ifno];
		do
		{
			p = skipbl( p );
		}
		while ( *inp == '\n' );		/* skip \n too */
		if ( *inp == '(' )
		{
			for ( plvl = 1; plvl != 0; )
			{
				*ca++ = '\0';
				for ( ;; )
				{
					outp = inp = p;
					p = cotoken( p );
					if ( *inp == '(' )
						++plvl;
					if ( *inp == ')' && --plvl == 0 )
					{
						if ( ca > acttxt+1 )
							--params;
						break;
					}
					if ( plvl == 1 && *inp == ',' )
					{
						--params;
						break;
					}
					while ( inp < p )
					{
						/*
						*  toss newlines in arguments
						*  to macros - keep problems
						*  to a minimum.
						*  if a backslash is just
						*  before the newline, assume
						*  it is in a string and
						*  leave it in.
						*/
						if ( *inp == '\n'
							&& inp[-1] != '\\' )
						{
							*inp = ' ';
						}
						*ca++ = *inp++;
					}
					if ( ca > &acttxt[BUFSIZ] )
						pperror( "%s: actuals too long", sp->name );
				}
				if ( pa >= &actual[MAXFRM] )
					ppwarn( match, sp->name );
				else
					*pa++ = ca;
			}
		}
		else {				/* should have seen (, because def
						** had args
						*/
			ppwarn( match, sp->name );
			p = inp;		/* back up to current token */
		}
		if ( maclin != lineno[ifno] )	/* embedded linefeeds in macro call */
		{
			int i, j = lineno[ifno];

			for ( i = sizeof( int ) / sizeof( char ); --i >= 0; )
			{
				if ( bob( p ) )
				{
					outp = inp = p;
					p = unfill( p );
				}
				*--p = j;
				j >>= 8;
			}
			if ( bob( p ) )
			{
				outp = inp = p;
				p = unfill( p );
			}
			*--p = warnc;
		}
		/* def with one arg and use with zero args is special ok case */
		if ( params < 0 || (params > 0 && dparams != 1) )
			ppwarn( match, sp->name );
		while ( --params >= 0 )
			*pa++ = "" + 1;	/* null string for missing actuals */
		--flslvl;
		fasscan();
	}
	for ( ;; )	/* push definition onto front of input stack */
	{
		while ( !iswarn( *--vp ) )
		{
			if ( bob( p ) )
			{
				outp = inp = p;
				p = unfill( p );
			}
			*--p = *vp;
		}
		if ( *vp == warnc )	/* insert actual param */
		{
			ca = actual[*--vp - 1];
			while ( *--ca )
			{
				if ( bob( p ) )
				{
					outp = inp = p;
					p = unfill( p );
				}
				*--p = *ca;
			}
		}
		else
			break;
	}
	outp = inp = p;
	return( p );
}




char *
trmdir( s )
	register char *s;
{
	register char *p = s;

	while ( *p++ )
		;
	--p;
	while ( p > s && *--p != '/' )
		;
# if unix || DMERT
	if ( p == s )
		*p++ = '.';
# endif
	*p = '\0';
	return( s );
}

STATIC char *
copy( s )
	register char *s;
{
	register char *old;

	old = savch;
	while ( *savch++ = *s++ )
		;
	return( old );
}

yywrap()
{
	return( 1 );
}

main( argc, argv )
	char *argv[];
{
	register int i, c;
	register char *p;
	char *tf, **cp2;
	int forclass = 0;		/* 1 if C-with-classes selected */

# if gcos
	if ( setjmp( env ) )
		return( exfail ? ( exfail == CLASSCODE ? CLASSCODE : 2 ) : 0 );
# endif
	p = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	i = 0;
	while ( c = *p++ )
	{
		( fastab + COFF )[c] |= IB | NB | SB;
		( toktyp + COFF )[c] = IDENT;
#if scw2
		/* 53 == 63-10; digits rarely appear in identifiers,
		/* and can never be the first char of an identifier.
		/* 11 == 53*53/sizeof(macbit) .
		*/
		++i;
		( t21 + COFF )[c] = ( 53 * i ) / 11;
		( t22 + COFF )[c] = i % 11;
#endif
	}
	p = "0123456789.";
	while ( c = *p++ )
	{
		( fastab + COFF )[c] |= NB | SB;
                ( toktyp + COFF )[c] = NUMBR;
	}
# if gcos
	p = "\n\"'`/\\";
# else
	p = "\n\"'/\\";
# endif
	while ( c = *p++ )
		( fastab + COFF )[c] |= SB;
# if gcos
	p = "\n\"'`\\";
# else
	p = "\n\"'\\";
# endif
	while ( c = *p++ )
		( fastab + COFF )[c] |= QB;
	p = "*\n";
	while ( c = *p++ )
		( fastab + COFF)[c] |= CB;
	( fastab + COFF )[warnc] |= WB | SB;
	( fastab + COFF )['\0'] |= CB | QB | SB | WB;
	for ( i = ALFSIZ; --i >= 0; )
		slotab[i] = fastab[i] | SB;
	p = " \t\v\f\r";	/* note no \n; */
	while ( c = *p++ )
		( toktyp + COFF )[c] = BLANK;
#if scw2
	for ( ( t23 + COFF )[i = ALFSIZ + 7 - COFF] = 1; --i >= -COFF; )
		if ( ( ( t23 + COFF )[i] = ( t23 + COFF + 1 )[i] << 1 ) == 0 )
			( t23 + COFF )[i] = 1;
#endif

# if unix || DMERT
	fnames[ifno = 0] = "";
	dirnams[0] = dirs[0] = ".";
# endif
# if ibm
	fnames[ifno = 0] = "";
# endif
# if gcos
	if ( inquire( stdin, _TTY ) )
		freopen( "*src", "rt", stdin );
# endif
# if gimpel || gcos
	fnames[ifno = 0] = (char *) inquire( stdin, _FILENAME );
	dirnams[0] = dirs[0] = trmdir( copy( fnames[0] ) );
# endif
	for ( i = 1; i < argc; i++ )
	{
		switch ( argv[i][0] )
		{
		case '-':
# if gcos
					/* case-independent on GCOS */
			switch ( toupper( argv[i][1] ) )
# else
			switch( argv[i][1] )
# endif
			{
			case 'P':
				pflag++;
			case 'E':
				continue;
			case 'R':
				++rflag;
				continue;
			case 'C':
				passcom++;
				continue;
#ifdef CXREF
			case 'F':
				if ((outfp = fopen(argv[++i],"w")) == NULL)
				{
					fprintf(stderr, "Can't open %s\n",
					    argv[i]);
					exit(1);
				}
				continue;
#endif
			case 'D':
				if ( predef > prespc + NPREDEF )
				{
					pperror( "too many -D options, ignoring %s", argv[i] );
					continue;
				}
				/* ignore plain "-D" (no argument) */
				if ( *( argv[i] + 2 ) )
					*predef++ = argv[i] + 2;
				continue;
			case 'U':
				if ( prund > punspc + NPREDEF )
				{
					pperror( "too many -U options, ignoring %s", argv[i] );
					continue;
				}
				*prund++ = argv[i] + 2;
				continue;
			case 'I':
				if ( nd > MAXINC - 4 )
					pperror( "excessive -I file (%s) ignored", argv[i] );
				else
					dirs[nd++] = argv[i] + 2;
				continue;
			case 'Y':
				dfltdir = argv[i] + 2;
				continue;
			case '\0':
				continue;
			case 'T':
				ncps = 8;	/* backward name compatabilty */
				continue;
			case 'H':		/* print included filenames */
				print_incs++;
				continue;
			case 'W':		/* enable C with classes */
				forclass = 1;
				continue;
			default: 
				pperror( "unknown flag %s", argv[i] );
				continue;
			}
		default:
			if ( fin == stdin )
			{
				if ( NULL == ( fin = fopen( argv[i], READ ) ) )
				{
					pperror( "No source file %s", argv[i] );
					exit(2);
				}
				fnames[ifno] = copy( argv[i] );
#ifdef CXREF
				fprintf(outfp,"\"%s\"\n", fnames[ifno]);
#endif
				dirs[0] = dirnams[ifno] = trmdir( argv[i] );
# ifndef gcos
/* too dangerous to have file name in same syntactic position
   be input or output file depending on file redirections,
   so force output to stdout, willy-nilly
	[i don't see what the problem is.  jfr]
*/
			}
			else if ( fout == stdout )
			{
				static char _sobuf[BUFSIZ];

				if ( NULL == ( fout = fopen( argv[i], WRITE ) ) )
				{
					pperror( "Can't create %s", argv[i] );
					exit(2);
				}
				else
				{
					fclose( stdout );
					setbuf( fout, _sobuf );
				}
# endif
			}
			else
				pperror( "extraneous name %s", argv[i] );
		}
	}

	fins[ifno] = fin;
	exfail = 0;
	/* after user -I files here are the standard include libraries */
	/* use user-supplied default first */
	if (dfltdir != (char *) 0)
		dirs[nd++] = dfltdir;
	else {
# if unix || DMERT
#    if defined(SGSINC) || defined(DMERT)
		if ( strlen( SGSINC ) != 0 )
			dirs[nd++] = SGSINC;	/* For cross cpp's */
#    endif
#    ifdef USRINC
		if ( strlen( USRINC ) != 0 )
			dirs[nd++] = USRINC;	/* usually /usr/include */
#    endif
#    if !defined( SGSINC ) && !defined( USRINC )
		dirs[nd++] = "/usr/include";
#    endif
# endif
# if gcos
	dirs[nd++] = "cc/include";
# endif
# if ibm
#	ifndef gimpel
		dirs[nd++] = "BTL$CLIB";
#	endif
# endif
# ifdef gimpel
	dirs[nd++] = intss() ?  "SYS3.C." : "";
# endif
# ifdef compool
	dirs[nd++] = "/compool";
# endif
	}
	dirs[nd++] = 0;
	defloc = ppsym( "define" );
	udfloc = ppsym( "undef" );
	incloc = ppsym( "include" );
	elsloc = ppsym( "else" );
	eifloc = ppsym( "endif" );
	elifloc = ppsym( "elif" );
	ifdloc = ppsym( "ifdef" );
	ifnloc = ppsym( "ifndef" );
	ifloc = ppsym( "if" );
	lneloc = ppsym( "line" );
	if (forclass)			/* only when C with classes enabled */
	    clsloc = ppsym( "class" );
	idtloc = ppsym( "ident" );
#ifdef	PRAGMA
	pragmaloc = ppsym( "pragma" );
#endif
	for ( i = sizeof( macbit ) / sizeof( macbit[0] ); --i >= 0; )
		macbit[i] = 0;

	if ( pdef_mach[PD_MACH] )
		ysysloc = stsym( pdef_mach[PD_MACH] );
	if ( pdef_sys[PD_SYS] )
		varloc = stsym( pdef_sys[PD_SYS] );

	tf = fnames[ifno];		/* do the -D and -U options now */
	fnames[ifno] = "command line";
	lineno[ifno] = 1;
	cp2 = prespc;
	while ( cp2 < predef )
		stsym( *cp2++ );
	cp2 = punspc;
	while ( cp2 < prund )
	{
		if ( p = strchr( *cp2, '=' ) )
			*p++ = '\0';
		/*
		* Truncate to ncps characters if needed.
		*/
		if ( strlen( *cp2 ) > ncps )
			(*cp2)[ncps] = '\0';
		lookup( *cp2++, DROP );
	}

	ulnloc = stsym( "__LINE__" );
	uflloc = stsym( "__FILE__" );
	fnames[ifno] = tf;
	pbeg = buffer + ncps;
	pbuf = pbeg + BUFSIZ;
	pend = pbuf + BUFSIZ;

	trulvl = 0;
	flslvl = 0;
	lineno[0] = 1;
	sayline();
	outp = inp = pend;
#ifdef CXREF
	ready = 1;
#endif
	control( pend );
	if ( fout && ferror( fout ) )
		pperror( "Problems with output file; probably out of temp space" );
	exit( exfail ? ( exfail == CLASSCODE ? CLASSCODE : 2 ) : 0 );
}
 
#ifdef CXREF
ref( name, line )
	char *name;
	int line;
{
#ifdef FLEXNAMES
	fprintf(outfp, "R%s\t%05d\n", name, line);
#else
	fprintf(outfp, "R%.8s\t%05d\n", name, line);
#endif
}

def( name, line )
	char *name;
	int line;
{
	if (ready)
#ifdef FLEXNAMES
		fprintf(outfp, "D%s\t%05d\n", name, line);
#else
		fprintf(outfp, "D%.8s\t%05d\n", name, line);
#endif
}

newf( name, line )
	char *name;
	int line;
{
#ifdef FLEXNAMES
	fprintf(outfp, "F%s\t%05d\n", name, line);
#else
	fprintf(outfp, "F%.8s\t%05d\n", name, line);
#endif
}

char *
xcopy( ptr1, ptr2 )
	register char *ptr1, *ptr2;
{
	static char name[NCPS];
	char *saveptr, ch;
	register char *ptr3 = name;

	/* locate end of name; save character there */
	if ((ptr2 - ptr1) > ncps)
	{
		saveptr = ptr1 + ncps;
		ch = *saveptr;
		*saveptr = '\0';
	}
	else {
		ch = *ptr2;
		*ptr2 = '\0';
		saveptr = ptr2;
	}
	while (*ptr3++ = *ptr1++) ;	/* copy name */
	*saveptr = ch;	/* replace character */
	return( name );
}
#endif

/* Scan over the end of a directive, making sure there's no
** residual junk.
**
** This code is a little messier than you might expect.  To avoid bumping
** the line number when we hit the newline, print the message the first time
** we find we've got too many tokens.
*/

static char *
chkend( p, ntoken )
char * p;
int ntoken;				/* number of tokens at which to warn */
{
    int extratokens = 0;
    int save_passcom = passcom;

    passcom = 0;			/* ignore comments in this context */
    ++flslvl;				/* don't expand anything */

    /* if at newline, force rescan to do end-of-line stuff */
    if (*inp == '\n')
	p = inp;

    for (;;) {
	p = skipbl(p);		/* get next token */
	if (*inp == '\n')
	    break;			/* reached end */
	if(++extratokens == ntoken)	/* hit number of extras; flag here */
	    ppwarn("extra tokens (ignored) after directive");
    }

    /* restore comment flag */
    passcom = save_passcom;
    --flslvl;				/* back to previous expansion level */

    return( p );
}
