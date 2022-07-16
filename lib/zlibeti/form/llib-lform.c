/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/llib-lform.c	1.6"

/*LINTLIBRARY*/

#include "form.h"

	/***********************
	*  FIELDTYPE routines  *
	***********************/

FIELDTYPE * new_fieldtype (fcheck, ccheck)
PTF_int fcheck;
PTF_int ccheck;
{ return (FIELDTYPE *) 0; }

FIELDTYPE * link_fieldtype (left, right)
FIELDTYPE * left;
FIELDTYPE * right;
{ return (FIELDTYPE *) 0; }

int free_fieldtype (t)
FIELDTYPE *	t;
{ return E_SYSTEM_ERROR; }

int set_fieldtype_arg (t, makearg, copyarg, freearg)
FIELDTYPE * t;
PTF_charP makearg;
PTF_charP copyarg;
PTF_void freearg;
{ return E_SYSTEM_ERROR; }

int set_fieldtype_choice (t, next, prev)
FIELDTYPE * t;
PTF_int next;
PTF_int prev;
{ return E_SYSTEM_ERROR; }

	/*******************
	*  FIELD routines  *
	*******************/

FIELD * new_field (rows, cols, frow, fcol, nrow, nbuf)
int rows;
int cols;
int frow;
int fcol;
int nrow;
int nbuf;
{ return (FIELD *) 0; }

FIELD * dup_field (field, frow, fcol)
FIELD * field;
int frow;
int fcol;
{ return (FIELD *) 0; }

FIELD * link_field (field, frow, fcol)
FIELD * field;
int frow;
int fcol;
{ return (FIELD *) 0; }

int free_field (f)
FIELD *	 f;
{ return E_SYSTEM_ERROR; }

int field_info (f, rows, cols, frow, fcol, nrow, nbuf)
FIELD * f;
int * rows;
int * cols;
int * frow;
int * fcol;
int * nrow;
int * nbuf;
{ return E_SYSTEM_ERROR; }

int dynamic_field_info (f, drows, dcols, max)
FIELD *f;
int   *drows;
int   *dcols;
int   *max;
{ return E_SYSTEM_ERROR; }

int set_max_field(f, max)
FIELD *f;
int   *max;
{ return E_SYSTEM_ERROR; }

int move_field (f, frow, fcol)
FIELD * f;
int frow;
int fcol;
{ return E_SYSTEM_ERROR; }

/*VARARGS*/
int set_field_type ()
{ return E_SYSTEM_ERROR; }

FIELDTYPE * field_type (f)
FIELD * f;
{ return (FIELDTYPE *) 0; }

char * field_arg (f)
FIELD * f;
{ return (char *) 0; }

int set_new_page (f, flag)
FIELD * f;
int flag;
{ return E_SYSTEM_ERROR; }

int new_page (f)
FIELD * f;
{ return FALSE; }

int set_field_just (f, just)
FIELD * f;
int just;
{ return E_SYSTEM_ERROR; }

int field_just (f)
FIELD * f;
{ return NO_JUSTIFICATION; }

int set_field_fore (f, fore)
FIELD * f;
chtype fore;
{ return E_SYSTEM_ERROR; }

chtype field_fore (f)
FIELD * f;
{ return A_NORMAL; }

int set_field_back (f, back)
FIELD * f;
chtype back;
{ return E_SYSTEM_ERROR; }

chtype field_back (f)
FIELD * f;
{ return A_NORMAL; }

int set_field_pad (f, pad)
FIELD * f;
int pad;
{ return E_SYSTEM_ERROR; }

int field_pad (f)
FIELD * f;
{ return ' '; }

int set_field_buffer (f, n, v)
FIELD * f;
int n;
char * v;
{ return E_SYSTEM_ERROR; }

char * field_buffer (f, n)
FIELD * f;
int n;
{ return (char *) 0; }

int set_field_status (f, status)
FIELD * f;
int status;
{ return E_SYSTEM_ERROR; }

int field_status (f)
FIELD * f;
{ return FALSE; }

int set_field_userptr (f, userptr)
FIELD * f;
char * userptr;
{ return E_SYSTEM_ERROR; }

char * field_userptr (f)
FIELD * f;
{ return (char *) 0; }

int set_field_opts (f, opts)
FIELD * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

OPTIONS field_opts (f)
FIELD * f;
{ return (OPTIONS) 0; }

int field_opts_on (f, opts)
FIELD * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

int field_opts_off (f, opts)
FIELD * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

	/******************
	*  FORM routines  *
	******************/

FORM * new_form (field)
FIELD ** field;
{ return (FORM *) 0; }

int free_form (f)
FORM * f;
{ return E_SYSTEM_ERROR; }

int set_form_fields (f, fields)
FORM * f;
FIELD ** fields;
{ return E_SYSTEM_ERROR; }

FIELD ** form_fields (f)
FORM * f;
{ return (FIELD **) 0; }

int field_count (f)
FORM * f;
{ return -1; }

int set_form_win (f, window)
FORM * f;
WINDOW * window;
{ return E_SYSTEM_ERROR; }

WINDOW * form_win (f)
FORM * f;
{ return (WINDOW *) 0; }

int set_form_sub (f, window)
FORM * f;
WINDOW * window;
{ return E_SYSTEM_ERROR; }

WINDOW * form_sub (f)
FORM * f;
{ return (WINDOW *) 0; }

int set_current_field (f, c)
FORM * f;
FIELD * c;
{ return E_SYSTEM_ERROR; }

FIELD * current_field (f)
FORM * f;
{ return (FIELD *) 0; }

int field_index (f)
FIELD * f;
{ return -1; }

int set_form_page (f, page)
FORM * f;
int page;
{ return E_SYSTEM_ERROR; }

int form_page (f)
FORM * f;
{ return -1; }

int scale_form (f, rows, cols)
FORM * f;
int * rows;
int * cols;
{ return E_SYSTEM_ERROR; }

int set_form_init (f, func)
FORM * f;
PTF_void func;
{ return E_SYSTEM_ERROR; }

PTF_void form_init (f)
FORM * f;
{ return (PTF_void) 0; }

int set_form_term (f, func)
FORM * f;
PTF_void func;
{ return E_SYSTEM_ERROR; }

PTF_void form_term (f)
FORM * f;
{ return (PTF_void) 0; }

int set_field_init (f, func)
FORM * f;
PTF_void func;
{ return E_SYSTEM_ERROR; }

PTF_void field_init (f)
FORM * f;
{ return (PTF_void) 0; }

int set_field_term (f, func)
FORM * f;
PTF_void func;
{ return E_SYSTEM_ERROR; }

PTF_void field_term (f)
FORM * f;
{ return (PTF_void) 0; }

int post_form (f)
FORM * f;
{ return E_SYSTEM_ERROR; }

int unpost_form (f)
FORM * f;
{ return E_SYSTEM_ERROR; }

int pos_form_cursor (f)
FORM * f;
{ return E_SYSTEM_ERROR; }

int form_driver (f, c)
FORM * f;
int c;
{ return E_SYSTEM_ERROR; }

int set_form_userptr (f, userptr)
FORM * f;
char * userptr;
{ return E_SYSTEM_ERROR; }

char * form_userptr (f)
FORM * f;
{ return (char *) 0; }

int set_form_opts (f, opts)
FORM * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

OPTIONS form_opts (f)
FORM * f;
{ return (OPTIONS) 0; }

int form_opts_on (f, opts)
FORM * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

int form_opts_off (f, opts)
FORM * f;
OPTIONS opts;
{ return E_SYSTEM_ERROR; }

int data_ahead( f )
FORM *f;
{ return FALSE; }

int data_behind( f )
FORM *f;
{ return FALSE; }
