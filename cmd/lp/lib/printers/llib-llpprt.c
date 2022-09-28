/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* LINTLIBRARY */

/*	from file chkprinter.c */
# include	"lp.h"
# include	"printers.h"

/**
 ** chkprinter() - CHECK VALIDITY OF PITCH/SIZE/CHARSET FOR TERMINFO TYPE
 **/
unsigned long chkprinter (const char * type, const char * cpi,
			  const char * lpi, const char * len,
			  const char * wid, const char * cs)
{
    static unsigned long  _returned_value;
    return _returned_value;
}

/*	from file default.c */

/**
 ** getdefault() - READ THE NAME OF THE DEFAULT DESTINATION FROM DISK
 **/
char * getdefault ( void )
{
    static char * _returned_value;
    return _returned_value;
}

/**
 ** putdefault() - WRITE THE NAME OF THE DEFAULT DESTINATION TO DISK
 **/
int putdefault ( const char * dflt )
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** deldefault() - REMOVE THE NAME OF THE DEFAULT DESTINATION
 **/
int deldefault ( void )
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file delprinter.c */

/**
 ** delprinter()
 **/
int delprinter ( const char * name )
{
    static int  _returned_value;
    return _returned_value;
}


/*	from file freeprinter.c */
/**
 **  freeprinter() - FREE MEMORY ALLOCATED FOR PRINTER STRUCTURE
 **/
void freeprinter (PRINTER * pp)
{
}

/*	from file getprinter.c */

/**
 ** getprinter() - EXTRACT PRINTER STRUCTURE FROM DISK FILE
 **/
PRINTER * getprinter ( const char * name )
{
    static PRINTER * _returned_value;
    return _returned_value;
}

/*	from file okprinter.c */

/**
 ** okprinter() - SEE IF PRINTER STRUCTURE IS SOUND
 **/
int okprinter (const char * name, const PRINTER * prbufp, int isput)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file printwheels.c */
/**
 ** getpwheel() - GET PRINT WHEEL INFO FROM DISK
 **/
PWHEEL * getpwheel (const char * name)
{
    static PWHEEL * _returned_value;
    return _returned_value;
}

/**
 ** putpwheel() - PUT PRINT WHEEL INFO TO DISK
 **/
int putpwheel ( const char * name, const PWHEEL * pwheelp)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** delpwheel() - DELETE PRINT WHEEL INFO FROM DISK
 **/
int delpwheel (const char * name)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 **  freepwheel() - FREE MEMORY ALLOCATED FOR PRINT WHEEL STRUCTURE
 **/
void freepwheel ( PWHEEL * ppw)
{
}

/*	from file putprinter.c */
/**
 ** putprinter() - WRITE PRINTER STRUCTURE TO DISK FILES
 **/
int putprinter (const char * name, const PRINTER * prbufp)
{
    static int  _returned_value;
    return _returned_value;
}
