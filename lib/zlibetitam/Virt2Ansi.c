/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:Virt2Ansi.c	1.3"
/***************************************************************************
 *                                                                         *
 * name:  char *Virtual2Ansi( virtual_key )                                *
 *                                                                         *
 * function: This function returns the escape sequence associated with a   *
 *           TAM virtual key.                                              *
 *                                                                         *
 *****************************************************************************/

#include <curses.h>

#define MinVkey	KEY_BREAK
#define	MaxVkey	KEY_UNDO

/***
 *** This table must contain one entry for each virtual key in curses.h
 *** Each entry maps a particular curses virtual key into the corresponding
 *** escape sequence for the UNIX PC console.
 ***
 ***/

static char	*VkeyMap[] =
{
	0,			/* KEY_BREAK	*/
	"\033[B",		/* KEY_DOWN	*/
	"\033[A",		/* KEY_UP	*/
	"\033[D",		/* KEY_LEFT	*/
	"\033[C",		/* KEY_RIGHT	*/
	"\033[H",		/* KEY_HOME	*/
	0,			/* KEY_BACKSPACE	*/
	0,			/* KEY_F(0)	*/
	"\033Oc",		/* KEY_F(1)	*/
	"\033Od",		/* KEY_F(2)	*/
	"\033Oe",		/* KEY_F(3)	*/
	"\033Of",		/* KEY_F(4)	*/
	"\033Og",		/* KEY_F(5)	*/
	"\033Oh",		/* KEY_F(6)	*/
	"\033Oi",		/* KEY_F(7)	*/
	"\033Oj",		/* KEY_F(8)	*/
	"\033OC",		/* KEY_F(9)	*/
	"\033OD",		/* KEY_F(10)	*/
	"\033OE",		/* KEY_F(11)	*/
	"\033OF",		/* KEY_F(12)	*/
	"\033OG",		/* KEY_F(13)	*/
	"\033OH",		/* KEY_F(14)	*/
	"\033OI",		/* KEY_F(15)	*/
	"\033OJ",		/* KEY_F(16)	*/
	0,0,0,0,0,0,0,		/* KEY_F(17) - KEY_F(63) */
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	"\033Ne",		/* KEY_DL	*/
	0,			/* KEY_IL	*/
	"\033Nf",		/* KEY_DC	*/
	"\033Nj",		/* KEY_IC	*/
	0,			/* KEY_EIC	*/
	"\033[J",		/* KEY_CLEAR	*/
	0,			/* KEY_EOS	*/
	0,			/* KEY_EOL	*/
	"\033[S",		/* KEY_SF	*/
	"\033[T",		/* KEY_SR	*/
	"\033[U",		/* KEY_NPAGE	*/
	"\033[V",		/* KEY_PPAGE	*/
	0,			/* KEY_STAB	*/
	0,			/* KEY_CTAB	*/
	0,			/* KEY_CATAB	*/
	0,			/* KEY_ENTER	*/
	0,			/* KEY_SRESET	*/
	0,			/* KEY_RESET	*/
	"\033Oz",		/* KEY_PRINT	*/
	0,			/* KEY_LL	*/
	0,			/* KEY_A1	*/
	0,			/* KEY_A3	*/
	0,			/* KEY_B2	*/
	0,			/* KEY_C1	*/
	0,			/* KEY_C3	*/
	0,			/* KEY_BTAB	*/
	"\0339",		/* KEY_BEG	*/
	"\033Ow",		/* KEY_CANCEL	*/
	"\033OV",		/* KEY_CLOSE	*/
	"\033Ou",		/* KEY_COMMAND	*/
	"\033Nd",		/* KEY_COPY	*/
	"\033On",		/* KEY_CREATE	*/
	"\0330",		/* KEY_END	*/
	"\033Ok",		/* KEY_EXIT	*/
	"\033Ox",		/* KEY_FIND	*/
	"\033Om",		/* KEY_HELP	*/
	"\033Ni",		/* KEY_MARK	*/
	"\033Ol",		/* KEY_MESSAGE	*/
	"\033Nc",		/* KEY_MOVE	*/
	"\033Nh",		/* KEY_NEXT	*/
	"\033Ov",		/* KEY_OPEN	*/
	"\033Or",		/* KEY_OPTIONS	*/
	"\033Ng",		/* KEY_PREVIOUS	*/
	"\033Ot",		/* KEY_REDO	*/
	"\033OB",		/* KEY_REFERENCE	*/
	"\033Na",		/* KEY_REFRESH	*/
	"\033Oy",		/* KEY_REPLACE	*/
	"\033Ob",		/* KEY_RESTART	*/
	"\033Oq",		/* KEY_RESUME	*/
	"\033Oo",		/* KEY_SAVE	*/
	"\033NB",		/* KEY_SBEG	*/
	"\033OW",		/* KEY_SCANCEL	*/
	"\033OU",		/* KEY_SCOMMAND	*/
	"\033ND",		/* KEY_SCOPY	*/
	"\033ON",		/* KEY_SCREATE	*/
	"\033NF",		/* KEY_SDC	*/
	"\033NE",		/* KEY_SDL	*/
	"\033NI",		/* KEY_SELECT	*/
	"\033NN",		/* KEY_SEND	*/
	"\033OA",		/* KEY_SEOL	*/
	"\033OK",		/* KEY_SEXIT	*/
	"\033OX",		/* KEY_SFIND	*/
	"\033OM",		/* KEY_SHELP	*/
	"\033NM",		/* KEY_SHOME	*/
	"\033NJ",		/* KEY_SIC	*/
	"\033NK",		/* KEY_SLEFT	*/
	"\033OL",		/* KEY_SMESSAGE	*/
	"\033NC",		/* KEY_SMOVE	*/
	"\033NH",		/* KEY_SNEXT	*/
	"\033OR",		/* KEY_SOPTIONS	*/
	"\033NG",		/* KEY_SPREVIOUS	*/
	"\033OZ",		/* KEY_SPRINT	*/
	"\033OT",		/* KEY_SREDO	*/
	"\033OY",		/* KEY_SREPLACE	*/
	"\033NL",		/* KEY_SRIGHT	*/
	"\033OQ",		/* KEY_SRSUME	*/
	"\033OO",		/* KEY_SSAVE	*/
	"\033OP",		/* KEY_SSUSPEND	*/
	"\033OS",		/* KEY_SUNDO	*/
	"\033Op",		/* KEY_SUSPEND	*/
	"\033Os",		/* KEY_UNDO	*/
};

char	*Virtual2Ansi( vkey )

int	vkey;
{
	if((vkey < MinVkey) || (vkey > MaxVkey))
		return (char *) 0;

	return VkeyMap[vkey-MinVkey];
}


