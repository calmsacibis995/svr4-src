/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/shnodes.h	1.2.3.1"
/*
 *	UNIX shell
 *	Written by David Korn
 *
 */


#include	"io.h"
#include	"brkincr.h"

/* command tree for tretyp */
#define COMBITS		4
#define COMMSK		((1<<COMBITS)-1)
#define COMSCAN		(01<<COMBITS)
#define FPRS		(01<<COMBITS)
#define FINT		(02<<COMBITS)
#define FAMP		(04<<COMBITS)
#define FTMP		(010<<COMBITS)
#define FPIN		(020<<COMBITS)
#define FPOU		(040<<COMBITS)
#define FPCL		(0100<<COMBITS)
#define	FCOMSUB		(0200<<COMBITS)
#define FCOOP		(0400<<COMBITS)

#define TNEGATE		(01<<COMBITS)
#define TBINARY		(02<<COMBITS)
#define TUNARY		(04<<COMBITS)
#define TPAREN		(010<<COMBITS)
#define TSHIFT		(COMBITS+4)

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	(TWH|COMSCAN)
#define TTST	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11
#define TSELECT	12
#define	TTIME	13
#define TSETIO	14
#define TPROC	15


/* heap storage */
/* this node is a proforma for those that follow */

struct trenod
{
	int		tretyp;
	struct ionod	*treio;
};


struct dolnod
{
	struct dolnod	*dolnxt;
	int		doluse;
	char		*dolarg[1];
};

struct forknod
{
	int		forktyp;
	struct ionod	*forkio;
	union anynode	*forktre;
	int		forkline;
};

struct comnod
{
	int		comtyp;
	struct ionod	*comio;
	struct argnod	*comarg;
	struct argnod	*comset;
	int		comline;
};

struct ifnod
{
	int		iftyp;
	union anynode	*iftre;
	union anynode	*thtre;
	union anynode	*eltre;
};

struct whnod
{
	int		whtyp;
	union anynode	*whtre;
	union anynode	*dotre;
};

struct fornod
{
	int		fortyp;
	union anynode	*fortre;
	char	 *fornam;
	struct comnod	*forlst;
};

struct swnod
{
	int		swtyp;
	struct argnod	*swarg;
	struct regnod	*swlst;
};

struct regnod
{
	struct argnod	*regptr;
	union anynode	*regcom;
	struct regnod	*regnxt;
	char		regflag;
};

struct parnod
{
	int		partyp;
	union anynode	*partre;
};

struct lstnod
{
	int		lsttyp;
	union anynode	*lstlef;
	union anynode	*lstrit;
};


struct procnod
{
	int		proctyp;
	int		procline;
	union anynode	*proctre;
	char		*procnam;
	off_t		procloc;
	struct blk	*procblk;
};

struct ionod
{
	int		iofile;
	char		*ioname;
	char		*iolink;
	struct ionod	*ionxt;
	struct ionod	*iolst;
	char		*iodelim;
};

struct argnod
{
	union
	{
		struct argnod	*ap;
		char		*cp;
	}		argnxt;
	struct argnod	*argchn;
	char		argflag;
	char		argval[4];
};

/* mark for peek-ahead characters */
#define MARK	0100000


/* legal argument flags */
#define A_RAW	0x1		/* string needs no processing */
#define A_MAKE	0x2		/* bit set during argument expansion */
#define A_MAC	0x4		/* string needs macro expansion */
#define	A_EXP	0x8		/* string needs file expansion */
#define A_SPLIT	0x10		/* string needs word splitting */
#define A_ALIAS	0x20		/* formal alias argument */
#define A_JOIN	0x40		/* join with next argument */


/* types of ionodes */
#define IOUFD	0x1f
#define IOPUT	0x20
#define IOAPP	0x40
#define IOMOV	0x80
#define IODOC	0x100
#define IOSTRIP 0x200
#define IOCLOB	0x400
#define IORDW	0x800
#define IORAW	0x1000
#define IOSTRG	0x2000


union anynode
{
	struct argnod	arg;
	struct ionod	io;
	struct whnod	wh;
	struct swnod	sw;
	struct ifnod	if_;
	struct dolnod	dol;
	struct comnod	com;
	struct trenod	tre;
	struct forknod	fork;
	struct fornod	for_;
	struct regnod	reg;
	struct parnod	par;
	struct lstnod	lst;
	struct procnod	proc;
};

#ifdef PROTO
    extern union anynode	*sh_mkfork(int,union anynode*);
    extern union anynode	*sh_parse(int,int);
    extern int			sh_lex(void);
    extern void 		sh_eval(char*);
    extern void 		sh_prompt(int);
    extern void 		sh_syntax(void);
    extern int			sh_trace(char**,int);
    extern int			sh_exec(union anynode*,int);
#else
    extern union anynode	*sh_mkfork();
    extern union anynode	*sh_parse();
    extern int			sh_lex();
    extern void 		sh_eval();
    extern void 		sh_prompt();
    extern void 		sh_syntax();
    extern int			sh_trace();
    extern int			sh_exec();
#endif /* PROTO */

