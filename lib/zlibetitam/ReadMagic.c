/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:ReadMagic.c	1.4"
/**************************************************************************
 *                         ReadMagic                                      *
 **************************************************************************
 *                                                                        *
 *  name:      ReadMagic                                                  *
 *                                                                        *
 *  function:  Convert UNIX PC console escape sequences into their        *
 *             equivalent SVR3 curses virtual key.                        *
 *                                                                        *
 *  input:     CursesKey  --  a virtual key from curses.                  *
 *                                                                        *
 *  output:    0          --  no sequence matched yet.                    *
 *             -1         --  escape sequence aborted midstream.          *
 *             N>0        --  a virtual curses key.                       *
 *                                                                        *
 **************************************************************************/

#include "cvttam.h"

#define	NOCHAR			0	/* No characters available     */
#define BRKESCAPE		-1	/* Broken escape sequence      */
#define Comb(a,b)		a<<8 | b

typedef enum {				/* States for recognizer       */
  BeforeEscape,
  StartEscape,
  EscapeFirstChar
} MachineState;


int
ReadMagic (CursesKey)
int CursesKey;
{
  static MachineState	MState = BeforeEscape;
  static unsigned	answer;
  static int		Aborted = 0;
  static int		lastchar;
  register unsigned int	i;


  if (!CursesKey) {
    return NOCHAR;
  }

  switch (MState) {

    case BeforeEscape: {

      switch (CursesKey) {

	case '\033': {
	  MState = StartEscape;
	  answer = NOCHAR;
	  break;
	}
	default: {
	  answer = CursesKey;
	  break;
	}
      }
      break;
    }

    case StartEscape: {		/* Last char was an escape */

      MState = BeforeEscape;
      switch (CursesKey) {
	
	case '1':		/* F1 */
	case '2':		/* F2 */
	case '3':		/* F3 */
	case '4':		/* F4 */
	case '5':		/* F5 */
	case '6':		/* F6 */
	case '7':		/* F7 */
	case '8': {		/* F8 */
	  answer = KEY_F(CursesKey-'0');
	  break;
	}
	case '!': {		/* s_F1 */
	  answer = KEY_F(9);
	  break;
	}
	case '@': {		/* s_F2 */
	  answer = KEY_F(10);
	  break;
	}
	case '#': {		/* s_F3 */
	  answer = KEY_F(11);
	  break;
	}
	case '$': {		/* s_F4 */
	  answer = KEY_F(12);
	  break;
	}
	case '%': {		/* s_F5 */
	  answer = KEY_F(13);
	  break;
	}
	case '^': {		/* s_F6 */
	  answer = KEY_F(14);
	  break;
	}
	case '&': {		/* s_F7 */
	  answer = KEY_F(15);
	  break;
	}
	case '*': {		/* s_F8 */
	  answer = KEY_F(16);
	  break;
	}
	case 't': {		/* Tab */
	  answer = '\t';
	  break;
	}
	case '?': {		/* Help */
	  answer = KEY_HELP;
	  break;
	}
	case '\177': {		/* Dlete Char */
	  answer = KEY_DC;
	  break;
	}
	case '\033': {		/* Esc */
	  answer = CursesKey;
	  break;
	}
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'H':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'R':
	case 'S':
	case 'U': {
	  answer = NOCHAR;
	  MState = EscapeFirstChar;
	  lastchar = CursesKey;
	  break;
	}
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'h':
	case 'i':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'r':
	case 's':
	case 'u': {
	  answer = NOCHAR;
	  MState = EscapeFirstChar;
	  lastchar = CursesKey;
	  break;
	}
	default: {
	  Aborted = 1;
	  answer = NOCHAR;
	  break;
	}
      }
      break;
    }

    case EscapeFirstChar: {
      i = Comb (lastchar, CursesKey);
      switch (i) {
	case Comb('b','g'): {			/* Beg */
	  answer = KEY_BEG;
          break;
	}
	case Comb('b','r'): {			/* Break */
	  answer = KEY_BREAK;
          break;
	}
	case Comb('b','w'): {			/* Left arrow */
	  answer = KEY_LEFT;
          break;
	}
	case Comb('c','e'): {			/* Clear */
	  answer = KEY_CLEAR;
          break;
	}
	case Comb('c','i'): {			/* ClearLine */
	  answer = KEY_EOL;
          break;
	}
	case Comb('c','l'): {			/* Close */
	  answer = KEY_CLOSE;
          break;
	}
	case Comb('c','m'): {			/* Cmd */
	  answer = KEY_COMMAND;
          break;
	}
	case Comb('c','n'): {			/* Cancl */
	  answer = KEY_CANCEL;
          break;
	}
	case Comb('c','p'): {			/* Copy */
	  answer = KEY_COPY;
          break;
	}
	case Comb('c','r'): {			/* Creat */
	  answer = KEY_CREATE;
          break;
	}
	case Comb('d','c'): {			/* DleteChar */
	  answer = KEY_DC;
          break;
	}
	case Comb('d','l'): {			/* Dlete */
	  answer = KEY_DL;
          break;
	}
	case Comb('d','n'): {			/* Down arrow */
	  answer = KEY_DOWN;
          break;
	}
	case Comb('e','n'): {			/* End */
	  answer = KEY_END;
          break;
	}
	case Comb('e','x'): {			/* Exit */
	  answer = KEY_EXIT;
          break;
	}
	case Comb('f','1'): {			/* PF1 */
	  answer = KEY_F(17);
	  break;
	}
	case Comb('f','2'): {			/* PF2 */
	  answer = KEY_F(18);
	  break;
	}
	case Comb('f','3'): {			/* PF3 */
	  answer = KEY_F(19);
	  break;
	}
	case Comb('f','4'): {			/* PF4 */
	  answer = KEY_F(20);
	  break;
	}
	case Comb('f','5'): {			/* PF5 */
	  answer = KEY_F(21);
	  break;
	}
	case Comb('f','6'): {			/* PF6 */
	  answer = KEY_F(22);
	  break;
	}
	case Comb('f','7'): {			/* PF7 */
	  answer = KEY_F(23);
	  break;
	}
	case Comb('f','8'): {			/* PF8 */
	  answer = KEY_F(24);
	  break;
	}
	case Comb('f','9'): {			/* PF9 */
	  answer = KEY_F(25);
	  break;
	}
	case Comb('f','0'): {			/* PF10 */
	  answer = KEY_F(26);
	  break;
	}
	case Comb('f','-'): {			/* PF11 */
	  answer = KEY_F(27);
	  break;
	}
	case Comb('f','='): {			/* PF12 */
	  answer = KEY_F(28);
	  break;
	}
	case Comb('f','i'): {			/* Find */
	  answer = KEY_FIND;
          break;
	}
	case Comb('f','w'): {			/* Forward */
	  answer = KEY_RIGHT;
          break;
	}
	case Comb('h','l'): {			/* Help */
	  answer = KEY_HELP;
          break;
	}
	case Comb('h','m'): {			/* Home */
	  answer = KEY_HOME;
          break;
	}
	case Comb('i','m'): {			/* Input Mode */
	  answer = KEY_IC;
          break;
	}
	case Comb('m','k'): {			/* Mark */
	  answer = KEY_MARK;
          break;
	}
	case Comb('m','s'): {			/* Msg */
	  answer = KEY_MESSAGE;
          break;
	}
	case Comb('m','v'): {			/* Move */
	  answer = KEY_MOVE;
          break;
	}
	case Comb('n','x'): {			/* Next */
	  answer = KEY_NEXT;
          break;
	}
	case Comb('o','p'): {			/* Open */
	  answer = KEY_OPEN;
          break;
	}
	case Comb('o','t'): {			/* Opts */
	  answer = KEY_OPTIONS;
          break;
	}
	case Comb('p','g'): {			/* Page */
	  answer = KEY_NPAGE;
          break;
	}
	case Comb('p','r'): {			/* Print */
	  answer = KEY_PRINT;
          break;
	}
	case Comb('p','v'): {			/* Prev */
	  answer = KEY_PREVIOUS;
          break;
	}
	case Comb('r','d'): {			/* RollDn */
	  answer = KEY_SF;
          break;
	}
	case Comb('r','e'): {			/* Ref */
	  answer = KEY_REFERENCE;
          break;
	}
	case Comb('r','f'): {			/* Rfrsh */
	  answer = KEY_REFRESH;
          break;
	}
	case Comb('r','m'): {			/* Rsume */
	  answer = KEY_RESUME;
          break;
	}
	case Comb('r','o'): {			/* Redo */
	  answer = KEY_REDO;
          break;
	}
	case Comb('r','p'): {			/* Rplac */
	  answer = KEY_REPLACE;
          break;
	}
	case Comb('r','s'): {			/* Rstrt */
	  answer = KEY_RESTART;
          break;
	}
	case Comb('r','u'): {			/* RollUp */
	  answer = KEY_SR;
          break;
	}
	case Comb('r','x'): {			/* Reset */
	  answer = KEY_BREAK;
          break;
	}
	case Comb('s','l'): {			/* Slect */
	  answer = KEY_SELECT;
          break;
	}
	case Comb('s','s'): {			/* Suspd */
	  answer = KEY_SUSPEND;
          break;
	}
	case Comb('s','v'): {			/* Save */
	  answer = KEY_SAVE;
          break;
	}
	case Comb('u','d'): {			/* Undo */
	  answer = KEY_UNDO;
          break;
	}
	case Comb('u','p'): {			/* Up */
	  answer = KEY_UP;
          break;
	}
	case Comb('B','G'): {			/* s_Beg */
	  answer = KEY_SBEG;
          break;
	}
	case Comb('B','R'): {			/* s_Break */
	  answer = KEY_BREAK;
          break;
	}
	case Comb('B','W'): {			/* s_Back */
	  answer = KEY_SLEFT;
          break;
	}
	case Comb('C','E'): {			/* s_Clear */
	  answer = KEY_CLEAR;
          break;
	}
	case Comb('C','I'): {			/* s_ClearLine */
	  answer = KEY_SEOL;
          break;
	}
	case Comb('C','L'): {			/* s_Close */
	  answer = KEY_CLOSE;
          break;
	}
	case Comb('C','M'): {			/* s_Cmd */
	  answer = KEY_SCOMMAND;
          break;
	}
	case Comb('C','N'): {			/* s_Cancl */
	  answer = KEY_SCANCEL;
          break;
	}
	case Comb('C','P'): {			/* s_Copy */
	  answer = KEY_SCOPY;
          break;
	}
	case Comb('C','R'): {			/* s_Creat */
	  answer = KEY_SCREATE;
          break;
	}
	case Comb('D','C'): {			/* s_DleteChar */
	  answer = KEY_SDC;
          break;
	}
	case Comb('D','L'): {			/* s_Dlete */
	  answer = KEY_SDL;
          break;
	}
	case Comb('D','N'): {			/* RollDn */
	  answer = KEY_SF;
          break;
	}
	case Comb('E','N'): {			/* s_End */
	  answer = KEY_SEND;
          break;
	}
	case Comb('E','X'): {			/* s_Exit */
	  answer = KEY_SEXIT;
          break;
	}
	case Comb('F','I'): {			/* s_Find */
	  answer = KEY_SFIND;
          break;
	}
	case Comb('F','W'): {			/* s_Forward */
	  answer = KEY_SRIGHT;
          break;
	}
	case Comb('H','L'): {			/* s_Help */
	  answer = KEY_SHELP;
          break;
	}
	case Comb('H','M'): {			/* s_Home */
	  answer = KEY_SHOME;
          break;
	}
	case Comb('M','K'): {			/* s_Mark */
	  answer = KEY_SELECT;
          break;
	}
	case Comb('M','S'): {			/* s_Msg */
	  answer = KEY_SMESSAGE;
          break;
	}
	case Comb('M','V'): {			/* s_Move */
	  answer = KEY_SMOVE;
          break;
	}
	case Comb('N','J'): {			/* s_InputMode */
	  answer = KEY_SIC;
          break;
	}
	case Comb('N','X'): {			/* s_Next */
	  answer = KEY_SNEXT;
          break;
	}
	case Comb('O','P'): {			/* s_Open */
	  answer = KEY_CLOSE;
          break;
	}
	case Comb('O','T'): {			/* s_Opts */
	  answer = KEY_SOPTIONS;
          break;
	}
	case Comb('P','G'): {			/* s_Page */
	  answer = KEY_PPAGE;
          break;
	}
	case Comb('P','R'): {			/* s_Print */
	  answer = KEY_SPRINT;
          break;
	}
	case Comb('P','V'): {			/* s_Prev */
	  answer = KEY_SPREVIOUS;
          break;
	}
	case Comb('R','D'): {			/* s_RollDn */
	  answer = KEY_SF;
          break;
	}
	case Comb('R','E'): {			/* s_Ref */
	  answer = KEY_RESTART;
          break;
	}
	case Comb('R','F'): {			/* s_Rfrsh */
	  answer = KEY_CLEAR;
          break;
	}
	case Comb('R','M'): {			/* s_Rsume */
	  answer = KEY_SRSUME;
          break;
	}
	case Comb('R','O'): {			/* s_Redo */
	  answer = KEY_SREDO;
          break;
	}
	case Comb('R','P'): {			/* s_Rplac */
	  answer = KEY_SREPLACE;
          break;
	}
	case Comb('R','S'): {			/* s_Rstrt */
	  answer = KEY_RESTART;
          break;
	}
	case Comb('R','U'): {			/* s_RollUp */
	  answer = KEY_SR;
          break;
	}
	case Comb('S','L'): {			/* s_Slect */
	  answer = KEY_SELECT;
          break;
	}
	case Comb('S','S'): {			/* s_Suspd */
	  answer = KEY_SSUSPEND;
          break;
	}
	case Comb('S','V'): {			/* s_Save */
	  answer = KEY_SSAVE;
          break;
	}
	case Comb('U','D'): {			/* s_Undo */
	  answer = KEY_SUNDO;
          break;
	}
	case Comb('U','P'): {			/* s_Up */
	  answer = KEY_SR;
          break;
	}
	default: {
	  Aborted = 1;
	  answer = NOCHAR;
	  break;
	}
      }
      MState = BeforeEscape;
      break;
    }
  }

  if (Aborted) {
    Aborted = 0;
    MState = BeforeEscape;
    return BRKESCAPE;
  }
  return answer;
}
