/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:Sioctl.c	1.1"

#include <errno.h>
#include <stdio.h>
#include "vars.h"
#include <sys/termio.h>

extern int errno;

/*
 * Ioctl is called when the 286 program does an ioctl.  It's mission
 * is to convert the arguments from what the 286 program supplies to
 * what the 386 expects, do the ioctl, and adjust the results to be
 * in whatever form the 286 program wants.
 *
 * Currently, the only ioctl's supported are the terminal ones.
 *
 * There is a special case for the blit ioctl to get window size.
 * We don't support the blit.  However, a lot of programs do this
 * one ioctl.  Normally, an unimplemented ioctl gets an error message,
 * so that someone will know that it needs implementing.  For the
 * blit window size ioctl, this message is not printed.
 *
 * Too see how to add more ioctls, read the comments and the example
 * given in Ioctl().
 */
Ioctl(ap)
	short *ap;      /* pointer to 286 args */
{
	int arg;        /* 386 arg */

	/* convert the arg as appropriate for the request */
	switch (ap[1]) {

	default:
		/*
		 * check for request for window size on the blit.
		 * If that is what it is, then don't print an error
		 * message.
		 */
		if ( ap[1] != 0x6a05 )
			ioctlError( ap[0], ap[1] );
		return -1;
		break;
	/*
	 * If an ioctl just supplies a simple argument ( i.e., no
	 * pointer ), and the command is the same on the 286 and
	 * the 386, then just do the ioctl.
	 */
	case TCSBRK:
	case TCXONC:
	case TCFLSH:
		return ioctl( ap[0], ap[1], ap[2] );
		break;

	/*
	 * If the ioctl supplies a pointer to a structure, and that
	 * structure has the same byte layout on the 386 that it
	 * does on the 286, then the pointer must be conveted to
	 * a 386 pointer.
	 */
	case TCGETA:
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		arg = (int)cvtptr(*(int *)&ap[2]);
		return ioctl( ap[0], ap[1], arg );
		break;

#ifdef EXAMPLE
	/*
	 * This is an example of how an ioctl would be handled that
	 * requires conversion of a pointer to a structure
	 *
	 * Assume that this is the structure:
	 *	struct example {
	 *		short member1;
	 *		long member2;
	 *		long member3;
	 *		long member4;
	 *	};
	 *
	 * On the 286, things are aligned to short boundries, whereas
	 * on the 386 they are aligned to long boundries.  Thus, on
	 * the 286 the offsets of the four members are 0,2,6, and 10.
	 * On the 386 they are 0,4,8, and 12.
	 */
	case EXAMPLE:
	{
		struct example s386;
		char * ps286, *p386;
		int retval;

		/*
		 * First get a pointer to the 286 structure
		 *
		 * cvtchkptr takes an address from the 286 program
		 * and returns a 386 address that points to the same
		 * place, if the address was valid.  If it was not
		 * valid, then it longjmps back to the system call
		 * handler in syscall.c.  This will return EFAULT
		 * to the 286 program.
		 */
		ps286 = cvtchkptr(*(int *)&ap[2]);
		ps386 = &p386;
		/*
		 * Now convert the 286 struct to 386 form
		 *
		 * For most structures, this will just involve
		 * moving things around, because of alignment
		 * differences.  However, if something is declared
		 * as an "int" in both machines, then you will
		 * have to handle the conversion from 16 to 32
		 * bits.
		 */
		copymem( ps286,    ps386,   2 );	/* member1 */
		copymem( ps286+2, ps386+4, 12 );	/* member2,3,4 */ 

		/*
		 * Do the ioctl, using the 386 structure
		 */
		retval = ioctl( ap[0], ap[1], ps386 );

		/*
		 * If the ioctl modifies the structure, then it
		 * will have to be copied back and converted to
		 * the 286 program.  That would go here.
		 *
		 * If this may change errno, then errno should be
		 * saved and then restored when we are done.
		 */

		/*
		 * Return whatever the ioctl returned.  If an error
		 * occured, we should leave errno set to whatever
		 * the ioctl set it to.  This will be passed back
		 * to the 286 program.
		 */
		return retval;
	}
#endif
	}
}

#include <sys/stat.h>

/*
 * Report an unrecognized request
 */
ioctlError( fd, request )
	int fd;			/* file descriptor for device */
	int request;		/* desired ioctl */
{
	struct stat s;
	int dev;
	int mode;

	emprintf( "Unknown ioctl request: 0x%x on file %d\n",
		request, fd );

	if ( fstat( fd, &s ) == -1 ) {
		emprintf( "Attempt to stat file failed\n" );
		return;
	}
	mode = s.st_mode;
	dev = s.st_rdev;

	if ( (mode & 0170000) != 0020000 ) {
		emprintf( "File is not a character special device\n" );
		return;
	}

	emprintf( "Device ID: 0x%x", dev );
	finddev( dev );
	fprintf( stderr, "\n" );
}

/*
 * finddev( dev ) finds out what device has the specified ID.
 * It assumes that all devices are under /dev.  It works by
 * using find(1) to find all character devices.  It determines
 * wich one matches the specified ID by stat(2)ing each one.
 *
 * This can be slow.
 *
 * If you #define NOFIND, finddev will search /dev itseld, assuming
 * a system V directory structure.  Note that this is not portable.
 * It is included here because it was written before the person
 * who wrote this stuff thought of using find(1) to do the work.
 */

#ifdef NOFIND
#include <sys/dir.h>

finddev( dev )
	int dev;
{
	checkdir( "/dev", dev );
}

checkdir( path, dev )
	char * path;
	int dev;
{
	struct direct d;
	struct stat s;
	int cdev;
	int fd;
	char *np;
	char name[256];


	fd = open( path, 0 );
	if ( fd < 0 )
		return 0;

	strcpy( name, path );
	strcat( name, "/" );
	np = name + strlen(name);
	np[DIRSIZ] = '\0';
	while ( read( fd, &d, sizeof d) == sizeof d ) {
		if ( d.d_ino == 0 )
			continue;
		if ( d.d_name[0] == '.' ) {
			if ( d.d_name[1] == '\0' )
				continue;
			else
			if ( d.d_name[1] == '.' && d.d_name[2] == '\0' );
				continue;
		}
		strncpy( np, d.d_name, 14 );
		if ( stat( name, &s ) == -1 )
			continue;
		switch ( s.st_mode & 0170000 ) {
		case 0040000:
			if ( checkdir( name, dev ) != 0 )
				return 1;
			break;
		case 0020000:
			cdev = s.st_rdev;
			if ( cdev == dev ) {
				fprintf( stderr, " Name: %s", name );
				return 1;
			}
			break;
		}
	}
	return 0;
}
#endif

#ifndef NOFIND
finddev( dev )
	int dev;
{
	FILE *fp, *popen();
	char path[256];
	extern char *strchr();
	struct stat s;
	int cdev;

	fp = popen( "find /dev -type c -print 2>/dev/null", "r" );
	while ( fgets(path,256,fp) != NULL ) {
		char *np;
		np = strchr(path,'\n');
		if ( np != NULL )
			*np = '\0';
		if ( stat( path, &s ) == -1 )
			continue;
		cdev = s.st_rdev;
		if ( cdev != dev )
			continue;
		fprintf( stderr, " Name: %s", path );
		break;
	}
	pclose(fp);
}
#endif
