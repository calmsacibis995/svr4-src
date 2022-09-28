/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/

/* from file _getmessage.c */
# include <stdarg.h>
# include "msgs.h"

/* VARARGS */
int _getmessage ( char * buf, short rtype, va_list arg )
{
    static int _returned_value;
    return _returned_value;
}

/* from file _putmessage.c */
/* VARARGS */
int _putmessage ( char * buf, short type, va_list arg )
{
    static int _returned_value;
    return _returned_value;
}

/* from file getmessage.c */
/* VARARGS */
int getmessage ( char * buf, short type, ... )
{
    static int _returned_value;
    return _returned_value;
}

/* from file hslconv.c */
char * ltos ( char * s, unsigned long l)
{
    static char * _returned_value;
    return _returned_value;
}
char * htos ( char * s, unsigned short h)
{
    static char * _returned_value;
    return _returned_value;
}
unsigned long stol ( const char * s )
{
    static unsigned long _returned_value;
    return _returned_value;
}
unsigned short stoh ( const char * s )
{
    static unsigned short _returned_value;
    return _returned_value;
}

/* from file putmessage.c */
/* VARARGS */
int putmessage(char * buf, short type, ... )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mclose.c */
int mclose ( void )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mconnect.c */
MESG * mconnect ( const char * path, int id1, int id2 )
{
    static MESG * _returned_value;
    return _returned_value;
}

/* from file mdisconnect.c */
int mdisconnect ( MESG * md )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mgetputm.c */
int mgetm ( MESG * md, int type, ... )
{
    static int _returned_value;
    return _returned_value;
}
int mputm ( MESG * md, int type, ... )
{
    static int _returned_value;
    return _returned_value;
}
void __mbfree ( void )
{
}
short mpeek ( MESG * md )
{
    static short _returned_value;
    return _returned_value;
}

/* from file mlisten.c */
int mlisteninit ( MESG * md )
{
    static int _returned_value;
    return _returned_value;
}
int mlistenadd ( MESG * md, short events )
{
    static int _returned_value;
    return _returned_value;
}
MESG * mlistenreset ( void )
{
    static MESG * _returned_value;
    return _returned_value;
}
MESG * mlisten ( void )
{
    static MESG * _returned_value;
    return _returned_value;
}
int mon_discon ( MESG * md, void (*fn)() )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mcreate.c */

MESG * mcreate ( const char * path )
{
    static MESG * _returned_value;
    return _returned_value;
}

/* from file mdestroy.c */
int mdestroy ( MESG * md )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mneeds.c */
/**
** mneeds() - RETURN NUMBER OF FILE DESCRIPTORS NEEDED BY mopen()
**/
int mneeds ( void )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mopen.c */
int
mopen ( void )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mread.c */
int mread ( const MESG * md, char * msgbuf, int size )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mrecv.c */
int mrecv ( char *msgbuf, int size )
{
    static int _returned_value;
    return _returned_value;
}

/* from file msend.c */
int msend ( char * msgbuf )
{
    static int _returned_value;
    return _returned_value;
}

/* from file mwrite.c */
int mwrite ( MESG * md, char * msgbuf )
{
    static int _returned_value;
    return _returned_value;
}

/* from file read_fifo.c */
int read_fifo ( int fifo, char *buf, unsigned int size)
{
    static int _returned_value;
    return _returned_value;
}
int peek3_2 ( int fifo )
{
    static int _returned_value;
    return _returned_value;
}

/* from file write_fifo.c */
int write_fifo ( int fifo, char *buf, unsigned int size)
{
    static int _returned_value;
    return _returned_value;
}
