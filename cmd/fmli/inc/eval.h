/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)fmli:inc/eval.h	1.3"

#define EV_TOKEN	0x20
#define EV_GROUP	0x40
#define EV_USE_STRING	0x80
#define EV_USE_FP	0x100
#define EV_READONLY	0x200
#define EV_APPEND	0x400
#ifndef EV_SQUIG                /* must match EV_SQUIG in oh/fm_mn_par.h   */
#define EV_SQUIG	0x8000	/* set when {} are special in a descriptor */
#endif                          /* careful.. flag is flipped in eval()     */

typedef struct io_struct {
	int	flags;
	union {
		FILE	*fp;
		struct {
			char	*val;
			int	count;
			int	pos;
		} str;
	} mu;
	struct io_struct	*next;
} IOSTRUCT;

int eval();
int io_close();
IOSTRUCT *io_open();
char *io_string();
char *io_ret_string();

/* eval TOKENS (see spchars in eval.c) */
#define ET_EOF		0
#define ET_WORD		1
#define ET_DQUOTE	2
#define ET_SQUOTE	3
#define ET_BSLASH	4
#define ET_BQUOTE	5
#define ET_DOLLAR	6
#define ET_NEWLINE	7
#define ET_SPACE	8
#define ET_TAB		9
#define ET_OSQUIG	10
#define ET_CSQUIG	11
#define ET_PIPE		12
#define ET_AMPERSAND	13
#define ET_SEMI		14
#define ET_LTHAN	15
#define ET_GTHAN	16
#define ET_TWO		17

#define DOUBLE		32	/* must be a power of 2 > largest TOKEN above */
