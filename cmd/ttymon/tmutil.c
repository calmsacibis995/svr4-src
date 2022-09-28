/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmutil.c	1.10.4.1"

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
# include "ttymon.h"
# include "tmstruct.h"

#define	NSTRPUSH	9	/* should agree with the tunable in	*/
				/* 		/etc/master.d/kernel	*/

extern	char	Scratch[];
extern	void	log();

/*
 *	check_device - check to see if the device exists,
 *		     - and if it is a character device
 *		     - return 0 if everything is ok. Otherwise, return -1
 */

check_device(device)
char	*device;
{
	struct stat statbuf;

	if ((device == NULL) || (*device == '\0')) {
		log("error -- device field is missing");
		return(-1);
	}
	if (*device != '/') {
		(void)sprintf(Scratch,
		"error -- must specify full path name for \"%s\".", device);
		log(Scratch);
		return(-1);
	}
	if (access(device, 0) == 0) {
		if (stat(device,&statbuf) < 0) {
			(void)sprintf(Scratch,"stat(%s) failed.", device);
			log(Scratch);
			return(-1);
		}
		if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
			(void)sprintf(Scratch, "error -- \"%s\" not character special device",device);
			log(Scratch);
			return(-1);
		}
	}
	else {
		(void)sprintf(Scratch, "error -- device \"%s\" does not exist",device);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 *	check_cmd - check to see if the cmd file exists,
 *		  - and if it is executable
 *		  - return 0 if everything is ok. Otherwise, return -1
 */

check_cmd(cmd)
char	*cmd;
{
	struct stat statbuf;
	char	tbuf[BUFSIZ];
	char	*tp = tbuf;

	if ((cmd == NULL) || (*cmd == '\0')) {
		log("error -- server command is missing");
		return(-1);
	}
	(void)strcpy(tp,cmd);
	(void)strtok(tp, " \t");
	if (*tp != '/') {
		(void)sprintf(Scratch, 
		"error -- must specify full path name for \"%s\".", tp);
		log(Scratch);
		return(-1);
	}
	if (access(tp, 0) == 0) {
		if (stat(tp,&statbuf) < 0) {
			(void)sprintf(Scratch,"stat(%s) failed.", tp);
			log(Scratch);
			return(-1);
		}
		if (!(statbuf.st_mode & 0111)) {
			(void)sprintf(Scratch, "error -- \"%s\" not executable\n",tp);
			log(Scratch);
			return(-1);
		}
		if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
			(void)sprintf(Scratch, "error -- \"%s\" not a regular file",tp);
			log(Scratch);
			return(-1);
		}
	}
	else {
		(void)sprintf(Scratch, "error -- \"%s\" does not exist",tp);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 * strcheck(sp, flag)	- check string
 *				- if flag == ALNUM, all char. are expected to
 *				  be alphanumeric
 *				- if flag == NUM, all char. are expected to
 *				  be digits and the number must be >= 0
 *				- return 0 if successful, -1 if failed.
 */
int
strcheck(sp, flag)
char	*sp;		/* string ptr		*/
int	flag;		/* either NUM or ALNUM	*/
{
	register	char	*cp;
	if (flag == NUM) {
		for (cp = sp; *cp; cp++) {
			if (!isdigit(*cp)) {
				return(-1);
			}
		}
	}
	else {	/* (flag == ALNUM) */ 
		for (cp = sp; *cp; cp++) {
			if (!isalnum(*cp)) {
				return(-1);
			}
		}
	}
	return(0);
}

/*
 * vml(modules)	- validate a list of modules
 *		- return 0 if successful, -1 if failed
 */
int
vml(modules)
char	*modules;
{
	char	buf[BUFSIZ];
	char	*modp = buf;
	int	i, fd;
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
			(void)sprintf(Scratch,
			"too many modules in <%s>", modules);
			return(-1);
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
			log("Warning - can't validate module list, /dev/sad/user busy");
			return(0);
		}
		(void)sprintf(Scratch,"open /dev/sad/user failed, errno = %d",errno);
		log(Scratch);
		return(-1);
	}
	if ( (i = ioctl(fd, SAD_VML, &newlist)) < 0 ) {
		(void)sprintf(Scratch,
			"Validate modules ioctl failed, modules = <%s>, errno = %d", 
			modules, errno);
		log(Scratch);
		(void)close(fd);
		return(-1);
	}
	if ( i != 0 ) {
		(void)sprintf(Scratch,
			"Error - invalid STREAMS module list <%s>.", 
			modules);
		log(Scratch);
		(void)close(fd);
		return(-1);
	}
	(void)close(fd);
	return(0);
}

/*
 * copystr(s1, s2) - copy string s2 to string s1
 *		   - also put '\' in front of ':'
 */
void
copystr(s1,s2)
char	*s1, *s2;
{
	while (*s2) {
		if (*s2 == ':') {
			*s1++ = '\\';
		}
		*s1++ = *s2++;
	}
	*s1 = '\0';
}
