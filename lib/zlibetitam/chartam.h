/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:chartam.h	1.12"
/********************************************************************
 *                         chartam.h                                *
 ********************************************************************
 *                                                                  *
 * This file attempts to successfully map all tam function calls    *
 * to function calls within the conversion library for character    *
 * terminals.  If a particular function is not supported in the     *
 * conversion library then nasty preprocessor demons should prevent *
 * user source from compiling.                                      *
 *                                                                  *
 ********************************************************************/

/* The following is from window.h */

#define NBORDER		0x1	/* Borderless */
#define TRUE		(1)
#define FALSE		(0)
#define CERR		(-1)

/***
 *** First define away all true TAM entry points
 ***/

#ifndef lint

#define winit()			TAMwinit()
#define iswind()		0
#define wtargeton()		0
#define wrastop(w,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13)	0
#define wcreate(r,c,h,w,f)	TAMwcreate((short)(r),(short)(c),(short)(h),(short)(w),(unsigned short) (f))

#define wexit(s)		TAMwexit(s)
#define wdelete(w)		TAMwdelete((short)(w))
#define wselect(w)		TAMwselect((short)(w))
#define wgetsel()		TAMwgetsel()
#define wgetstat(w,s)		TAMwgetstat((short)(w),(WSTAT *)(s))
#define wsetstat(w,s)		TAMwsetstat((short)(w),(WSTAT *)(s))
#define wputc(w,c)		TAMwputc((short)(w),(char)(c))
#define wputs(w,c)		TAMwputs((short)(w),(char*)(c))
#define wprintf			TAMwprintf
#define printw			TAMprintw
#define wslk			TAMwslk			/* 2 forms of this call */
#define wcmd(w,c)		TAMwcmd((short)(w),(char*)(c))
#define wprompt(w,c)		TAMwprompt((short)(w),(char*)(c))
#define wlabel(w,c)		TAMwlabel((short)(w),(char*)(c))
#define wrefresh(w)		TAMwrefresh((short)(w))
#define wuser(w,c)		TAMwuser((short)(w),(char*)(c))
#define wgoto(w,r,c)		TAMwgoto((short)(w),(short)(r),(short)(c))
#define wgetpos(w,r,c)		TAMwgetpos((short)(w),(int*)(r),(int*)(c))
#define wgetc(w)		TAMwgetc((short)(w))
#define kcodemap(c)		(char *)Virtual2Ansi((unsigned int)(c))
#define keypad(d,f)		TAMkeypad((int)(d),(int)(f))
#define wsetmouse(w,ms)		0
#define wgetmouse(w,ms)		0
#define wreadmouse(w,a,b,c,d)	0
#define wprexec()		TAMwprexec()
#define wpostwait()		TAMwpostwait()
#define wnl(w,f)		0
#define wicon(w, r, c, i)	0
#define wicoff(w, r, c, i)	0
#define track(w, t, o, b, w1)	TAMtrack ((short)(w))
#define wndelay(w, b)		TAMwnodelay((short)(w),(int)(b))

/***
 *** Compatibility routines from the TAM file "wcurses.c"
 ***/

#define initscr()		TAMinitscr()
#define nl()			0
#define nonl()			0
#define cbreak()		TAMcbreak()
#define nocbreak()		TAMnocbreak()
#define echo()			TAMecho()
#define noecho()		TAMnoecho()
#define inch()			TAMinch()
#define getch()			TAMgetch()
#define flushinp()		TAMflushinp()
#define attron(m)		TAMattron((long)(m))
#define attroff(m)		TAMattroff((long)(m))
#define savetty()		TAMsavetty()
#define resetty()		TAMresetty()

/***
 *** Define TAM entry points that are really #define's in TAM
 ***/

#define stdscr			wncur			/* In tam.h */
#define addch(c)		wputc(wncur,c)
#define addstr(s)		wputs(wncur,s)
#define beep()			TAMbeep()
#define clear()			TAMclear((short)(wncur))
#define clearok(dum1, dum2)
#define clrtobot()		TAMclrtobot((short)(wncur))
#define clrtoeol()		TAMclrtoeol((short)(wncur))
#define delch()			TAMdelch((short)(wncur))
#define deleteln()		TAMdeleteln((short)(wncur))
#define erase()			clear()
#define flash()			beep()
#define getyx(d,r,c)		wgetpos(wncur,&r,&c);
#define insch(c)		TAMinsch((short)(wncur), (char)(c))
#define insertln()		TAMinsertln((short)(wncur))
#define leaveok(a,b)		0
#define move(r,c)		wgoto(wncur,r,c)
#define mvaddch(r,c,ch)		(move(r,c)==CERR?CERR:addch(ch))
#define mvaddstr(r,c,s)		(move(r,c)==CERR?CERR:addstr(s))
#define mvinch(r,c)		(move(r,c)==CERR?CERR:inch())
#define nodelay(d,b)		TAMwnodelay((short)(wncur),(int)(b))
#define refresh()		wrefresh(wncur)

extern void TAMwinit ();
extern void TAMwexit ();

#else

extern void winit ();
extern void wexit ();

#endif /* lint */
