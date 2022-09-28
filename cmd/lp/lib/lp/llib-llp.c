/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/

/* from file Syscalls.c */
# include	<fcntl.h>
# include	"lp.set.h"
# include	"lp.h"

_Access ( const char * s, int i )
{
 static _returned_value;
 return _returned_value;
}
_Chdir ( const char * s )
{
 static _returned_value;
 return _returned_value;
}
_Chmod ( const char * s, int i )
{
 static _returned_value;
 return _returned_value;
}
_Chown (const char * s, int i, int j )
{
 static _returned_value;
 return _returned_value;
}
_Close ( int i )
{
 static _returned_value;
 return _returned_value;
}
_Creat ( const char * s, int i )
{
 static _returned_value;
 return _returned_value;
}
_Fcntl ( int i, int j, ...)
{
 static _returned_value;
 return _returned_value;
}
_Fstat ( int i, struct stat * st )
{
 static _returned_value;
 return _returned_value;
}
_Link ( const char * s1, const char * s2)
{
 static _returned_value;
 return _returned_value;
}
_Mknod ( const char * s, int i, int j )
{
 static _returned_value;
 return _returned_value;
}
_Open ( const char * s, int i, ... )
{
 static _returned_value;
 return _returned_value;
}
_Read ( int i, char * s, unsigned j )
{
 static _returned_value;
 return _returned_value;
}
_Stat ( const char * s, struct stat * st )
{
 static _returned_value;
 return _returned_value;
}
_Unlink ( const char * s )
{
 static _returned_value;
 return _returned_value;
}
_Wait ( int * i )
{
 static _returned_value;
 return _returned_value;
}
_Write ( int i, const char * s, unsigned j )
{
 static _returned_value;
 return _returned_value;
}

/* from file addlist.c */

int addlist ( char *** plist, const char * item )
{
    static int _returned_value;
    return _returned_value;
}

/* from file appendlist.c */

int appendlist ( char *** plist, const char * item )
{
    static int _returned_value;
    return _returned_value;
}


/**
 ** putalert () - WRITE ALERT TO FILES
 **/
int putalert ( const char * parent, const char * name,
	       const FALERT * alertp )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** getalert () - EXTRACT ALERT FROM FILES
 **/
FALERT * getalert ( const char * parent, const char * name )
{
    static FALERT * _returned_value;
    return _returned_value;
}

/**
 ** delalert () - DELETE ALERT FILES
 **/
int delalert ( const char * parent, const char * name )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** envlist () - PRINT OUT ENVIRONMENT LIST SAFELY
 **/

/**
 ** printalert () - PRINT ALERT DESCRIPTION
 **/
void printalert ( FILE * fp, const FALERT * alertp, int isfault )
{
}

/* from file charset.c */
/**
 ** search_cslist () - SEARCH CHARACTER SET ALIASES FOR CHARACTER SET
 **/
char * search_cslist ( const char * item, const char ** list )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file cs_strcmp.c */
int cs_strcmp ( const char * s1, const char * s2)
{
    static int
_returned_value;
    return _returned_value;
}

/* from file cs_strncmp.c */
int cs_strncmp ( const char * s1, const char * s2, int n )
{
    static int
_returned_value;
    return _returned_value;
}

/* from file dellist.c */

/**
 ** dellist () - REMOVE ITEM FROM (char **) LIST
 **/
int dellist ( char *** plist, const char * item )
{
    static int _returned_value;
    return _returned_value;
}

/* from file dashos.c */
/**
 ** dashos () - PARSE -o OPTIONS, (char *) --> (char **)
 **/
char ** dashos ( char * o )
{
    static char ** _returned_value;
    return _returned_value;
}

/* from file dirs.c */

/**
 ** mkdir_lpdir ()
 **/
int mkdir_lpdir ( const char * path, int mode )
{
    static int _returned_value;
    return _returned_value;
}


/**
 ** duplist () - DUPLICATE A LIST OF STRINGS
 **/
char ** duplist ( const char ** src )
{
    static char ** _returned_value;
    return _returned_value;
}


/**
 ** open_lpfile () - OPEN AND LOCK A FILE; REUSE STATIC BUFFER
 ** close_lpfile () - CLOSE FILE; RELEASE STATIC BUFFER
 **/

/*VARARGS2*/
FILE * open_lpfile ( const char * path, const char * type, int mode )
{
    static FILE * _returned_value;
    return _returned_value;
}
int close_lpfile ( FILE * fp )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** chown_lppath ()
 **/
int chown_lppath ( const char * path )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** rmfile () - UNLINK FILE BUT NO COMPLAINT IF NOT THERE
 **/
int rmfile ( const char * path )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** loadline () - LOAD A ONE-LINE CHARACTER STRING FROM FILE
 **/
char * loadline ( const char * path )
{
    static char * _returned_value;
    return _returned_value;
}

/**
 ** loadstring () - LOAD A CHARACTER STRING FROM FILE
 **/
char * loadstring ( const char * path )
{
    static char * _returned_value;
    return _returned_value;
}

/**
 ** dumpstring () - DUMP CHARACTER STRING TO FILE
 **/
int dumpstring ( const char * path, const char * str )
{
    static int _returned_value;
    return _returned_value;
}

/* from file freelist.c */
/**
 ** freelist () - FREE ALL SPACE USED BY LIST
 **/
void freelist ( char ** list )
{
}


/**
 ** getlist () - CONSTRUCT LIST FROM STRING
 **/
char ** getlist ( char * str, const char * ws,
		  const char * hardsep )
{
    static char ** _returned_value;
    return _returned_value;
}

/**
 ** unq_strdup ()
 **/

/* from file getname.c */
char * getname ( void )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file getpaths.c */

# undef	getpaths
# undef	getadminpaths

void getpaths ( void )
{
}
void getadminpaths ( const char * admin )
{
}

/**
 ** getprinterfile () - BUILD NAME OF PRINTER FILE
 **/
char * getprinterfile ( const char * name, const char * component )
{
    static char * _returned_value;
    return _returned_value;
}

/**
 ** getclassfile () - BUILD NAME OF CLASS FILE
 **/
char * getclassfile ( const char * name )
{
    static char * _returned_value;
    return _returned_value;
}

/**
 ** getfilterfile () - BUILD NAME OF FILTER TABLE FILE
 **/
char * getfilterfile ( const char * table )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file getspooldir.c */
char * getspooldir ( void )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file isterminfo.c */

/**
 ** isterminfo () - SEE IF TYPE IS IN TERMINFO DATABASE
 **/
int isterminfo ( const char * type )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** _isterminfo ()
 **/

/* from file lenlist.c */
/**
 ** lenlist () - COMPUTE LENGTH OF LIST
 **/
int lenlist ( const char ** list )
{
    static int _returned_value;
    return _returned_value;
}


/**
 ** makepath () - CREATE PATHNAME FROM COMPONENTS
 **/

/*VARARGS0*/
char * makepath ( const char * s, ...)
{
    static char * _returned_value;
    return _returned_value;
}


/**
 ** makestr () - CONSTRUCT SINGLE STRING FROM SEVERAL
 **/

/*VARARGS0*/
char * makestr ( const char * s, ...)
{
    static char * _returned_value;
    return _returned_value;
}

/* from file mergelist.c */

/**
 ** mergelist () - ADD CONTENT OF ONE LIST TO ANOTHER
 **/
int mergelist ( char *** dstlist, const char ** srclist )
{
    static int _returned_value;
    return _returned_value;
}

/* from file printlist.c */
/**
 ** printlist_setup () - ARRANGE FOR CUSTOM PRINTING
 ** printlist_unsetup () - RESET STANDARD PRINTING
 **/
void printlist_setup ( const char * prefix, const char * suffix,
		       const char * sep, const char * newline )
{
}
void printlist_unsetup ( void )
{
}

/**
 ** printlist () - PRINT LIST ON OPEN CHANNEL
 **/
int printlist ( FILE * fp, const char ** list )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** q_print () - PRINT STRING, QUOTING SEPARATOR CHARACTERS
 **/

/* from file sdn.c */
/**
 ** printsdn () - PRINT A SCALED DECIMAL NUMBER NICELY
 **/
void printsdn_setup ( const char * prefix, const char * suffix,
		      const char * newline )
{
}
void printsdn_unsetup ( void )
{
}
void printsdn ( FILE * fp, SCALED sdn )
{
}

/* from file sprintlist.c */
/**
 ** sprintlist () - FLATTEN (char **) LIST INTO (char *) LIST
 **/
char * sprintlist ( const char ** list )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file searchlist.c */
/**
 ** searchlist () - SEARCH (char **) LIST FOR ITEM
 **/
int searchlist ( const char * item, const char ** list )
{
    static int _returned_value;
    return _returned_value;
}

/* from file set_charset.c */

int set_charset ( const char * char_set, int putout, const char * type )
{
    static int _returned_value;
    return _returned_value;
}

/* from file set_pitch.c */

/**
 ** set_pitch ()
 **/
int set_pitch ( const char * str, int which, int putout )
{
    static int _returned_value;
    return _returned_value;
}

/* from file set_size.c */

int set_size ( const char * str, int which, int putout )
{
    static int _returned_value;
    return _returned_value;
}

/* from file sop.c */
/**
 ** sop_up_rest () - READ REST OF FILE INTO STRING
 **/
char * sop_up_rest ( FILE * fp, char * endsop )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file strip.c */
/**
 ** strip () - STRIP LEADING AND TRAILING BLANKS
 **/
char * strip ( char * str )
{
    static char * _returned_value;
    return _returned_value;
}

/* from file syntax.c */
int syn_name ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}
int syn_type ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}
int syn_text ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}
int syn_comment ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}
int syn_machine_name ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}
int syn_option ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}

/* from file tidbit.c */
/**
 ** _Getsh () - GET TWO-BYTE SHORT FROM (char *) POINTER PORTABLY
 **/

/**
 ** tidbit () - TERMINFO DATABASE LOOKUP
 **/

/*VARARGS2*/
int tidbit ( const char * term, const char * cap, ...)
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** untidbit () - FREE SPACE ASSOCIATED WITH A TERMINFO ENTRY
 **/
void untidbit ( const char * term )
{
}

/**
 ** open_terminfo_file () - OPEN FILE FOR TERM ENTRY
 **/

/* from file wherelist.c */
/**
 ** wherelist () - RETURN POINTER TO ITEM IN LIST
 **/
char ** wherelist ( const char * item, const char ** list )
{
    static char ** _returned_value;
    return _returned_value;
}

/* from file which.c */
/**
 ** isprinter () - SEE IF ARGUMENT IS A REAL PRINTER
 **/
int isprinter ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** isclass () - SEE IF ARGUMENT IS A REAL CLASS
 **/
int isclass ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** isrequest () - SEE IF ARGUMENT LOOKS LIKE A REAL REQUEST
 **/
int isrequest ( const char * str )
{
    static int _returned_value;
    return _returned_value;
}

int isnumber ( const char * s )
{
    static int _returned_value;
    return _returned_value;
}

/*	from file next.c */

# if	defined(__STDC__)
char * next_x ( const char * parent, long * lastdirp, unsigned int what )
#else
char *
next_x(parent, lastdirp, what)
char		*parent;
long		*lastdirp;
unsigned int	what;
#endif
{
    static char * _returned_value;
    return _returned_value;
}
