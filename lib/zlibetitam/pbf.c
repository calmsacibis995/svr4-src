/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:pbf.c	1.1"
/*
 *  pbf.c - V1.1
 *
 *	Paste buffer utilities
 */
 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "pbf.h"

#define  TRUE	1
#define  FALSE	0

extern	char	*ttyname ();

/*
 *  Shared variables for paste buffer procedures
 */

static	char	_pb_first;		/* First time flag */
static	char	*_pbuf;			/* Paste buffer pointer */
static	int	_pbcount;		/* Buffer count */
static	int	_pbsize;		/* Buffer size */
static	int	(*_pbstore) ();		/* Buffer store function */
static	FILE	*_pbfile;		/* Paste buffer IO stream ptr */

/*
 *  ADF interpretation tables
 */

#define	atk_adf		 1		/* ADF keyword */
#define	atk_app		 2		/* APPLICATION keyword */
#define	atk_ver		 3		/* VERSION keyword */
#define	atk_eof		 4		/* End of file */
#define	atk_txt		 5		/* Text */
#define	atk_int		 6		/* Integer */
#define	atk_float	 7		/* Floating point number */
#define	atk_time	 8		/* Time */
#define	atk_schema	 9		/* Database schema */
#define	atk_tuple	10		/* Database record */
#define	atk_tbldef	11		/* Table layout definition */
#define	atk_table	12		/* Tabular data */
#define	atk_page	13		/* Page layout */
#define	atk_para	14		/* Paragraph formatting */
#define	atk_end		15		/* End block */

#define ttk_ind		 1		/* Indent character */
#define ttk_cen		 2		/* Center character */
#define ttk_rb		 3		/* Required backspace */
#define ttk_hs		 4		/* Hard space */
#define ttk_oh		 5		/* Optional hyphen */
#define ttk_hh		 6		/* Hard hyphen */
#define ttk_hi		 7		/* Hanging indent */
#define ttk_bb		 8		/* Begin block */
#define ttk_eb		 9		/* End block */
#define ttk_pn		10		/* Page number */
#define ttk_eop		11		/* End of page */
#define ttk_hp		12		/* Hard end of page */
#define ttk_bf		13		/* Begin fieldname */
#define ttk_ef		14		/* End field name */
#define ttk_rs		15		/* Record separator */
#define ttk_eot		16		/* End of text */
#define ttk_ul		17		/* Underline */
#define ttk_wu		18		/* Word underline */
#define ttk_du		19		/* Double underline */
#define ttk_us		20		/* Underline stop */
#define ttk_bl		21		/* Bold */
#define ttk_bs		22		/* Bold stop */
#define ttk_mi		23		/* Mark insert */
#define ttk_mis		24		/* Mark insert stop */
#define ttk_md		25		/* Mark delete */
#define ttk_mds		26		/* Mark delete stop */
#define ttk_sup		27		/* Superscript */
#define ttk_sps		28		/* Superscript stop */
#define ttk_sub		29		/* Subscript */
#define ttk_sbs		30		/* Subscript stop */

#define ntk_ju		 1		/* Justification mode */
#define ntk_fo		 2		/* Display format */

static struct s_kwtbl _pbadf_kw [] = {	/* Top level keywords */
  "TEXT",		atk_txt,
  "PAGE",		atk_page,
  "PARAGRAPH",		atk_para,
  "INT",		atk_int,
  "FLOAT",		atk_float,
  "TIME",		atk_time,
  "SCHEMA",		atk_schema,
  "TUPLE",		atk_tuple,
  "TABLEDEF",		atk_tbldef,
  "TABLE",		atk_table,
  "ADF",		atk_adf,
  "VERSION",		atk_ver,
  "APPLICATION",	atk_app,
  "EOF",		atk_eof,
  "}",			atk_end,
  0,			atk_err
};

static	struct	s_kwtbl	_pbtxt_kw [] = {	/* Text codes */
  "IND",	ttk_ind,
  "CEN",	ttk_cen,
  "RB",		ttk_rb,
  "HS",		ttk_hs,
  "OH",		ttk_oh,
  "HH",		ttk_hh,
  "HI",		ttk_hi,
  "BB",		ttk_bb,
  "EB",		ttk_eb,
  "PN",		ttk_pn,
  "EOP",	ttk_eop,
  "HP",		ttk_hp,
  "BF",		ttk_bf,
  "EF",		ttk_ef,
  "RS",		ttk_rs,
  "EOT",	ttk_eot,
  "UL",		ttk_ul,
  "WU",		ttk_wu,
  "DU",		ttk_du,
  "US",		ttk_us,
  "BL",		ttk_bl,
  "BS",		ttk_bs,
  "MI",		ttk_mi,
  "MIS",	ttk_mis,
  "MD",		ttk_md,
  "MDS",	ttk_mds,
  "SUP",	ttk_sup,
  "SPS",	ttk_sps,
  "SUB",	ttk_sub,
  "SBS",	ttk_sbs,
  0,		atk_err
};

static struct s_kwtbl _pbnum_kw [] = {
  "JU",		ntk_ju,
  "FORMAT",	ntk_fo,
  0,		atk_err
};

static struct s_kwtbl _pbtbl_kw [] = {	/* Keywords in table or tuple */
  "TEXT",	atk_txt,
  "}",		atk_end,
  0,		atk_err
};


/*
 *  pb_open  -  Open/create the paste buffer file
 *
 *	Returns NULL pointer on failure
 */

FILE *
pb_open ()
{
  char	*ptr;
  int	desc;
  char	*pb_name ();

  ptr = pb_name ();
  if (ptr == NULL) {
    return (NULL);
  }
  if ((desc = open (ptr, O_RDWR + O_CREAT + O_EXCL, 0666)) == -1) {
    if ((desc = open (ptr, O_RDWR)) == -1) {
      return (NULL);		/* Can't open file */
    }
  }
  else {
    (void)chmod (ptr, 0666);		/* New file, fix mode */
  }
  _pb_first = TRUE;
  return (fdopen (desc, "r+"));
}

/*
 *  pb_check  -  Check if the paste buffer contains anything
 *
 *	Returns TRUE if paste buffer non empty
 *		FALSE if empty
 */

pb_check (io_stream)
FILE	*io_stream;
{
  char	buf [6];
  int	n;

  n = fread (buf, sizeof (buf), sizeof (char), io_stream);
  rewind (io_stream);
  if ((strncmp (buf, "EMPTY\n", sizeof (buf)) == 0) || (n == 0)) {
    return (FALSE);
  }
  else {
    return (TRUE);
  }
}

/*
 *  pb_empty  -  Clear out the paste buffer and close it
 */

pb_empty (io_stream)
FILE	*io_stream;
{
  char	*pb_name ();

  rewind (io_stream);
  (void)fputs ("EMPTY\n", io_stream);
  (void)fclose (io_stream);
}

/*
 *  pb_name  -  Get name for paste buffer file
 *
 *	Returns null pointer if standard out is not a tty device
 */

char *pb_name ()
{
  static	char	name [25];
  register char	*ptr;

  (void)strcpy (name, "/usr/tmp/paste.");
  ptr = ttyname (1);
  if (ptr == NULL) {
    return (NULL);
  }
  (void)strcat (name, ptr + 5);
  return (name);
}


/*
 *  pb_puts  -  Output string to paste buffer in ADF format
 *
 *	Returns EOF on error
 */

pb_puts (buf, io_stream)
register char	*buf;
register FILE	*io_stream;
{
  register	col;

  if (_pb_first) {
    if (fputs ("ADF\nVERSION 1.0\n", io_stream) == EOF) {
      return (EOF);
    }
    _pb_first = FALSE;
  }
  if (fputs ("TEXT\n", io_stream) == EOF) {
    return (EOF);
  }

  col = 0;
  while (*buf) {
    (void)fputc (*buf++, io_stream);
    if (col++ > 70) {
      (void)fputs ("\\\n", io_stream);
      col = 0;
    }
  }

  if (fputs ("\\EOT\\\n", io_stream) == EOF) {
    return (EOF);
  }
  return (0);
}

/*
 *  pb_weof  -  Output EOF string to paste buffer and close file
 */

pb_weof (io_stream)
register FILE	*io_stream;
{
  (void)fputs ("EOF\n", io_stream);
  (void)fclose (io_stream);
}


/*
 *  pb_seek  -  Seek to end of paste buffer file, set for append
 */

pb_seek (io_stream)
FILE	*io_stream;
{
  char	looping;
  char	rbuf [80];
  char	wrd_buf [80];
  long	disp;
  char	*ptr;

  rewind (io_stream);
  disp = 0;
  looping = TRUE;
  while (looping) {
    if (fgets (rbuf, 80, io_stream) == NULL) {
      break;
    }
    ptr = adf_gtwrd (rbuf, wrd_buf);
    switch (adf_gttok (wrd_buf, _pbadf_kw)) {
      case atk_eof: {		/* End of file */
        looping = FALSE;
        break;
      }

      case atk_txt: {		/* Text */
        disp += strlen (rbuf);
        disp += _adf_skptxt (ptr, io_stream);
        break;
      }

      default: {
        disp += strlen (rbuf);
        break;
      }
    }
  }
  (void)fseek (io_stream, disp, 0);
}

/*
 *  _adf_skptxt  -  Skip to end of ADF text item
 */

_adf_skptxt (ptr, io_stream)
register char	*ptr;
FILE	*io_stream;
{
  char	rbuf [80];
  char	wbuf [80];
  long	len;
  char	eot;

  len = 0;
  while ((*ptr == 0x20) || (*ptr == 0x09) || (*ptr == 0x0a)) {
    ptr++;			/* Skip to start of text */
  }
  if (*ptr == 0) {
    if (fgets (rbuf, 80, io_stream) == NULL) {
      return (len);
    }
    len += strlen (rbuf);
  }
  eot = FALSE;
  while (!eot) {
    if (*ptr == 0) {
      if ((ptr = fgets (rbuf, 80, io_stream)) == NULL) {
        break;
      }
      len += strlen (rbuf);
    }
    else if (*ptr == '\\') {
      ptr++;
      if (*ptr == '\\') {
        ptr++;
      }
      else if (*ptr == 0x0a) {
        ptr++;
      }
      else {
        ptr = adf_gtxcd (ptr, wbuf);
        switch (adf_gttok (wbuf, _pbtxt_kw)) {
          case ttk_eot: {	/* End of text */
            eot = TRUE;
            break;
	  }

          default: {
            break;
	  }
	}
      }
    }
    else {
      ptr++;
    }
  }

  return (len);
}


/*
 *  pb_gets  -  Read paste buffer file, converting to text
 */

char *pb_gets (buf, n, io_stream)
char	*buf;
register int	n;
FILE	*io_stream;
{
  static	char	pb_buf [512];
  static	char	*pb_ptr;
  register char	*sptr;
  register char	*dptr;

  if (_pb_first) {		/* On first call, fill buffer */
    pb_gbuf (pb_buf, 512, NULL, io_stream);
    pb_ptr = pb_buf;
  }

  sptr = pb_ptr;
  dptr = buf;
  while (--n >= 0) {
    if (*sptr == 0) {
      break;
    }
    else if (*sptr == 0x0a) {
      *dptr++ = *sptr++;
      break;
    }
    else {
      *dptr++ = *sptr++;
    }
  }
  *dptr = 0;
  pb_ptr = sptr;
  return (dptr == buf ? NULL : buf);
}


/*
 *  pb_gbuf  -  Read paste buffer file to buffer
 */

pb_gbuf (buf, bufsize, store_fn, io_stream)
char	*buf;
int	bufsize;
int	(*store_fn) ();
FILE	*io_stream;
{
  char	looping;
  char	rbuf [80];
  char	wrd_buf [80];
  char	*ptr;
  int	twidth;			/* Table dimensions */
  int	theight;

  _pbuf = buf;			/* Init shared variables */
  _pbcount = 0;
  _pbsize = bufsize;
  _pbstore = store_fn;
  _pbfile = io_stream;

  looping = TRUE;
  while (looping) {
    if ((ptr = fgets (rbuf, 80, _pbfile)) == NULL) {
      break;
    }
    ptr = adf_gtwrd (rbuf, wrd_buf);
    switch (adf_gttok (wrd_buf, _pbadf_kw)) {
      case atk_err:		/* Error */
      case atk_null:		/* Empty line */
      case atk_adf:		/* ADF keyword */
      case atk_app:		/* APPLICATION keyword */
      case atk_ver: {		/* VERSION keyword */
        break;
      }

      case atk_eof: {		/* End of file */
        looping = FALSE;
        break;
      }

      case atk_txt: {		/* Text */
        _adf_rtxt (ptr);
        break;
      }

      case atk_int:		/* Integer */
      case atk_float:		/* Floating point number */
      case atk_time: {		/* Time */
        _adf_rnum (ptr);
        break;
      }

      case atk_schema: {	/* Database schema */
        break;
      }

      case atk_tuple: {		/* Database record */
        _adf_rtbl (40);
        break;
      }

      case atk_tbldef: {	/* Table layout definition */
        (void)sscanf (ptr, " %d %d", &twidth, &theight);
        break;
      }

      case atk_table: {		/* Tabular data */
        _adf_rtbl (twidth);
        break;
      }

      case atk_page:		/* Page layout */
      case atk_para:		/* Paragraph formatting */
      case atk_end: {		/* End block */
        break;
      }
    }
    if (_chkpbfull ()) {
      break;
    }
  }
  if (_pbstore == NULL) {
    if (_pbcount < _pbsize) {
      _pbuf [_pbcount] = 0;
    }
    else {
      _pbuf [_pbsize - 1] = 0;
    }
  }
  else if (_pbcount > 0) {
    (*_pbstore) (_pbuf, _pbcount);
  }
}


/*
 *  _adf_rtbl  -  Read table from ADF file and add to buffer
 */

_adf_rtbl (twidth)
int	twidth;
{
  int	width;
  register char	*ptr;
  char	rbuf [80];
  char	wrd_buf [80];
  char	looping;

  width = 0;
  looping = TRUE;
  while (looping) {
    if ((ptr = fgets (rbuf, 80, _pbfile)) == NULL) {
      break;		/* End of file */
    }
    ptr = adf_gtwrd (rbuf, wrd_buf);
    switch (adf_gttok (wrd_buf, _pbtbl_kw)) {
      case atk_err: {		/* Error - Not a keyword, */
				/*  Check if it is a number */
        while (wrd_buf [0] != 0) {
          if ((_adf_rsnm (wrd_buf)) == 0) {
            break;
	  }
          if (_chkpbfull ()) {
            break;
	  }
          _pbuf [_pbcount] = 0x09;
          if (++width >= twidth) {
            width = 0;
            _pbuf [_pbcount] = 0x0a;
	  }
          _pbcount++;
          if (_chkpbfull ()) {
            break;
	  }
          ptr = adf_gtwrd (ptr, wrd_buf);
	}
        break;
      }

      case atk_null: {		/* Empty line */
        break;
      }

      case atk_txt: {		/* Text */
        _adf_rtxt (ptr);
        if (_chkpbfull ()) {
          break;
	}
        _pbuf [_pbcount] = 0x09;
        if (++width >= twidth) {
          width = 0;
          _pbuf [_pbcount] = 0x0a;
	}
        _pbcount++;
        break;
      }

      case atk_end: {		/* End block */
        if (width > 0) {
          _pbuf [_pbcount++] = 0x0a;
	}
        looping = FALSE;
        break;
      }
    }
    if (_chkpbfull ()) {
      break;
    }
  }
}


/*
 *  _adf_rtxt  -  Read text data from ADF file and insert in document
 */

_adf_rtxt (ptr)
register char	*ptr;
{
  char	rbuf [80];
  char	wbuf [80];
  char	eot;

  while ((*ptr == 0x20) || (*ptr == 0x09) || (*ptr == 0x0a)) {
    ptr++;			/* Skip to start of text */
  }
  if (*ptr == 0) {
    if ((ptr = fgets (rbuf, 80, _pbfile)) == NULL) {
      return;
    }
  }
  eot = FALSE;
  while (!eot) {
    if (*ptr == 0) {
      if ((ptr = fgets (rbuf, 80, _pbfile)) == NULL) {
        break;
      }
    }
    else if (*ptr == '\\') {
      ptr++;
      if (*ptr == '\\') {
        _pbuf [_pbcount++] = *ptr++;
      }
      else if (*ptr == 0x0a) {
        ptr++;
      }
      else {
        ptr = adf_gtxcd (ptr, wbuf);
        switch (adf_gttok (wbuf, _pbtxt_kw)) {
          case atk_err:	/* Error */
          case atk_null: {	/* Empty line */
            break;
	  }

          case ttk_ind: {	/* Indent character */
            _pbuf [_pbcount++] = 0x09;
            break;
	  }

          case ttk_cen:		/* Center character */
          case ttk_rb: {	/* Required backspace */
            break;
	  }

          case ttk_hs: {	/* Hard space */
            _pbuf [_pbcount++] = ' ';
            break;
	  }

          case ttk_oh: {	/* Optional hyphen */
            break;
	  }

          case ttk_hh: {	/* Hard hyphen */
            _pbuf [_pbcount++] = '-';
            break;
	  }

          case ttk_hi:		/* Hanging indent */
          case ttk_bb:		/* Begin block */
          case ttk_eb:		/* End block */
          case ttk_pn:		/* Page number */
          case ttk_eop:		/* End of page */
          case ttk_hp: {	/* Hard end of page */
            break;
	  }

          case ttk_bf: {	/* Begin fieldname */
            _pbuf [_pbcount++] = '(';
            break;
	  }

          case ttk_ef: {	/* End field name */
            _pbuf [_pbcount++] = ')';
            break;
	  }

          case ttk_rs: {	/* Record separator */
            _pbuf [_pbcount++] = '/';
            break;
	  }

          case ttk_eot:		/* End of text */
          case ttk_ul:		/* Underline */
          case ttk_wu:		/* Word underline */
          case ttk_du:		/* Double underline */
          case ttk_us:		/* Underline stop */
          case ttk_bl:		/* Bold */
          case ttk_bs:		/* Bold stop */
          case ttk_mi:		/* Mark insert */
          case ttk_mis:		/* Mark insert stop */
          case ttk_md:		/* Mark delete */
          case ttk_mds:		/* Mark delete stop */
          case ttk_sup:		/* Superscript */
          case ttk_sps:		/* Superscript stop */
          case ttk_sub:		/* Subscript */
          case ttk_sbs: {	/* Subscript stop */
            break;
	  }
	}
      }
    }
    else {
      _pbuf [_pbcount++] = *ptr++;
    }
    if (_chkpbfull ()) {
      break;
    }
  }
}


/*
 *  _adf_rnum  -  Read numeric data from ADF file and insert in document
 */

_adf_rnum (ptr)
register char	*ptr;
{
  char	rbuf [80];
  char	wbuf [80];

  while ((*ptr == 0x20) || (*ptr == 0x09) || (*ptr == 0x0a)) {
    ptr++;			/* Skip to start of number */
  }
  while (TRUE) {
    if (*ptr == 0) {
      if ((ptr = fgets (rbuf, 80, _pbfile)) == NULL) {
        break;
      }
      ptr = adf_gtwrd (rbuf, wbuf);
      switch (adf_gttok (wbuf, _pbnum_kw)) {
        case atk_err: {		/* Error */
          ptr = rbuf;
          break;
	}

        case atk_null:		/* Empty line */
        case ntk_ju:		/* Justification mode */
        case ntk_fo: {		/* Display format */
          *ptr = 0;
          break;
        }
      }
    }
    else {
      ptr = adf_gtwrd (ptr, wbuf);
      (void)_adf_rsnm (wbuf);
      if (*ptr == 0) {
        if (strcmp (wbuf, "\\") != 0) {
          break;
	}
      }
      _pbuf [_pbcount++] = ' ';
      if (_chkpbfull ()) {
        break;
      }
    }
  }
}

/*
 *  _adf_rsnm  -  Read a single number and insert in buffer
 */

_adf_rsnm (ptr)
register char	*ptr;		/* Pointer to buffer with number */
{
  long	len;
  register	n;

  len = 0;
  if (((*ptr >= '0') && (*ptr <= '9')) || (*ptr == '+') || (*ptr == '-')) {
    n = len = strlen (ptr);
    while (n-- > 0) {
      _pbuf [_pbcount++] = *ptr++;
      if (_chkpbfull ()) {
        break;
      }
    }
  }
  return (len);
}

/*
 *  _chkpbfull -  Return TRUE if paste buffer full and can't be
 *		emptied.
 */

_chkpbfull ()
{
  if (_pbcount >= _pbsize) {
    if (_pbstore == NULL) {
      return (TRUE);
    }
    if ((*_pbstore) (_pbuf, _pbcount) < 0) {/* Storage operation failed */
      _pbcount = 0;
      return (TRUE);
    }
    _pbcount = 0;
  }
  return (FALSE);
}


/*
 *  adf_gttok  -  Convert word to token
 */

adf_gttok (ptr, kw_tbl)
register char	*ptr;
register struct	s_kwtbl	*kw_tbl;
{
  if ((*ptr == 0) || (*ptr == '#')) {
    return (atk_null);
  }
  while (kw_tbl -> keyword) {	/* Search for match to end of table */
    if (strcmp (kw_tbl -> keyword, ptr) == 0) {
      break;		/* Found the keyword */
    }
    kw_tbl++;
  }
  return (kw_tbl -> token);
}

/*
 *  adf_gtwrd  -  Get next word from string and copy to buffer
 */

char *
adf_gtwrd (s_ptr, d_ptr)
register char	*s_ptr;		/* Pointer to string */
register char	*d_ptr;		/* Pointer to buffer */
{
  while ((*s_ptr == 0x20) || (*s_ptr == 0x09) || (*s_ptr == 0x0a)) {
    s_ptr++;			/* Skip to token */
  }
  while ((*s_ptr != 0) && (*s_ptr != 0x20) && (*s_ptr != '\\') &&
         (*s_ptr != 0x09) && (*s_ptr != 0x0a)) {
    *d_ptr++ = *s_ptr++;	/* Copy token to buffer */
  }
  *d_ptr++ = 0;			/* Terminate string */
  return (s_ptr);
}

/*
 *  adf_gtxcd  -  Get next text code from string and copy to buffer
 */

char *
adf_gtxcd (s_ptr, d_ptr)
register char	*s_ptr;		/* Pointer to string */
register char	*d_ptr;		/* Pointer to buffer */
{
  while ((*s_ptr != 0) && (*s_ptr != '\\')) {
    *d_ptr++ = *s_ptr++;	/* Copy token to buffer */
  }
  *d_ptr++ = 0;			/* Terminate string */
  if (*s_ptr == '\\') {
    s_ptr++;
  }
  return (s_ptr);
}
