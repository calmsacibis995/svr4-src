/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:llib-lcvt.c	1.2"
#include "cvttam.h"
#include "path.h"
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

static unsigned _uns;
static char *_cp;
static TAMWIN *_twp;

int	ReadMagic (CursesKey) int CursesKey; {return _uns;}
char	*Virtual2Ansi (vkey) int vkey; {return _cp;}
int	TAMinitscr () {return 0;}
int	TAMcbreak () {return 0;}
int	TAMnocbreak () {return 0;}
int	TAMecho () {return 0;}
int	TAMnoecho () {return 0;}
int	TAMinch () {return 0;}
int	TAMgetch () {return 0;}
int	TAMflushinp () {return 0;}
int	TAMattron (m) long m; {return 0;}
int	TAMattroff (m) long m; {return 0;}
int	TAMsavetty () {return 0;}
int	TAMresetty () {return 0;}
void	_noncurrent (tw) TAMWIN *tw; {}
void	_current (tw) TAMWIN *tw; {}
void	_doborder (tw) TAMWIN *tw; {}
void	_envinit() {}
int	TAMkeypad (wn, flag) int wn; int flag; {return 0;}
TAMWIN	*_listdel (list, lp) TAMWINLIST *list; TAMWIN *lp; {return lp;}
void	_listadd (list, lp) TAMWINLIST *list; TAMWIN *lp; {}
void	_post (tw) TAMWIN *tw; {}
int	TAMbeep () {return 0;}
int	TAMclear (wn) short wn; {return 0;}
int	TAMclearok (wn, flag) short wn; int flag; {return 0;}
int	TAMclrtobot (wn) short wn; {return 0;}
int	TAMclrtoeol (wn) short wn; {return 0;}
int	TAMdelch (wn) short wn; {return 0;}
int	TAMdeleteln (wn) short wn; {return 0;}
int	TAMinsch (wn, c) short wn; {return 0;}
int	TAMinsertln (wn) short wn; {return 0;}
void	_undowindow (w) WINDOW *w; {}
TAMWIN	*_validwindow (wn) short wn; {return _twp;}
int	TAMwcmd (wn, c) short wn; char *c; {return 0;}
int	wcreate (r, c, h, w, f) short r, c, h, w; unsigned short f; {return 0;}
int	TAMwcreate (r, c, h, w, f) short r, c, h, w; unsigned short f; {return 0;}
int	wdelete (tw) TAMWIN *tw; {return 0;}
int	TAMwdelete (w) short w; {return 0;}
int	TAMwerase (wn) short wn; {return 0;}
void	TAMwexit (s) int s; {}
int	TAMwgetc (w) short w; {return 0;}
int	TAMwgetpos (wn, r, c) short wn, *r, *c; {return 0;}
int	TAMwgetsel() {return 0;}
int	TAMwgetstat (w, s) short w; WSTAT *s; {return 0;}
int	TAMwgoto (wn, r, c) short wn, r, c; {return 0;}
void	TAMwinit () {}
int	_winsize (r, c, h, w, f) int r, c, h, w, f; {return 0;}
int	TAMwlabel (wn, c) short wn; char *c; {return 0;}
int	TAMwnl (wn, flag) short wn; int flag; {return 0;}
int	TAMwnodelay (wn, flag) short wn; int flag; {return 0;}
int	TAMwpostwait () {return 0;}
int	TAMwprexec () {return 0;}
	/*VARARGS*/
int	TAMwprintf (w, format, args) short w; char *format;  {return 0;}
	/*VARARGS*/
int	TAMprintw (format, args) char *format; {return 0;}
int	TAMwprompt (wn, c) short wn; char *c; {return 0;}
int	TAMwputc (wn, c) short wn; char c; {return 0;}
int	TAMwputs (wn, s) short wn; char *s; {return 0;}
int	TAMwrefresh (w) short w; {return 0;}
int	TAMwselect (w) short w; {return 0;}
int	TAMwsetstat (wn, wsp) short wn; WSTAT *wsp; {return 0;}
int	TAMwslk (wn, kn, slong, sshort, sextra) short wn, kn; char *slong, *sshort, *sextra; {return 0;}
int	TAMwuser (wn, c) short wn; char *c; {return 0;}
