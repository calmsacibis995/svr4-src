/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:llib-ltam.c	1.3"
/*LINTLIBRARY*/
#include "tam.h"
#include "message.h"
#include "form.h"
#include "wind.h"
#include "kcodes.h"
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "menu.h"
#include <ctype.h>
#include <errno.h>
#include "path.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "pbf.h"
#include "track.h"
#include "sys/window.h"

static	char *_cp;
int	LINES,
	COLS;
short	wncur;

int	initscr () {return 0;}
int	cbreak () {return 0;}
int	nocbreak () {return 0;}
int	echo () {return 0;}
int	noecho () {return 0;}
int	inch () {return 0;}
int	getch () {return 0;}
int	flushinp () {return 0;}
int	attron (m) long m; {return 0;}
int	attroff (m) long m; {return 0;}
int	savetty () {return 0;}
int	resetty () {return 0;}
int	keypad (wn, flag) int wn; int flag; {return 0;}
int	beep () {return 0;}
int	clear (wn) short wn; {return 0;}
int	clearok (wn, flag) short wn; int flag; {return 0;}
int	clrtobot (wn) short wn; {return 0;}
int	clrtoeol (wn) short wn; {return 0;}
int	delch (wn) short wn; {return 0;}
int	deleteln (wn) short wn; {return 0;}
int	insch (wn, c) short wn; {return 0;}
int	insertln (wn) short wn; {return 0;}
int	wcmd (wn, c) short wn; char *c; {return 0;}
int	wcreate (r, c, h, w, f) short r, c, h, w; unsigned short f; {return 0;}
int	wdelete (w) short w; {return 0;}
int	werase (wn) short wn; {return 0;}
void	wexit (s) int s; {}
int	wgetc (w) short w; {return 0;}
int	wgetpos (wn, r, c) short wn, *r, *c; {return 0;}
int	wgetsel() {return 0;}
int	wgetstat (w, s) short w; WSTAT *s; {return 0;}
int	wgoto (wn, r, c) short wn, r, c; {return 0;}
void	winit () {}
int	wlabel (wn, c) short wn; char *c; {return 0;}
int	wnl (wn, flag) short wn; int flag; {return 0;}
int	wnodelay (wn, flag) short wn; int flag; {return 0;}
int	wpostwait () {return 0;}
int	wprexec () {return 0;}
	/*VARARGS*/
int	wprintf (w, format, args) short w; char *format;  {return 0;}
	/*VARARGS*/
int	printw (format, args) char *format; {return 0;}
int	wprompt (wn, c) short wn; char *c; {return 0;}
int	wputc (wn, c) short wn; char c; {return 0;}
int	wputs (wn, s) short wn; char *s; {return 0;}
int	wrefresh (w) short w; {return 0;}
int	wselect (w) short w; {return 0;}
int	wsetstat (wn, wsp) short wn; WSTAT *wsp; {return 0;}
int	wslk (wn, kn, slong, sshort, sextra) short wn, kn; char *slong, *sshort, *sextra; {return 0;}
int	wuser (wn, c) short wn; char *c; {return 0;}
int	form(f, op) form_t *f; int op; {return 0;}
int	form_2(f, op) form_t *f; int op; {return 0;}
int	field(w, f, fl, tf, bf, doin, pval, okr, okc, width)
	int w;
	form_t *f;
	field_t *fl;
	char tf, bf, doin;
	int *pval, okr, okc;
	short width;
	{return 0;}
int	flmenu(fl) field_t *fl; {return 0;}
field_t	*fright(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fleft(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fdown(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fup(f, cfl) form_t *f; field_t *cfl; {return cfl;}
void	disp_ins(w, label, width) char *label; short width; {}
int	mbegin(m) menu_t *m; {return 0;}
int	mcitems(m, pwidth) menu_t *m; int *pwidth; {return 0;}
void	nctitle(m,pheight,pwidth) menu_t *m; int *pheight,*pwidth; {}
void	mdisplay(m,supress,nitems,maxwidth,titheight,titwidth,prows,pvcols)
	menu_t *m;
	int supress,nitems,maxwidth,titheight,titwidth;
	int *prows, *pvcols;
	{}
void	mend(m) menu_t *m; {}
int	menu(m, op) menu_t *m; int op; {return 0;}
int	minput(m) menu_t *m; {return 0;}
int	mmatch(m) menu_t *m; {return 0;}
void	mscroll(m,nitems,rows,n) menu_t *m; int nitems,rows,n; {}
int	mshape(nitems,maxwidth,prows,pcols) int nitems,maxwidth, *prows,*pcols; {return 0;}
void	mtitle(m,width) menu_t *m; int width; {}
void	mtrunc(s1,s2,n) char *s1, *s2; int n; {}
	/*VARARGS*/
int	message(mtype, file, title, format, args) int mtype; char *file, *title, *format; {return 0;}
int	_domsg (mtype, file, title, ptr) int mtype; char *file, *title, *ptr; {return 0;}
int	_mwcr (height, top_ju, help_fl) int height; char top_ju, help_fl; {return 0;}
int	exhelp (file, title) char *file, *title; {return 0;}
FILE	* pb_open () {return NULL;}
int	pb_check (io_stream) FILE *io_stream; {return 0;}
void	pb_empty (io_stream) FILE *io_stream; {}
char	*pb_name () {return _cp;}
int	pb_puts (buf, io_stream) char *buf; FILE *io_stream; {return 0;}
int	pb_weof (io_stream) FILE *io_stream; {return 0;}
int	pb_seek (io_stream) FILE *io_stream; {return 0;}
long	_adf_skptxt (ptr, io_stream) char *ptr; FILE *io_stream; {return 0L;}
char	*pb_gets (buf, n, io_stream) char *buf; int n; FILE *io_stream; {return _cp;}
void	pb_gbuf (buf, bufsize, store_fn, io_stream) char *buf; int bufsize, (*store_fn) (); FILE *io_stream; {}
void	_adf_rtbl (twidth) int twidth; {}
void	_adf_rtxt (ptr) char *ptr; {}
void	_adf_rnum (ptr) char *ptr; {}
long	_adf_rsnm (ptr) char *ptr; {return 0L;}
int	_chkpbfull () {return 0;}
int	adf_gttok (ptr, kw_tbl) char *ptr; struct s_kwtbl *kw_tbl; {return 0;}
char	*adf_gtwrd (s_ptr, d_ptr) char *s_ptr, *d_ptr; {return _cp;}
char	*adf_gtxcd (s_ptr, d_ptr) char *s_ptr, *d_ptr; {return _cp;}
int	wind (type,height,width,flags,pfont) int type,height,width; short flags; char *pfont[]; {return 0;}
void do_w_new(height,width)
int height,width; {}
