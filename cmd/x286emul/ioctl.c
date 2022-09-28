/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:ioctl.c	1.1"

#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/termio.h>
#include <sys/ttold.h>
#include <sys/emap.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include "vars.h"

extern int errno;

/*
 * Ioctl is called when the 286 program does an ioctl.  It's mission
 * is to convert the arguments from what the 286 program supplies to
 * what the 386 expects, do the ioctl, and adjust the results to be
 * in whatever form the 286 program wants.
 *
 * Too see how to add more ioctls, read the comments and the example
 * given in Ioctl().
 */
Ioctl(ap)
	unsigned short *ap;      /* pointer to 286 args */
{
	int arg;        /* 386 arg */

#ifdef TRACE
	if (systrace) {
		fprintf(dbgfd,"Ioctl '%c'<<8|%d ", (ap[1]>>8)&0xff, ap[1]&0xff);
	}
#endif
	/* 
	 * because these ioctl numbers conflicted with new UNIX
	 * streams ioctl numbers, they were moved.  Old XENIX applications
	 * don't know this, and blithely generate the old numbers.
	 * XENIX 386 applications appear as x.out executables, so the
	 * driver knows to map the requests.  However, the emulator
	 * appears as a COFF executable, so the driver declines to make
	 * the mapping.  Thus, we do it here.
	 */
	switch(ap[1]) {
	case O_SW_B40x25:
		ap[1] = SW_B40x25;
		break;
	case O_SW_B80x25:
		ap[1] = SW_B80x25;
		break;
	case O_SW_BG320:
		ap[1] = SW_BG320;
		break;
	case O_SW_BG640:
		ap[1] = SW_BG640;
		break;
	case O_SW_C40x25:
		ap[1] = SW_C40x25;
		break;
	case O_SW_C80x25:
		ap[1] = SW_C80x25;
		break;
	case O_SW_CG320:
		ap[1] = SW_CG320;
		break;
	case O_SW_CG320_D:
		ap[1] = SW_CG320_D;
		break;
	case O_SW_CG640_E:
		ap[1] = SW_CG640_E;
		break;
	case O_SW_CG640x350:
		ap[1] = SW_CG640x350;
		break;
	case O_SW_EGAMONO80x25:
		ap[1] = SW_EGAMONO80x25;
		break;
	case O_SW_EGAMONOAPA:
		ap[1] = SW_EGAMONOAPA;
		break;
	case O_SW_ENHB40x25:
		ap[1] = SW_ENHB40x25;
		break;
	case O_SW_ENHB80x25:
		ap[1] = SW_ENHB80x25;
		break;
	case O_SW_ENHB80x43:
		ap[1] = SW_ENHB80x43;
		break;
	case O_SW_ENHC40x25:
		ap[1] = SW_ENHC40x25;
		break;
	case O_SW_ENHC80x25:
		ap[1] = SW_ENHC80x25;
		break;
	case O_SW_ENHC80x43:
		ap[1] = SW_ENHC80x43;
		break;
	case O_SW_ENH_CG640:
		ap[1] = SW_ENH_CG640;
		break;
	case O_SW_ENH_MONOAPA2:
		ap[1] = SW_ENH_MONOAPA2;
		break;
	case O_SW_MCAMODE:
		ap[1] = SW_MCAMODE;
		break;
	default: /* no action needed */
		break;
	}

	/* convert the arg as appropriate for the request */
	switch (ap[1]) {

	default:
		errno = EINVAL;
#ifdef IOCTLTRACE
		ioctlError( ap[0], ap[1] );
#endif /* IOCTLTRACE */
		return -1;
		break;
	/*
	 * If the ioctl supplies a pointer to a structure, and that
	 * structure has the same byte layout on the 386 that it
	 * does on the 286, then the pointer must be conveted to
	 * a 386 pointer.
	 */
	case TCGETA:		case TCSETA:		case TCSETAW:
	case TCSETAF:		case GETFKEY:		case GIO_KEYMAP:
	case GIO_STRMAP:	case PIO_KEYMAP:	case PIO_STRMAP:
	case SETFKEY:		case LDNMAP:		case LDGMAP:
	case LDSMAP:		case TIOCSETD:		case TIOCGETD:
	case TIOCGETC:		case TIOCSETC:
	case GIO_STRMAP_21:	case PIO_STRMAP_21:
		arg = (int)cvtptr(Ldata ? *(int *)&ap[2]
					: ap[2] | (Stacksel << 16) );
#ifdef TRACE
		if (systrace) {
			fprintf( dbgfd,"arg 2 0x%lx -> 0x%lx\n",
			Ldata ? *(int *)&ap[2] : ap[2] | (Stacksel << 16), arg);
		}
#endif
		return ioctl( ap[0], ap[1], arg );
		break;

	/*
	 * These ioctls either don't use the arg value or need only an int
	 * conversion.
	 */
	case CGA_GET:		case CONS_CURRENT:	case CONS_GET:
	case EGA_GET:		case GIO_ATTR:		case GIO_COLOR:
	case KBENABLED:		case PGA_GET:		case SWAPCGA:
	case SWAPEGA:		case SWAPMONO:		case SWAPPGA:
	case SW_B40x25:		case SW_B80x25:		case SW_BG320:
	case SW_BG640:		case SW_C40x25:		case SW_C80x25:
	case SW_CG320:		case SW_CG320_D:	case SW_CG640_E:
	case SW_CG640x350:	case SW_EGAMONO80x25:	case SW_EGAMONOAPA:
	case SW_ENHB40x25:	case SW_ENHB80x25:	case SW_ENHB80x43:
	case SW_ENHC40x25:	case SW_ENHC80x25:	case SW_ENHC80x43:
	case SW_ENH_CG640:	case SW_ENH_MONOAPA2:	case SW_MCAMODE:
	case TIOCKBOF:		case TIOCKBON:		case DIOCGETP:
	case DIOCSETP:		case FIORDCHK:		case IOCTYPE:
	case LDCHG:		case LDCLOSE:		case LDOPEN:
	case TCFLSH:		case TCSBRK:		case TCXONC:
	case TIOCEXCL:		case TIOCNXCL:		case TIOCFLUSH:
	case TIOCHPCL:		case MCA_GET:
		return ioctl(ap[0], ap[1], ap[2]);

	/*
	 * These ioctls return a 286 selector that maps the console
	 * screen memory.  We must determine how much memory is needed
	 * for the adapter and mode in order to map things properly,
	 * so we ask the kernel for a CONS_GET and determine from the
	 * mode returned.
	 */
	case MAPCGA:
	case MAPEGA:
	case MAPMONO:
	case MAPPGA:
	case MAPCONS:
	{
#define KB(x)	((x)*1024L)		/* x Kbytes */
		char *scrbase;		/* screen base address */
		long size;		/* screen memory size */
		unsigned short sel;	/* 286 selector */

		/* find out how much screen memory is avail in this mode */
		switch(ioctl(ap[0], CONS_GET, 0)) {
		default:
		case M_EGAMONO80x25:
		case M_MCA_MODE:
			size = KB(32);
			break;
		case M_B40x25:	case M_C40x25:	case M_B80x25:
		case M_C80x25:	case M_BG320:	case M_CG320:
		case M_BG640:	case M_CG320_D:	case M_CG640_E:
		case M_ENH_B80x25: case M_ENH_C80x25:	case M_ENH_B80x43:
		case M_ENH_C80x43:
			size = KB(16);
			break;
		case M_EGAMONOAPA:
		case M_CG640x350:
		case M_ENHMONOAPA2:
		case M_ENH_CG640:
			size = KB(64);
			break;
		case M_ENH_B40x25:
		case M_ENH_C40x25:
			size = KB(128);
			break;
		}

			/* do ioctl, get 386 screen base address */
		if( (scrbase = (char *)ioctl(ap[0], ap[1], ap[2])) ==
							(char *)-1 ) {
			return -1;
		}

		/*
		 * Assign 1 or 2 selectors to map the screen memory,
		 * depending on how much is available.  The biggest
		 * memory is 128k, which will get 2 consecutive selectors.
		 */
		sel = nextfreedseg();
		if(size <= KB(64)) {
			setsegdscr(sel*8+7, scrbase, size, size, (2<<16)|DATA);
		} else {
			setsegdscr(sel*8+7, scrbase,
				KB(64), KB(64), (2<<16)|DATA );
			setsegdscr(nextfreedseg()*8+7, scrbase+KB(64),
				size-KB(64), size-KB(64), (2<<16)|DATA );
		}
			
		return(sel*8+7);
	}

	/*
	 * These ioctls use a struct sgttyb *.
	 */
	case TIOCGETP:
	case TIOCSETP:
	case TIOCSETN:
	{
		struct sgttyb {		/* UNIX sgttyb struct */
			char	sg_ispeed;		/* input speed */
			char	sg_ospeed;		/* output speed */
			char	sg_erase;		/* erase character */
			char	sg_kill;		/* kill character */
			int	sg_flags;		/* mode flags */
		} s;
		struct xsgttyb {	/* XENIX 286 sgttyb struct */
			char	sg_ispeed;		/* input speed */
			char	sg_ospeed;		/* output speed */
			char	sg_erase;		/* erase character */
			char	sg_kill;		/* kill character */
			short	sg_flags;		/* mode flags */
		} * s286;
		short rv;

		s286 = (struct xsgttyb *)cvtptr(Ldata ? *(int *)&ap[2]
					: ap[2] | (Stacksel << 16) );
		if (s286 == (struct xsgttyb *)BAD_ADDR) {
			errno = EFAULT;
			return -1;
		}
		s.sg_ispeed = s286->sg_ispeed;
		s.sg_ospeed = s286->sg_ospeed;
		s.sg_erase = s286->sg_erase;
		s.sg_kill = s286->sg_kill;
		s.sg_flags = s286->sg_flags;

		rv = ioctl(ap[0], ap[1], &s);

		s286->sg_ispeed = s.sg_ispeed;
		s286->sg_ospeed = s.sg_ospeed;
		s286->sg_erase = s.sg_erase;
		s286->sg_kill = s.sg_kill;
		s286->sg_flags = s.sg_flags;

		return rv;
	}

#ifdef EXAMPLE
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

#ifdef IOCTLTRACE
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

	fp = popen( "/usr/bin/find /dev -type c -print 2>/dev/null", "r" );
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
#endif /* IOCTLTRACE */

