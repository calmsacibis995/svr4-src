/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_LDTERM_H
#define _SYS_LDTERM_H

#ident	"@(#)head.sys:sys/ldterm.h	11.5.6.2"

#include <sys/emap.h>
#define	IBSIZE	16		/* "standard" input data block size */
#define	OBSIZE	64		/* "standard" output data block size */
#define	EBSIZE	16		/* "standard" echo data block size */

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define V_MIN 		(char) tp->t_modes.c_cc[VMIN]
#define V_TIME		(char) tp->t_modes.c_cc[VTIME]
#define RAW_MODE	!(tp->t_modes.c_lflag & ICANON)
#define CANON_MODE	(tp->t_modes.c_lflag & ICANON)

/*
 * The following for EUC.
 */

#define EUCSIZE	sizeof(struct eucioc)
#define EUCIN	0	/* copying eucioc_t IN from ioctl */
#define EUCOUT	1	/* copying it OUT to user format */

/*
 * One assumption made throughout this module is:  EUC characters have
 * a display width less than 255.  Also, assumed around, is that they
 * consist of < 256 bytes, but we don't worry much about that.
 */

#define EUC_TWIDTH	255	/* Width of a TAB, as returned by "ldterm_dispwidth" */
#define EUC_BSWIDTH	254	/* Width of a backspace as returned */
#define EUC_NLWIDTH	253	/* newline & cr */
#define EUC_CRWIDTH	252
#define EUC_MAXW	4	/* max display width and memory width, both */
#define EUC_WARNCNT	20	/* # bad EUC erase attempts before hollering */

typedef struct ldterm_mod {
	mblk_t	*t_savbp;	/* saved mblk that holds ld struct */
	struct termios t_modes;	/* Effective modes set by the provider below */
	struct termios t_amodes;/* Apparent modes for user programs */
	struct termios t_dmodes;/* Modes that driver wishes to process */
	unsigned long t_state;	/* internal state of ldterm module */
	int	t_line;		/* output line of tty */
	int	t_col;		/* output column of tty */
	int	t_rocount;	/* number of chars echoed since last output */
	int	t_rocol;	/* column in which first such char appeared */
	mblk_t	*t_message;	/* pointer to first mblk in message being built */
	mblk_t	*t_endmsg;	/* pointer to last mblk in that message */
	int	t_msglen;	/* number of characters in that message */
	mblk_t	*t_echomp;	/* echoed output being assembled */
	int	t_rd_request;   /* Number of bytes requested by M_READ during
				 * vmin/vtime read
				 */
	int	t_tid;		/* vtime timer id */
	/*
	 * The following are for EUC processing. 
	 */
	unchar	t_codeset;	/* current code set indicator (read side) */
	unchar	t_eucleft;	/* bytes left to get in current char (read) */
	unchar	t_eucign;	/* bytes left to ignore (output post proc) */
	unchar	t_eucpad;	/* padding ... for eucwioc */
	eucioc_t eucwioc;	/* eucioc structure (have to use bcopy) */
	unchar	*t_eucp;	/* ptr to parallel array of column widths */
	mblk_t	*t_eucp_mp;	/* the m_blk that holds parallel array */
	unchar	t_maxeuc;	/* the max length in memory bytes of an EUC */
	int	t_eucwarn;	/* bad EUC counter */
	struct emp_tty t_emap;	/* XENIX character mapping info */
} ldtermstd_state_t;

/*
 * Internal state bits.
 */
#define	TS_XCLUDE	0x00000001	/* exclusive-use flag against open */
#define	TS_TTSTOP	0x00000002	/* output stopped by ^S */
#define	TS_TBLOCK	0x00000004	/* input stopped by IXOFF mode */
#define	TS_QUOT		0x00000008	/* last character input was \ */
#define	TS_ERASE	0x00000010	/* within a \.../ for PRTRUB */
#define	TS_SLNCH	0x00000020	/* next character service routine sees is literal */
#define	TS_PLNCH	0x00000040	/* next character put routine sees is literal */

#define TS_TTCR		0x00000080	/* mapping NL to CR-NL */
#define	TS_NOCANON	0x00000100	/* canonicalization done by somebody below us */
#define	TS_RESCAN	0x00000400	/* canonicalization mode changed, rescan input queue */
#define	TS_RTO		0x00000800	/* timer started for vmin/vtime */
#define	TS_TACT		0x00001000	/* timer active for vmin/vtime */
#define TS_MEUC		0x00010000	/* TRUE if multi-byte codesets used */
#define TS_WARNED	0x00020000	/* already warned on console */
#define TS_CLOSE	0x00040000	/* close in progress */



/*
 * Character types.
 */
#define	ORDINARY	0
#define	CONTROL		1
#define	BACKSPACE	2
#define	NEWLINE		3
#define	TAB		4
#define	VTAB		5
#define	RETURN		6

/*
 * The following for EUC handling:
 */
#define T_SS2		7
#define T_SS3		8

/*
 * Table indicating character classes to tty driver.  In particular,
 * if the class is ORDINARY, then the character needs no special
 * processing on output.
 *
 * Characters in the C1 set are all considered CONTROL; this will
 * work with terminals that properly use the ANSI/ISO extensions,
 * but might cause distress with terminals that put graphics in
 * the range 0200-0237.  On the other hand, characters in that
 * range cause even greater distress to other UNIX terminal drivers....
 */

static char typetab[256] = {
/* 000 */ 	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 004 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 010 */	BACKSPACE,	TAB,		NEWLINE,	CONTROL,
/* 014 */	VTAB,		RETURN,		CONTROL,	CONTROL,
/* 020 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 024 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 030 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 034 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 040 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 044 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 050 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 054 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 060 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 064 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 070 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 074 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 100 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 104 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 110 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 114 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 120 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 124 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 130 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 134 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 140 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 144 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 150 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 154 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 160 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 164 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 170 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 174 */	ORDINARY,	ORDINARY,	ORDINARY,	CONTROL,
/* 200 */ 	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 204 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 210 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 214 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 220 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 224 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 230 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 234 */	CONTROL,	CONTROL,	CONTROL,	CONTROL,
/* 240 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 244 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 250 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 254 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 260 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 264 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 270 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 274 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 300 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 304 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 310 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 314 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 320 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 324 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 330 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 334 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 340 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 344 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 350 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 354 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 360 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 364 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/* 370 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
/*
 * WARNING:  For EUC, 0xFF must be an ordinary character.  It is used with
 * single-byte EUC in some of the "ISO Latin Alphabet" codesets, and occupies
 * a screen position; in those ISO sets where that position isn't used, it
 * shouldn't make any difference.
 */
/* 374 */	ORDINARY,	ORDINARY,	ORDINARY,	ORDINARY,
};

/*
 * Translation table for output without OLCUC.  All ORDINARY-class characters
 * translate to themselves.  All other characters have a zero in the table,
 * which stops the copying.
 */
static unsigned char notrantab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
/* 050 */	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
/* 060 */	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
/* 070 */	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
/* 100 */	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 110 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 120 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 130 */	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
/* 140 */	'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',
/* 150 */	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
/* 160 */	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
/* 170 */	'x',	'y',	'z',	'{',	'|',	'}',	'~',	0,
/* 200 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 210 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 220 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 230 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 240 */	0240,	0241,	0242,	0243,	0244,	0245,	0246,	0247,
/* 250 */	0250,	0251,	0252,	0253,	0254,	0255,	0256,	0257,
/* 260 */	0260,	0261,	0262,	0263,	0264,	0265,	0266,	0267,
/* 270 */	0270,	0271,	0272,	0273,	0274,	0275,	0276,	0277,
/* 300 */	0300,	0301,	0302,	0303,	0304,	0305,	0306,	0307,
/* 310 */	0310,	0311,	0312,	0313,	0314,	0315,	0316,	0317,
/* 320 */	0320,	0321,	0322,	0323,	0324,	0325,	0326,	0327,
/* 330 */	0330,	0331,	0332,	0333,	0334,	0335,	0336,	0337,
/* 340 */	0340,	0341,	0342,	0343,	0344,	0345,	0346,	0347,
/* 350 */	0350,	0351,	0352,	0353,	0354,	0355,	0356,	0357,
/* 360 */	0360,	0361,	0362,	0363,	0364,	0365,	0366,	0367,
/*
 * WARNING: as for above ISO sets, 0377 may be used.  Translate it to
 * itself.
 */
/* 370 */	0370,	0371,	0372,	0373,	0374,	0375,	0376,	0377,
};

/*
 * Translation table for output with OLCUC.  All ORDINARY-class characters
 * translate to themselves, except for lower-case letters which translate
 * to their upper-case equivalents.  All other characters have a zero in
 * the table, which stops the copying.
 */
static unsigned char lcuctab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
/* 050 */	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
/* 060 */	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
/* 070 */	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
/* 100 */	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 110 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 120 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 130 */	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
/* 140 */	'`',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 150 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 160 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 170 */	'X',	'Y',	'Z',	'{',	'|',	'}',	'~',	0,
/* 200 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 210 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 220 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 230 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 240 */	0240,	0241,	0242,	0243,	0244,	0245,	0246,	0247,
/* 250 */	0250,	0251,	0252,	0253,	0254,	0255,	0256,	0257,
/* 260 */	0260,	0261,	0262,	0263,	0264,	0265,	0266,	0267,
/* 270 */	0270,	0271,	0272,	0273,	0274,	0275,	0276,	0277,
/* 300 */	0300,	0301,	0302,	0303,	0304,	0305,	0306,	0307,
/* 310 */	0310,	0311,	0312,	0313,	0314,	0315,	0316,	0317,
/* 320 */	0320,	0321,	0322,	0323,	0324,	0325,	0326,	0327,
/* 330 */	0330,	0331,	0332,	0333,	0334,	0335,	0336,	0337,
/* 340 */	0340,	0341,	0342,	0343,	0344,	0345,	0346,	0347,
/* 350 */	0350,	0351,	0352,	0353,	0354,	0355,	0356,	0357,
/* 360 */	0360,	0361,	0362,	0363,	0364,	0365,	0366,	0367,
/*
 * WARNING: as for above ISO sets, 0377 may be used.  Translate it to
 * itself.
 */
/* 370 */	0370,	0371,	0372,	0373,	0374,	0375,	0376,	0377,
};

/*
 * Input mapping table -- if an entry is non-zero, and XCASE is set,
 * when the corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
static char	imaptab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	0,	'|',	0,	0,	0,	0,	0,	'`',
/* 050 */	'{',	'}',	0,	0,	0,	0,	0,	0,
/* 060 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 070 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 100 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 110 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 120 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 130 */	0,	0,	0,	0,	'\\',	0,	'~',	0,
/* 140 */	0,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 150 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 160 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 170 */	'X',	'Y',	'Z',	0,	0,	0,	0,	0,
/* 200-377 aren't mapped */
};

/*
 * Output mapping table -- if an entry is non-zero, and XCASE is set,
 * the corresponding character is printed as "\" followed by the table
 * value.  Mostly used for upper-case only terminals.
 */
static char	omaptab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 050 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 060 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 070 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 100 */	0,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 110 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 120 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 130 */	'X',	'Y',	'Z',	0,	0,	0,	0,	0,
/* 140 */	'\'',	0,	0,	0,	0,	0,	0,	0,
/* 150 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 160 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 170 */	0,	0,	0,	'(',	'!',	')',	'^',	0,
/* 200-377 aren't mapped */
};

/*
 * Translation table for TS_MEUC output without OLCUC.  All printing ASCII
 * characters translate to themselves.  All other _bytes_ have a zero in
 * the table, which stops the copying.  This and the following table exist
 * only so we can use the existing movtuc processing with or without OLCUC.
 * Maybe it speeds up something...because we can copy a block of characters
 * by only looking for zeros in the table.
 *
 * If we took the simple expedient of DISALLOWING "olcuc" with multi-byte
 * processing, we could rid ourselves of both these tables and save 512 bytes;
 * seriously, it doesn't make much sense to use olcuc with multi-byte, and
 * it will probably never be used.  Consideration should be given to disallowing
 * the combination TS_MEUC & OLCUC.
 */
static unsigned char enotrantab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
/* 050 */	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
/* 060 */	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
/* 070 */	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
/* 100 */	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 110 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 120 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 130 */	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
/* 140 */	'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',
/* 150 */	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
/* 160 */	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
/* 170 */	'x',	'y',	'z',	'{',	'|',	'}',	'~',	0,
/* 200 - 377 aren't mapped (they're stoppers). */
};

/*
 * Translation table for TS_MEUC output with OLCUC.  All printing ASCII
 * translate to themselves, except for lower-case letters which translate
 * to their upper-case equivalents.  All other bytes have a zero in
 * the table, which stops the copying.  Useless for ISO Latin Alphabet
 * translations, but *sigh* OLCUC is really only defined for ASCII anyway.
 * We only have this table so we can use the existing OLCUC processing with
 * TS_MEUC set (multi-byte mode).  Nobody would ever think of actually
 * _using_ it...would they?
 */
static unsigned char elcuctab[256] = {
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
/* 050 */	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
/* 060 */	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
/* 070 */	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
/* 100 */	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 110 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 120 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 130 */	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
/* 140 */	'`',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 150 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 160 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 170 */	'X',	'Y',	'Z',	'{',	'|',	'}',	'~',	0,
/* 200 - 377 aren't mapped (they're stoppers). */
};


#endif	/* _SYS_LDTERM_H */
