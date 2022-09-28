/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cxref:cxref.c	1.4.3.1"
#include <stdio.h>
#include <sgs.h>
#include "cxref.h"
#include "string.h"
#ifdef __STDC__
#include <stdlib.h>
#endif

extern char *optarg;

/*
** Standard widths for name, file, function and lines; number of
** columns per line (for lines) - these can change via the -W option.
*/
#define COL 	5
static int cols = COL;

#define W_NAME	15
static int w_name = W_NAME;

#define W_FILE	13
static int w_file = W_FILE;

#define W_FUNC	15
static int w_func = W_FUNC;

#define W_LINE	4
static int w_line = W_LINE;

/*
** levels given to cxref by lint - block levels have values of 1 or greater
*/
#define STATIC	-1
#define GLOBAL	0

/*
** Number of names defined for the cross reference.
*/
static int maxnames = 0;

extern char *st_lookup();	/* from st.c			*/
static char *getline();		/* get string			*/
static void add_to_list();	/* add use/decl/defn to list	*/
static void printreport();	/* print out the report		*/
static XREF *al_xref();		/* allocation for XREF		*/
static LEV *al_lev();		/* allocation for LEV		*/
static FILEUSE *al_fileuse();	/* allocation for FILEUSE	*/
static USE *al_use();		/* allocation for USE		*/
static char trans();		/* translate code for output	*/
static void setlev();		/* set members of LEV		*/
static void sortnames();	/* sort the list		*/
static int comp();		/* comparison function for qsort() */

static char *nofunc = "---";	/* no function name		*/
static char *funcname;		/* current function name	*/
static char *fname;		/* current function name	*/
static XREF *head;		/* head of global list		*/
static XREF **nameary;		/* array to hold sorted list	*/
static int dflag = 0; 		/* -d disable printing decls	*/
static int lflag = 0; 		/* -l disable printing locals	*/
static int Lflag = 0;		/* -L n prints n columns of numbers    */
static int wflag = 0;		/* -w n prints with a width of n       */
static int Wflag = 0;		/* -Wn,n,n,n changes widths of columns */

main(argc, argv)
int argc;
char **argv;
{
    int code, c, line;
    short level = 0;
    static char *str = NULL, *name = NULL;
    char *tmpnames;
    short cmno;
    static char w_opt[] = "-w option cancelled by use of -L or -W\n";

    while ((c=getopt(argc, argv, "dlW:VL:w:t")) != EOF)
	switch (c) {
	    /* Version stamping implementation */
	    case 'V':
		(void)fprintf(stderr,"\"cxref: %s, %s\"\n",ESG_PKG,ESG_REL);
		exit(0);
		/* FALLTHRU */

	    /*
	    ** If -d specified, don't print declarations.  This makes the
	    ** report much easier to read since we don't have to bother
	    ** with all the stuff in the header files.
	    */
	    case 'd':
		dflag = 1;
		break;

	    /*
	    ** Don't print locals variables - only print globals, and
	    ** file scope statics.  This option will probably generally
	    ** be used, because the "important" part of a cross-reference
	    ** is seeing where globals (and file-scope statics) are used.
	    */
	    case 'l':
		lflag = 1;
		break;

	    /*
	    ** Specify the number of columns to print - defaults to 5
	    */
	    case 'L':
		if (wflag)
		    (void) fprintf(stderr, w_opt);
		Lflag = 1;
		cols = atoi(optarg);
		if (cols <= 0) {
		    (void) fprintf(stderr,"bad number to -L option\n");
		    exit(1);
		}
		break;

	    /*
	    ** The default is for a 80 column width - accept this
	    ** option blindly.
	    */
	    case 't':
		break;

	    /*
	    ** This option is obselete with the addition of -W and
	    ** -L.
	    */
	    case 'w': {
		if (Wflag || Lflag) {
		    (void) fprintf(stderr, w_opt);
		    break;
		}
		wflag = atoi(optarg);
		if (wflag <= 0) {
		    (void) fprintf(stderr,"bad number to -w option\n");
		    exit(1);
		}
		/*
		** Only do something special if the option to -w is
		** less than 80 - if it is greater, still print in
		** 80 columns.
		*/
		if ((wflag < 80) && (wflag >= 51)) {
		    if (wflag < (W_NAME + W_FILE + W_FUNC + 2*W_LINE + 5))
			cols = 1;
		    else if (wflag < (W_NAME + W_FILE + W_FUNC + 3*W_LINE + 6))
			cols = 2;
		    else if (wflag < (W_NAME + W_FILE + W_FUNC + 4*W_LINE + 7))
			cols = 3;
		    else if (wflag < (W_NAME + W_FILE + W_FUNC + 5*W_LINE + 8))
			cols = 4;
		}
		}
		break;

	    /*
	    ** Change default widths; comma separated list where all
	    ** 4 must be specified:
	    **    w_name, w_file, w_func, w_line
	    */
	    case 'W':
		if (wflag)
		    (void) fprintf(stderr, w_opt);
		Wflag = 1;
		tmpnames = (char *) malloc(strlen(optarg)+1);
		(void) strcpy(tmpnames, optarg);
		w_name = atoi(strtok(tmpnames, ","));
		w_file = atoi(strtok(NULL, ","));
		w_func = atoi(strtok(NULL, ","));
		w_line = atoi(strtok(NULL, ","));
		if (! w_name || ! w_file || ! w_func || ! w_line) {
		    (void) fprintf(stderr,"bad number to -W option\n");
		    exit(1);
		}
		break;

	    /*
	    ** Everything else is illegal.
	    */
	    default:
		(void) fprintf(stderr,"bad option: %d\n", c);
		exit(1);
	}

    /*
    ** Set current function name to be "---"
    */
    funcname = nofunc;

    /*
    ** Read file till EOF
    **
    ** Intermediate file is a text file with the first character
    ** of each line serving as a "type" code (type of record), and
    ** following text on the line is the record itself:
    **
    **     M #		- new (M)odule, followed by module number
    **     Nfile	- new file(N)ame, followed by file name (no space)
    **     E		- (E)nd of function definition
    **     F line level name - 
    **			  begin (F)unction name with definition at line, level
    **     L line level name - 
    **			  dec(L)aration at line, level, and name.
    **     R line level name - 
    **			  (R)eference of name at line, level.
    **     A line level name - 
    **			  (A)ssignment of name at line, level.
    **     D line level name - 
    **			  (D)efinition of name at line, level.
    **
    ** Explanation of levels:
    **   A file-scope static has level -1.
    **   A global has level 0.
    **   A function parameter has level 1.
    **   Levels 2 and up are for variables defined in blocks.
    */
    for (;;) {
	code = getchar();
	if (code == EOF)
	    break;

	switch (code) {
	    /*
	    ** New module number - set current to the number 
	    ** read in; module number assigned by the shell script.
	    ** A module number is assigned to each translation unit
	    ** (as defined in the ANSI-C draft).  This lets cxref
	    ** get scoping right for static identifiers.
	    */
	    case 'M':
		cmno = atoi(getline(&str, &name));
		break;

	    /*
	    ** New filename - set current file name to string
	    ** read in.
	    */
	    case 'N':
		fname = st_lookup(getline(&str, &name));
		break;

	    /*
	    ** End of function - no function name now.
	    */
	    case 'E':
		(void) getline(&str, &name);
		funcname = nofunc;
		break;

	    case 'F':	/* Function definition  */
	    case 'L':	/* decLaration		*/
	    case 'R':	/* Reference		*/
	    case 'A':	/* Assign		*/
	    case 'D':	/* Definition 		*/
		(void) getline(&str, &name);
		(void) sscanf(str, "%d %hd %s", &line, &level, name);
		if (! ((level > 0) && lflag))  {
		    add_to_list(code, line, st_lookup(name), level, cmno);
		    if (code == 'F')
			funcname = st_lookup(name);
		}
		break;

	    /*
	    ** File is corrupted.
	    */
	    default:
		(void) fprintf(stderr,"bad code in file: %c\n", code);
		exit(1);
	}
    }

    /*
    ** No more data - sort and dump report.
    */
    sortnames();
    printreport();
    exit(0);
    /* NOTREACHED */
}


static char *
getline(str, name)
char **str;
char **name;
{
    int c;
    char *strptr = *str;
    static int size = BUFSIZ;

    if (*str == NULL) {
	strptr = *str = (char *)malloc(size);
	*name = (char *)malloc(size);
	if ( *str == NULL || *name == NULL ) {
	    (void) fprintf(stderr, "out of heap space\n");
	    exit (1);
	}
    }
    while ((c=getchar()) != '\n') {
	*strptr++ = (char)c;
	if ( strptr >= *str + size ) {
	    size += BUFSIZ;
	    *str = (char *)realloc((char *)(*str), size);
	    *name = (char *)realloc((char *)(*name), size);
	    if ( *str == NULL || *name == NULL ) {
		(void) fprintf(stderr, "out of heap space\n");
		exit (1);
	    }
	    strptr = *str + (size - BUFSIZ);
	}
    }
    *strptr = '\0';
    return (*str);
}
	

#define printleft(str, width) (void)printf("%-*s", width, str)
#define levmatch(ptr, lev, mno) (((ptr)->level == (lev)) && \
					((ptr)->mno == (mno)))

/*
** Printreport - just go along linked lists, and print as we go along.
*/
static void
printreport()
{
    LEV *levptr;
    FILEUSE *fptr;
    USE *uptr;
    int j, i, namepr = 0, fpr = 0, tospace = 0, wwidth;

    /*
    ** Print header - if the l flag (local) was specified, don't print
    ** FUNCTION because FUNCTION is only used for local variables.
    */
    (void) printf("%-*s %-*s ", w_name, "NAME", w_file, "FILE");
    if (! lflag) {
	(void) printf("%-*s ", w_func, "FUNCTION");
	wwidth = w_name + w_file + w_func + 3;
    } else wwidth = w_name + w_file + 2;
    (void) printf("%-*s\n", w_line, "LINE");

    /*
    ** Go thru the array that was just sorted.
    */
    for (j=0;j<maxnames;j++) {
	levptr = nameary[j]->levels;

	/*
	** Loop through each different instance of this name.
	** End each name with newline.  Don't print out the
	** variable name until a non-decl is found (if the -d
	** is set).
	** Each levptr points to the same name, but in a different
	** scope.
	*/
	while (levptr != NULL) {

	    /* 
	    ** lflag specified - don't print out this level if it
	    ** is a block level.
	    */
	    if (lflag && (levptr->level > 0)) {
		levptr = levptr->next;
		continue;
	    }

	    /*
	    ** The name and file have not yet been printed for
	    ** this level.
	    */
	    namepr = fpr = 0;
	    fptr = levptr->files;

	    /*
	    ** Loop through every file this name is used/set in.
	    ** End each file usage with newline.
	    ** Don't print out the file until a non-decl is found
	    ** (if the -d is set).
	    ** Each fptr points to a different file that the variable
	    ** that levptr points to is used in.
	    */
	    while (fptr != NULL) {
		uptr = fptr->usage;
		i = 0;
	
		/*
		** Loop through every use/set of this name in this file.
		*/
		while (uptr != NULL) {

		    /*
		    ** If this is a declaration and the d flag was specified,
		    ** don't print anything out.
		    ** If this has a line number of 0, then don't print
		    ** anything out either (line number 0 is informational
		    ** only).
		    */
		    if (! (   (uptr->type == 'L') 
			   && (dflag || (uptr->line == 0))
			  )
		       ) {

			/*
			** This use/decl/set/defn is ok to print; check to
			** see if the name has been printed out yet.
			*/
			if (!namepr) {
			    printleft(nameary[j]->name, w_name+1);
			    namepr = 1;
			    tospace = 0;
			} else tospace = 1;

			/*
			** ditto as above; check if filename has been printed.
			*/
			if (!fpr) {
			    if (tospace)
				printleft("", w_name+1);
			    printleft(fptr->fname, w_file+1);
			    if (!lflag)
				printleft(levptr->funcname, w_func+1);
			    fpr = 1;
			}

			/*
			** Print cols items per row.
			*/
			if (i == cols) {
			    (void) putchar('\n');
			    printleft("", wwidth);
			    i = 0;
			}
			(void) printf("%*d%c ", w_line, uptr->line, 
					trans(uptr->type));
			i++;
		    }
		    /* next use */
		    uptr = uptr->next;
		}

		/* newline if done with file; get next file for this var. */
		if (fpr) {
		    (void) printf("\n");
		    fpr = 0;
		}
		fptr = fptr->next;
	    }

	    /* newline if done with this name; get 
	    ** next instance of this name
	    */
	    levptr = levptr->next;
	}
    }
}


/*
** Look for name along the list pointed to by ptr.
** If not found, add it to the end of the list.
** Return the pointer to the XREF containing name.
*/
static XREF *
findname(ptr, name)
XREF *ptr;
char *name;
{
    /*
    ** Search through the list until the entry with the right
    ** name was found - this is a slow, sequential search that
    ** should be improved.
    */
    while (ptr->next != NULL)
	if (ptr->name == name)
	    break;
	else ptr = ptr->next;

    /*
    ** If the name was not found, then allocate space for
    ** this name, and add it to the bottom of the list.
    */
    if (ptr->name != name) {
	ptr->next = al_xref();
	ptr = ptr->next;
	ptr->name = name;
	ptr->levels = NULL;
	ptr->next = NULL;
    }

    return ptr;
}


#define ST_OR_GL(lev) (((lev) == GLOBAL) || ((lev) == STATIC))

/*
** The main data structure looks something like this:
**
**  NAME -> level -> level -> level -> ......
**   |        |        |        |
**   |      file     file     file
**   |      |  |     |  |     |  |
**   |      | use    | use    | use
**   |      | list   | list   | list
**   |      |        |        |
**   |      file     file     file
**   |      |  |     |  |     |  |
**   |      | use    | use    | use
**   |      | list   | list   | list
**   |      |        |        |
**   |     ...      ...      ...
**   |
**  NAME -> level -> level -> level -> ......
**
**
** Each NAME corresponds to the text of a variable/function name.
**
** Each level corresponds to each variable/function with the name NAME,
** but in different scopes.  If a global version of NAME exists, it is
** the first in the list.  If file-scope statics exist, they follow
** the global (or are first if no global exists).  Note that there
** can be more than 1 file-scope static if they occur in different
** modules (identified by "mno").
** Following the globals and statics are block level versions of NAME.
** They have levels of 1 (for function parameters) or greater.
*/
static void
add_to_list(code, line, name, level, mno)
char code;
int line;
char *name;
short level, mno;
{
    XREF *ptr = head;
    USE *tmpuse;
    FILEUSE *flist;
    LEV *levptr, *tmplev;

    /*
    ** If head is null right now, then allocate space for
    ** the head of the list; set the name, levels, and next
    ** fields.
    */
    if (head == NULL) {
	head = al_xref();
	head->name = name;
	head->levels = NULL;
	head->next = NULL;
	ptr = head;
    } else ptr = findname(ptr, name);


    /*
    ** At this time, ptr points to a XREF with the same name that
    ** we are looking for.  Search along the levels list until we
    ** find one that both matches the module number we are in,
    ** and has the right level.
    */

    /*
    ** First time we have used this symbol.
    ** Set necessary information - now levptr will point to the
    ** right level.
    */
    if (ptr->levels == NULL) {
	ptr->levels = al_lev();
	levptr = ptr->levels;
	setlev(levptr, level, mno, code, NULL);
    } else {
	/*
	** Now we have to search the levels for the correct occurence
	** of this symbol.
	*/
	levptr = ptr->levels;

	switch (code) {
	    case 'R':
	    case 'A':
		/*
		** If the static or global, then we check to see if there is a
		** static available to use that also matches this module.
		** If it does not exist, then use the global.
		*/
		if (ST_OR_GL(level)) {
		    /*
		    ** First thing in the list is the static we want
		    ** to use - do nothing.
		    */
		    if (levmatch(levptr, STATIC, mno))
			/* EMPTY */;

		    /*
		    ** The next thing in the list is the static we want
		    ** to use (global may have been first), set levptr 
		    ** to that thing.
		    */
		    else if (   (levptr->next != NULL)
			     && (levmatch(levptr->next, STATIC, mno))
			    ) 
			    levptr = levptr->next;

		    /*
		    ** Otherwise the current pointer must be the global.
		    ** If not, then there is a problem - but continue
		    ** processing.
		    */
		    else if (levptr->level != GLOBAL) {
			(void) fprintf(stderr,
				"cannot find global/static: %s\n",name);
			return ;
		    }
		}

		/*
		** If the level we are looking for is > 1, then look along
		** the list until it is found.  
		** If not found, then there is a problem.
		*/
		else for (;;) {
		    if (levmatch(levptr, level, mno))
			break;
		    levptr = levptr->next;
		    if (levptr == NULL) {
			(void) fprintf(stderr,"cannot find %s at level %d\n",
					name, level);
			return ;
		    } 
		}
		break;


	    case 'L':
	    case 'D':
	    case 'F':
		/*
		** First look along the list to see if this identifier
		** has been seen in this module with the same level.
		** Only have to look at the first two in the list
		*/

		if (ST_OR_GL(level))
		    if ((levptr->mno == mno) && ST_OR_GL(levptr->level))
			break;
		    else if (   (levptr->next != NULL) 
			     && (levptr->next->mno == mno)
			     && ST_OR_GL(levptr->next->level)
			    ) {
			levptr = levptr->next;
			break;
		    }
		
		/*
		** If level == GLOBAL, then make sure that it is the first thing
		** in the list.  If it is already there (levptr->level == 0),
		** then there must have been a previous declaration.
		*/
		if (level == GLOBAL) {
		    if (levptr->level != GLOBAL) {
			tmplev = ptr->levels;
			ptr->levels = al_lev();
			levptr = ptr->levels;
			setlev(levptr, level, mno, code, tmplev);
		    }
		}

		/*
		** If level is static, then add it right after the global.
		** If the first thing in the list is not the global, then
		** make the static the first thing.
		*/
		else if (level == STATIC) {
		    if (levptr->level == GLOBAL) {
		        /*
		        ** Global in list - if the thing after the global is
		        ** what we are looking for, then use it.
		        */
			if (   (levptr->next != NULL) 
			    && (levmatch(levptr->next, STATIC, mno))
			   ) levptr = levptr->next;

			/*
			** Otherwise add it right after the global.
			*/
			else {
			    tmplev = levptr->next;
			    levptr->next = al_lev();
			    levptr = levptr->next;
			    setlev(levptr, level, mno, code, tmplev);
			}
		    }

		    /*
		    ** If the first thing in the list is what we are looking
		    ** for, use it.
		    */
		    else if (levmatch(levptr, STATIC, mno))
			/* EMPTY */;

		    /*
		    ** Otherwise, add it to the beginning of the list.
		    */
		    else {
			tmplev = ptr->levels;
			ptr->levels = al_lev();
			levptr = ptr->levels;
			setlev(levptr, level, mno, code, tmplev);
		    }
		}

		/*
		** If level is > 0, then there is a definition in a block.
		** (ignore declarations, for now, inside blocks).
		*/
		else if ((code == 'D') || (code == 'F')) {
		    if (levptr->level > 0) {
			tmplev = ptr->levels;
			ptr->levels = al_lev();
			levptr = ptr->levels;
		    } else {
			while (   (levptr->next != NULL) 
			       && (levptr->next->level < 1)
			      ) levptr = levptr->next;
			tmplev = levptr->next;
			levptr->next = al_lev();
			levptr = levptr->next;
		    }
		    setlev(levptr, level, mno, code, tmplev);
		}
		break;
	}
    }

    /*
    ** Now levptr points to the correct list for this symbol.  Search the
    ** list now for the right file.
    */

    /*
    ** No list of files - add one for the current file.
    */
    if (levptr->files == NULL) {
	levptr->files = al_fileuse();
	flist = levptr->files;
	flist->fname = fname;
	flist->usage = NULL;
	flist->next = NULL;
    } else {
	flist = levptr->files;
	while (flist->fname != fname) {
	    if (flist->next == NULL)
		break;
	    else flist = flist->next;
	}
	if (flist->fname != fname) {
	    flist->next = al_fileuse();
	    flist = flist->next;
	    flist->fname = fname;
	    flist->usage = NULL;
	    flist->next = NULL;
	}
    }

    /*
    ** Now flist points to the right file for this variable.
    ** Now just add the code to this list.
    */
    tmpuse = flist->usage;
    if (tmpuse == NULL) {
	flist->usage = flist->last = al_use();
	flist->usage->type = code;
	flist->usage->line = line;
	flist->usage->next = NULL;
    } else {
	flist->last->next = al_use();
	flist->last = flist->last->next;
	flist->last->type = code;
	flist->last->line = line;
	flist->last->next = NULL;
    }
}


/*
** Memory allocation routines.
**
** These will get called often.  For efficiency, allocate a large chunk
** and give little pieces as they are needed.  When each chunk is exhausted,
** allocate another one.
**
** There are 4 different types; define 4 different functions.  All
** are almost the same except for name and type.  Use a macro to
** automatically expand them.
*/
#define BLKSZ 100

#define AL_FUNC(func, type, stmt) \
    static type * func() { \
	static type blk0[BLKSZ]; \
	static type *blkptr = blk0; \
	static blknum = 0; \
	stmt \
	if (blknum == BLKSZ) { \
	    if ((blkptr = (type *) malloc(sizeof(type) * BLKSZ)) == NULL) { \
		(void) fprintf(stderr,"cannot allocate memory for tables\n"); \
		exit(1); \
	    } \
	    blknum = 0; \
	} \
	return blkptr + blknum++; \
}

AL_FUNC(al_xref, XREF, maxnames++;)
AL_FUNC(al_lev, LEV, ;)
AL_FUNC(al_fileuse, FILEUSE, ;)
AL_FUNC(al_use, USE, ;)

/*
** Translate lint's code into a printable format.
*/
static char
trans(c)
char c;
{
    switch (c) {
	case 'A': 
	    return '=';
	case 'R': 
	    return ' ';
	case 'D': 
	case 'F': 
	    return '*'; 
	case 'L':
	    return '-'; 
	default:
	    (void) fprintf(stderr,"bad type code in trans()\n");
	    return ' ';
    }
}


/*
** Set the level, module number, and next pointer of levptr
** to level, mno, and next respectively.
** Also, if this is a definition, set the function name.
*/
static void
setlev(levptr, level, mno, code, next)
LEV *levptr;
short level, mno;
LEV *next;
{
    levptr->level = level;
    levptr->mno = mno;
    if (code == 'D')
	levptr->funcname = funcname;
    else levptr->funcname = nofunc;
    levptr->next = next;
    levptr->files = NULL;
}


/*
** Sort the report using qsort - sort by object name.
*/
static void
sortnames()
{
    int i;
    XREF *hd = head;

    if ((nameary = (XREF **) malloc(sizeof(XREF *) * maxnames)) == NULL) {
	(void) fprintf(stderr,"cannot allocate space for sorting\n");
	exit(1);
    }
    for (i=0; i < maxnames; i++) {
	nameary[i] = hd;
	hd = hd->next;
    }
    qsort((char *)nameary, maxnames, sizeof(XREF **), comp);
}

/*
** Comparison function for qsort.
*/
static int
comp(h1, h2)
XREF **h1, **h2;
{
    return (strcmp((*h1)->name, (*h2)->name));
}
