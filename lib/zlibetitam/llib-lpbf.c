/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:llib-lpbf.c	1.1"
/*LINTLIBRARY*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "pbf.h"

char *_cp;

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
