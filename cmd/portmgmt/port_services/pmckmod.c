/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)portmgmt:port_services/pmckmod.c	1.2.2.1"

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include <sys/types.h>
# include <ctype.h>
# include <string.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/stropts.h>
# include <sys/sad.h>

#define	NSTRPUSH	9	/* should agree with the tunable in	*/
				/* 		/etc/master.d/kernel	*/

static	int	vml();

main(argc, argv)
int	argc;
char	*argv[];
{
	return(vml(argv[1]));
}

/*
 * vml(modules)	- validate a list of modules
 *		- return 0 if successful, -1 if failed
 */
static	int
vml(modules)
char	*modules;
{
	char	buf[BUFSIZ];
	char	*modp = buf;
	int	i, j, fd;
	struct str_mlist newmods[NSTRPUSH];	/* modlist for newlist	*/
	struct str_list	newlist;		/* modules to be pushed	*/

	if ((modules == NULL) || (*modules == '\0'))
		return(0);

	newlist.sl_modlist = newmods;
	newlist.sl_nmods = NSTRPUSH;
	(void)strcpy(modp, modules);
	/*
	 * pull mod names out of comma-separated list
	 */
	for ( i = 0, modp = strtok(modp, ",");
	modp != NULL; i++, modp = strtok(NULL, ",") ) {
		if ( i >= NSTRPUSH) {
			/* too many modules */
			return(2);
		}
		(void)strncpy(newlist.sl_modlist[i].l_name,
					modp, FMNAMESZ);
	}
	newlist.sl_nmods = i;

	/*
	 * Is it a valid list of modules?
	 */
	if ((fd = open(USERDEV, O_RDWR)) == -1) {
		if (errno == EBUSY) {
			/* can't validate module list, /dev/sad/user busy */
			return(0);
		}
		/* open /dev/sad/user failed */
		return(3);
	}
	if ( (i = ioctl(fd, SAD_VML, &newlist)) < 0 ) {
		/* SAD_VML ioctl failed */
		(void)close(fd);
		return(4);
	}
	if ( i != 0 ) {
		/* invalid STREAMS module list */
		(void)close(fd);
		return(1);
	}
	(void)close(fd);
	return(0);
}
