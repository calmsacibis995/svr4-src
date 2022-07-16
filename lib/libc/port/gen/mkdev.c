/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mkdev.c	1.3"

#include	"synonyms.h"
#include	<sys/types.h>
#include	<sys/errno.h>
#include	<sys/mkdev.h>

extern int errno;

/* create a formatted device number */

dev_t
__makedev(version, majdev, mindev)
const register int	version;
const register major_t	majdev;
const register minor_t mindev;
{
dev_t devnum;
	switch(version){

		case OLDDEV:
			if  (majdev > OMAXMAJ || mindev > OMAXMIN) {
				errno = EINVAL;
				return ((o_dev_t)NODEV);
			}
			devnum = ((majdev << ONBITSMINOR) | mindev);
			break;

		case NEWDEV:
			if (majdev > MAXMAJ || mindev > MAXMIN) {
				errno = EINVAL;
				return (NODEV);
			}

			if ((devnum = ((majdev << NBITSMINOR) | mindev)) == NODEV){
				errno = EINVAL;
				return (NODEV);
			}

			break;

		default:
			errno = EINVAL;
			return (NODEV);
			
	}

	return(devnum);
}

/* return major number part of formatted device number */

major_t
__major(version, devnum)
const register int version;
const register dev_t devnum;
{
major_t maj;

	switch(version) {

		case OLDDEV:

			maj = (devnum >> ONBITSMINOR);
			if (devnum == NODEV || maj > OMAXMAJ) {
				errno = EINVAL;
				return (NODEV);
			}
			break;

		case NEWDEV:
			maj = (devnum >> NBITSMINOR);
			if (devnum == NODEV || maj > MAXMAJ) {
				errno = EINVAL;
				return (NODEV);
			}
			break;

		default:

			errno = EINVAL;
			return (NODEV);
	}

	return (maj);
}


/* return minor number part of formatted device number */

minor_t
__minor(version, devnum)
const register int version;
const register dev_t devnum;
{

	switch(version) {

		case OLDDEV:

			if (devnum == NODEV) {
				errno = EINVAL;
				return(NODEV);
			}
			return(devnum & OMAXMIN);
			break;

		case NEWDEV:

			if (devnum == NODEV) {
				errno = EINVAL;
				return(NODEV);
			}
			return(devnum & MAXMIN);
			break;

		default:

			errno = EINVAL;
			return(NODEV);
	}
}
