/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/ex_data.c	1.13"

#include "ex.h"
#include "ex_tty.h"

/*
 * Initialization of option values.
 * The option #defines in ex_vars.h are made
 * from this file by the script makeoptions.
 *
 * These initializations are done char by char instead of as strings
 * to confuse xstr so it will leave them alone.
 */
unsigned char	direct[ONMSZ] =
	{'/', 't', 'm', 'p'}; 
unsigned char	paragraphs[ONMSZ] = {
	'I', 'P', 'L', 'P', 'P', 'P', 'Q', 'P',		/* -ms macros */
	'P', ' ', 'L', 'I',				/* -mm macros */
	'p', 'p', 'l', 'p', 'i', 'p', 'n', 'p',		/* -me macros */
	'b', 'p'					/* bare nroff */
};
unsigned char	sections[ONMSZ] = {
	'N', 'H', 'S', 'H',				/* -ms macros */
	'H', ' ', 'H', 'U',				/* -mm macros */
	'u', 'h', 's', 'h', '+', 'c'			/* -me macros */
};
unsigned char	shell[ONMSZ] =
	{ '/', 'b', 'i', 'n', '/', 's', 'h' };
unsigned char	tags[ONMSZ] = {
	't', 'a', 'g', 's', ' ',
	'/', 'u', 's', 'r', '/', 'l', 'i', 'b', '/', 't', 'a', 'g', 's'
};
unsigned char termtype[ONMSZ];

struct	option options[vi_NOPTS + 1] = {
	(unsigned char *)"autoindent",	(unsigned char *)"ai",	ONOFF,		0,	0,	0,
	(unsigned char *)"autoprint",	(unsigned char *)"ap",	ONOFF,		1,	1,	0,
	(unsigned char *)"autowrite",	(unsigned char *)"aw",	ONOFF,		0,	0,	0,
	(unsigned char *)"beautify",	(unsigned char *)"bf",	ONOFF,		0,	0,	0,
	(unsigned char *)"directory",	(unsigned char *)"dir",	STRING,		0,	0,	direct,
	(unsigned char *)"edcompatible",	(unsigned char *)"ed",	ONOFF,		0,	0,	0,
	(unsigned char *)"errorbells",	(unsigned char *)"eb",	ONOFF,		0,	0,	0,
	(unsigned char *)"exrc",	(unsigned char *)"ex",	ONOFF,		0,	0,	0,
	(unsigned char *)"flash",	(unsigned char *)"fl",	ONOFF,		1,	1,	0,
	(unsigned char *)"hardtabs",	(unsigned char *)"ht",	NUMERIC,	8,	8,	0,
	(unsigned char *)"ignorecase",	(unsigned char *)"ic",	ONOFF,		0,	0,	0,
	(unsigned char *)"lisp",		0,	ONOFF,		0,	0,	0,
	(unsigned char *)"list",		0,	ONOFF,		0,	0,	0,
	(unsigned char *)"magic",	0,	ONOFF,		1,	1,	0,
	(unsigned char *)"mesg",		0,	ONOFF,		1,	1,	0,
	(unsigned char *)"modelines",	(unsigned char *)"ml",	ONOFF,		0,	0,	0,
	(unsigned char *)"number",	(unsigned char *)"nu",	ONOFF,		0,	0,	0,
	(unsigned char *)"novice",	0,	ONOFF,		0,	0,	0,
	(unsigned char *)"optimize",	(unsigned char *)"opt",	ONOFF,		0,	0,	0,
	(unsigned char *)"paragraphs",	(unsigned char *)"para",	STRING,		0,	0,	paragraphs,
	(unsigned char *)"prompt",	0,	ONOFF,		1,	1,	0,
	(unsigned char *)"readonly",	(unsigned char *)"ro",	ONOFF,		0,	0,	0,
	(unsigned char *)"redraw",	0,	ONOFF,		0,	0,	0,
	(unsigned char *)"remap",	0,	ONOFF,		1,	1,	0,
	(unsigned char *)"report",	0,	NUMERIC,	5,	5,	0,
	(unsigned char *)"scroll",	(unsigned char *)"scr",	NUMERIC,	12,	12,	0,
	(unsigned char *)"sections",	(unsigned char *)"sect",	STRING,		0,	0,	sections,
	(unsigned char *)"shell",	(unsigned char *)"sh",	STRING,		0,	0,	shell,
	(unsigned char *)"shiftwidth",	(unsigned char *)"sw",	NUMERIC,	TABS,	TABS,	0,
	(unsigned char *)"showmatch",	(unsigned char *)"sm",	ONOFF,		0,	0,	0,
	(unsigned char *)"showmode",	(unsigned char *)"smd",	ONOFF,		0,	0,	0,
	(unsigned char *)"slowopen",	(unsigned char *)"slow",	ONOFF,		0,	0,	0,
	(unsigned char *)"tabstop",	(unsigned char *)"ts",	NUMERIC,	TABS,	TABS,	0,
	(unsigned char *)"taglength",	(unsigned char *)"tl",	NUMERIC,	0,	0,	0,
	(unsigned char *)"tags",		(unsigned char *)"tag",	STRING,		0,	0,	tags,
	(unsigned char *)"term",		0,	OTERM,		0,	0,	termtype,
	(unsigned char *)"terse",	0,	ONOFF,		0,	0,	0,
	(unsigned char *)"timeout",	(unsigned char *)"to",	ONOFF,		1,	1,	0,
	(unsigned char *)"ttytype",	(unsigned char *)"tty",	OTERM,		0,	0,	termtype,
	(unsigned char *)"warn",		0,	ONOFF,		1,	1,	0,
	(unsigned char *)"window",	(unsigned char *)"wi",	NUMERIC,	23,	23,	0,
	(unsigned char *)"wrapscan",	(unsigned char *)"ws",	ONOFF,		1,	1,	0,
	(unsigned char *)"wrapmargin",	(unsigned char *)"wm",	NUMERIC,	0,	0,	0,
	(unsigned char *)"writeany",	(unsigned char *)"wa",	ONOFF,		0,	0,	0,
	0,		0,	0,		0,	0,	0,
};
