/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:kcodes.h	1.2"
/*
 * This include file provides the set of 8-bit codes generated
 * by TAM from the keyboard input stream in program usable form.

char *kcodemap();
 */
#include "subcurses.h"		/* Contains defines necessary todo kcode mapping */
/*
 * 8-bit codes
 */
#define	DEL		0177
#define	Esc		033
#define	Backspace	010
#define	Tab		011
#define	BackTab		0267
#define	Break		KEY_BREAK	/* was 0377 in TAM */
#define	Reset		KEY_BREAK	/* was 0377 in TAM */
#define Mouse		0375
#define	Return		015
#define	Enter		012
#define	F1		KEY_F(1)	/* was 0321 in TAM */
#define	F2		KEY_F(2)	/* was 0322 in TAM */
#define	F3		KEY_F(3)	/* was 0323 in TAM */
#define	F4		KEY_F(4)	/* was 0324 in TAM */
#define	F5		KEY_F(5)	/* was 0325 in TAM */
#define	F6		KEY_F(6)	/* was 0326 in TAM */
#define	F7		KEY_F(7)	/* was 0327 in TAM */
#define	F8		KEY_F(8)	/* was 0330 in TAM */

#define	s_F1		KEY_F(9)	/* was 0241 in TAM */
#define	s_F2		KEY_F(10)	/* was 0242 in TAM */
#define	s_F3		KEY_F(11)	/* was 0243 in TAM */
#define	s_F4		KEY_F(12)	/* was 0244 in TAM */
#define	s_F5		KEY_F(13)	/* was 0245 in TAM */
#define	s_F6		KEY_F(14)	/* was 0246 in TAM */
#define	s_F7		KEY_F(15)	/* was 0247 in TAM */
#define	s_F8		KEY_F(16)	/* was 0250 in TAM */

#define	ClearLine	KEY_EOL		/* was 0331 in TAM */
#define	Creat		KEY_CREATE	/* was 0332 in TAM */
#define	Undo		KEY_UNDO	/* was 0333 in TAM */
#define	Find		KEY_FIND	/* was 0334 in TAM */
#define	Move		KEY_MOVE	/* was 0335 in TAM */
#define	Dlete		KEY_DL		/* was 0336 in TAM */
#define	Mark		KEY_MARK	/* was 0337 in TAM */
#define	Ref		KEY_REFERENCE	/* was 0340 in TAM */
#define	Save		KEY_SAVE	/* was 0341 in TAM */
#define	Redo		KEY_REDO	/* was 0342 in TAM */
#define	Rplac		KEY_REPLACE	/* was 0343 in TAM */
#define	Copy		KEY_COPY	/* was 0344 in TAM */
#define	DleteChar	KEY_DC		/* was 0345 in TAM */
#define	InputMode	KEY_IC		/* was 0346 in TAM */
#define	s_ClearLine	KEY_SEOL	/* was 0251 in TAM */
#define	s_Creat		KEY_SCREATE	/* was 0252 in TAM */
#define	s_Undo		KEY_SUNDO	/* was 0253 in TAM */
#define	s_Find		KEY_SFIND	/* was 0254 in TAM */
#define	s_Move		KEY_SMOVE	/* was 0255 in TAM */
#define	s_Dlete		KEY_SDL		/* was 0256 in TAM */
#define	Slect		KEY_SELECT	/* was 0257 in TAM */
#define	Rstrt		KEY_RESTART	/* was 0260 in TAM */
#define	s_Save		KEY_SSAVE	/* was 0261 in TAM */
#define	s_Redo		KEY_SREDO	/* was 0262 in TAM */
#define	s_Rplac		KEY_SREPLACE	/* was 0263 in TAM */
#define	s_Copy		KEY_SCOPY	/* was 0264 in TAM */
#define	s_DleteChar	KEY_SDC		/* was 0265 in TAM */
#define	s_InputMode	KEY_SIC		/* was 0266 in TAM */
#define	Exit		KEY_EXIT	/* was 0350 in TAM */
#define	Suspd		KEY_SUSPEND	/* was 0351 in TAM */
#define	Cmd		KEY_COMMAND	/* was 0352 in TAM */
#define	Print		KEY_PRINT	/* was 0353 in TAM */
#define	Beg		KEY_BEG		/* was 0354 in TAM */
#define	Prev		KEY_PREVIOUS	/* was 0355 in TAM */
#define	Back		KEY_LEFT	/* was 0356 in TAM */
#define	Msg		KEY_MESSAGE	/* was 0357 in TAM */
#define	Rsume		KEY_RESUME	/* was 0360 in TAM */
#define	Open		KEY_OPEN	/* was 0361 in TAM */
#define	Rfrsh		KEY_REFRESH	/* was 0362 in TAM */
#define	Home		KEY_HOME	/* was 0363 in TAM */
#define	Up		KEY_UP		/* was 0364 in TAM */
#define	Down		KEY_DOWN	/* was 0365 in TAM */
#define	Help		KEY_HELP	/* was 0366 in TAM */
#define	Opts		KEY_OPTIONS	/* was 0367 in TAM */
#define	Cancl		KEY_CANCEL	/* was 0370 in TAM */
#define	Page		KEY_NPAGE	/* was 0371 in TAM */
#define	End		KEY_END		/* was 0372 in TAM */
#define	Next		KEY_NEXT	/* was 0373 in TAM */
#define	Forward		KEY_RIGHT	/* was 0374 in TAM */
#ifndef S4
#define	Redraw		KEY_REFRESH	/* was 0376 in TAM */
#endif
#define	s_Exit		KEY_SEXIT	/* was 0270 in TAM */
#define	s_Suspd		KEY_SSUSPEND	/* was 0271 in TAM */
#define	s_Cmd		KEY_SCOMMAND	/* was 0272 in TAM */
#define	s_Print		KEY_SPRINT	/* was 0273 in TAM */
#define	s_Beg		KEY_SBEG	/* was 0274 in TAM */
#define	s_Prev		KEY_SPREVIOUS	/* was 0275 in TAM */
#define	s_Back		KEY_SLEFT	/* was 0276 in TAM */
#define	s_Msg		KEY_SMESSAGE	/* was 0277 in TAM */
#define	s_Rsume		KEY_SRSUME	/* was 0300 in TAM */
#define	Close		KEY_CLOSE	/* was 0301 in TAM */
#define	Clear		KEY_CLEAR	/* was 0302 in TAM */
#define	s_Home		KEY_SHOME	/* was 0303 in TAM */
#define	RollUp		KEY_SR		/* was 0304 in TAM */
#define	RollDn		KEY_SF		/* was 0305 in TAM */
#define	s_Help		KEY_SHELP	/* was 0306 in TAM */
#define	s_Opts		KEY_SOPTIONS	/* was 0307 in TAM */
#define	s_Cancl		KEY_SCANCEL	/* was 0310 in TAM */
#define	s_Page		KEY_PPAGE	/* was 0311 in TAM */
#define	s_End		KEY_SEND	/* was 0312 in TAM */
#define	s_Next		KEY_SNEXT	/* was 0313 in TAM */
#define	s_Forward	KEY_SRIGHT	/* was 0314 in TAM */
#define PF1		KEY_F(17)	/* was 0201 in TAM */
#define PF2		KEY_F(18)	/* was 0202 in TAM */
#define PF3		KEY_F(19)	/* was 0203 in TAM */
#define PF4		KEY_F(20)	/* was 0204 in TAM */
#define PF5		KEY_F(21)	/* was 0205 in TAM */
#define PF6		KEY_F(22)	/* was 0206 in TAM */
#define PF7		KEY_F(23)	/* was 0207 in TAM */
#define PF8		KEY_F(24)	/* was 0210 in TAM */
#define PF9		KEY_F(25)	/* was 0211 in TAM */
#define PF10		KEY_F(26)	/* was 0212 in TAM */
#define PF11		KEY_F(27)	/* was 0213 in TAM */
#define PF12		KEY_F(28)	/* was 0214 in TAM */
