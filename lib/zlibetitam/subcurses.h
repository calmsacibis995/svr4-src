/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:subcurses.h	1.1"
#define KEY_BREAK	0401		/* break key (unreliable) */
#define KEY_DOWN	0402		/* The four arrow keys ... */
#define KEY_UP		0403
#define KEY_LEFT	0404
#define KEY_RIGHT	0405		/* ... */
#define KEY_HOME	0406		/* Home key (upward+left arrow) */
#define KEY_BACKSPACE	0407		/* backspace (unreliable) */
#define KEY_F0		0410		/* Function keys.  Space for 64 */
#define KEY_F(n)	(KEY_F0+(n))	/* keys is reserved. */
#define KEY_DL		0510		/* Delete line */
#define KEY_IL		0511		/* Insert line */
#define KEY_DC		0512		/* Delete character */
#define KEY_IC		0513		/* Insert char or enter insert mode */
#define KEY_EIC		0514		/* Exit insert char mode */
#define KEY_CLEAR	0515		/* Clear screen */
#define KEY_EOS		0516		/* Clear to end of screen */
#define KEY_EOL		0517		/* Clear to end of line */
#define KEY_SF		0520		/* Scroll 1 line forward */
#define KEY_SR		0521		/* Scroll 1 line backwards (reverse) */
#define KEY_NPAGE	0522		/* Next page */
#define KEY_PPAGE	0523		/* Previous page */
#define KEY_STAB	0524		/* Set tab */
#define KEY_CTAB	0525		/* Clear tab */
#define KEY_CATAB	0526		/* Clear all tabs */
#define KEY_ENTER	0527		/* Enter or send (unreliable) */
#define KEY_SRESET	0530		/* soft (partial) reset (unreliable) */
#define KEY_RESET	0531		/* reset or hard reset (unreliable) */
#define KEY_PRINT	0532		/* print or copy */
#define KEY_LL		0533		/* home down or bottom (lower left) */
					/* The keypad is arranged like this: */
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		0534		/* upper left of keypad */
#define KEY_A3		0535		/* upper right of keypad */
#define KEY_B2		0536		/* center of keypad */
#define KEY_C1		0537		/* lower left of keypad */
#define KEY_C3		0540		/* lower right of keypad */
#define KEY_BTAB	0541		/* Back tab key */
#define KEY_BEG		0542		/* beg(inning) key */
#define KEY_CANCEL	0543		/* cancel key */
#define KEY_CLOSE	0544		/* close key */
#define KEY_COMMAND	0545		/* cmd (command) key */
#define KEY_COPY	0546		/* copy key */
#define KEY_CREATE	0547		/* create key */
#define KEY_END		0550		/* end key */
#define KEY_EXIT	0551		/* exit key */
#define KEY_FIND	0552		/* find key */
#define KEY_HELP	0553		/* help key */
#define KEY_MARK	0554		/* mark key */
#define KEY_MESSAGE	0555		/* message key */
#define KEY_MOVE	0556		/* move key */
#define KEY_NEXT	0557		/* next object key */
#define KEY_OPEN	0560		/* open key */
#define KEY_OPTIONS	0561		/* options key */
#define KEY_PREVIOUS	0562		/* previous object key */
#define KEY_REDO	0563		/* redo key */
#define KEY_REFERENCE	0564		/* ref(erence) key */
#define KEY_REFRESH	0565		/* refresh key */
#define KEY_REPLACE	0566		/* replace key */
#define KEY_RESTART	0567		/* restart key */
#define KEY_RESUME	0570		/* resume key */
#define KEY_SAVE	0571		/* save key */
#define KEY_SBEG	0572		/* shifted beginning key */
#define KEY_SCANCEL	0573		/* shifted cancel key */
#define KEY_SCOMMAND	0574		/* shifted command key */
#define KEY_SCOPY	0575		/* shifted copy key */
#define KEY_SCREATE	0576		/* shifted create key */
#define KEY_SDC		0577		/* shifted delete char key */
#define KEY_SDL		0600		/* shifted delete line key */
#define KEY_SELECT	0601		/* select key */
#define KEY_SEND	0602		/* shifted end key */
#define KEY_SEOL	0603		/* shifted clear line key */
#define KEY_SEXIT	0604		/* shifted exit key */
#define KEY_SFIND	0605		/* shifted find key */
#define KEY_SHELP	0606		/* shifted help key */
#define KEY_SHOME	0607		/* shifted home key */
#define KEY_SIC		0610		/* shifted input key */
#define KEY_SLEFT	0611		/* shifted left arrow key */
#define KEY_SMESSAGE	0612		/* shifted message key */
#define KEY_SMOVE	0613		/* shifted move key */
#define KEY_SNEXT	0614		/* shifted next key */
#define KEY_SOPTIONS	0615		/* shifted options key */
#define KEY_SPREVIOUS	0616		/* shifted prev key */
#define KEY_SPRINT	0617		/* shifted print key */
#define KEY_SREDO	0620		/* shifted redo key */
#define KEY_SREPLACE	0621		/* shifted replace key */
#define KEY_SRIGHT	0622		/* shifted right arrow */
#define KEY_SRSUME	0623		/* shifted resume key */
#define KEY_SSAVE	0624		/* shifted save key */
#define KEY_SSUSPEND	0625		/* shifted suspend key */
#define KEY_SUNDO	0626		/* shifted undo key */
#define KEY_SUSPEND	0627		/* suspend key */
#define KEY_UNDO	0630		/* undo key */
