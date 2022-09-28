/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:include/filters.h	1.10.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** The disk copy of the filter table:
 **/

/*
 * There are 9 fields in the filter table (the first one is ignored).
 */
#define	FL_MAX	9
# define FL_IGN		0
# define FL_PTYPS	1
# define FL_PRTRS	2
# define FL_ITYPS	3
# define FL_NAME	4
# define FL_OTYPS	5
# define FL_TYPE	6
# define FL_CMD		7
# define FL_TMPS	8

/*
 * Various strings.
 */
#define FL_SEP		":"
#define FL_END		"\n"
#define	FL_FAST		"fast"
#define	FL_SLOW		"slow"

/**
 ** The internal copy of a filter as seen by the rest of the world:
 **/

typedef enum FILTERTYPE {
	fl_none,
	fl_fast,
	fl_slow,
	fl_both
}			FILTERTYPE;

/*
 * A (char **) list is an array of string pointers (char *) with
 * a null pointer after the last item.
 */
typedef struct FILTER {
	char *		name;		/* name of filter (redundant) */
	char *		command;	/* shell command (full path) */
	FILTERTYPE	type;		/* type of filter (fast/slow) */
	char **		printer_types;	/* list of valid printer types */
	char **		printers;	/* list of valid printers */
	char **		input_types;	/* list of valid input types */
	char **		output_types;	/* list of valid output types */
	char **		templates;	/* list of option templates */
}			FILTER;

/**
 ** The internal copy of a filter as seen by the filter routines:
 **/

/*
 * To speed up processing the filter table, FL_MAX_GUESS slots
 * will be preallocated for the internal copy. If filter tables
 * are expected to be substantially larger than this, bump it up.
 */
#define FL_MAX_GUESS	10

typedef struct TYPE {
	char *			name;
	unsigned short		info;	/* 1 iff "name" is in Terminfo */
}			TYPE;

#define	PATT_STAR	"*"

typedef struct TEMPLATE {
	char *			keyword;
	char *			pattern;
	char *			re;
	char *			result;
	int			nbra;
}			TEMPLATE;

/*
 * A (TYPE *) list is an array of content-types (TYPE) with a null
 * "name" element. A (TEMPLATE *) list is an array of templates (TEMPLATE)
 * with a null "keyword" element.
 */
typedef struct _FILTER {
	struct _FILTER *	next;		/* for linking several */
	char *			name;
	char *			command;
	char **			printers;
	TYPE *			printer_types;
	TYPE *			input_types;	/* all possible choices */
	TYPE *			output_types;	/* all possible choices */
	TYPE *			inputp;		/* the one to be used */
	TYPE *			outputp;	/* the one to be used */
	TEMPLATE *		templates;
	FILTERTYPE		type;
	unsigned char		mark,
				level;
}			_FILTER;

#define	FL_CLEAR	0x00
#define	FL_SKIP		0x01
#define	FL_LEFT		0x02
#define	FL_RIGHT	0x04

#define PARM_INPUT	"INPUT"
#define PARM_OUTPUT	"OUTPUT"
#define PARM_TERM	"TERM"
#define PARM_PRINTER	"PRINTER"

#define NPARM_SPEC	8
# define PARM_CPI	"CPI"
# define PARM_LPI	"LPI"
# define PARM_LENGTH	"LENGTH"
# define PARM_WIDTH	"WIDTH"
# define PARM_PAGES	"PAGES"
# define PARM_CHARSET	"CHARSET"
# define PARM_FORM	"FORM"
# define PARM_COPIES	"COPIES"

#define PARM_MODES	"MODES"

#define FPARM_CPI	0x0001
#define FPARM_LPI	0x0002
#define FPARM_LENGTH	0x0004
#define FPARM_WIDTH	0x0008
#define FPARM_PAGES	0x0010
#define FPARM_CHARSET	0x0020
#define FPARM_FORM	0x0040
#define FPARM_COPIES	0x0080
#define FPARM_MODES	0x0100

/**
 ** Various routines.
 **/

/*
 * Null terminated list (filters[i].name == NULL).
 */
extern _FILTER		*filters;

extern size_t		nfilters;

#if	defined(__STDC__)

FILTER *	getfilter ( char * );

_FILTER *	search_filter ( char * );

FILTERTYPE	insfilter ( char ** , char * , char * , char * , char * , char ** , unsigned short * );
FILTERTYPE	s_to_filtertype ( char * );

TEMPLATE	s_to_template ( char * );

TEMPLATE *	sl_to_templatel ( char ** );

TYPE		s_to_type ( char * );

TYPE *		sl_to_typel ( char ** );

char *		template_to_s ( TEMPLATE );
char *		type_to_s ( TYPE );

char **		templatel_to_sl ( TEMPLATE * );
char **		typel_to_sl ( TYPE * );

FILE *		open_filtertable ( char * , char * );

int		get_and_load ( void );
int		putfilter ( char * , FILTER * );
int		delfilter ( char * );
int		loadfilters ( char * );

void		freetempl ( TEMPLATE * );
void		freefilter ( FILTER * );
void		free_filter ( _FILTER * );
void		trash_filters ( void );
void		close_filtertable ( FILE * );

#else

extern FILTER		*getfilter();

extern _FILTER		*search_filter();

extern FILTERTYPE	insfilter(),
			s_to_filtertype();

extern TYPE		s_to_type(),
			*sl_to_typel();

extern TEMPLATE		s_to_template(),
			*sl_to_templatel();

#if	defined(BUFSIZ)
extern FILE		*open_filtertable();
#endif

extern char		**typel_to_sl(),
			**templatel_to_sl(),
			*getfilterfile();

extern int		putfilter(),
			delfilter(),
			loadfilters(),
			get_and_load();

extern void		freefilter(),
			free_filter(),
			freetempl(),
			trash_filters(),
			close_filtertable();

#endif
