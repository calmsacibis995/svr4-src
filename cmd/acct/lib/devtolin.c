/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:lib/devtolin.c	1.9.3.2"

/*
 *	convert device to linename (as in /dev/linename)
 *	return ptr to LSZ-byte string, "?" if not found
 *	device must be character device
 *	maintains small list in tlist structure for speed
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include "acctdef.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

static	tsize1;
static struct tlist {
	char	tname[LSZ];	/* linename */
	dev_t	tdev;		/* device */
} tl[TSIZE1];

char	*strncpy();
dev_t	lintodev();

static char dev_dir[] = "/dev";
static char *def_srch_dirs[] = { "/dev/term",
				 "/dev/pts",
				 "/dev/xt",
				 NULL };
char file_name[MAX_DEV_PATH];	/* name being returned */

char *
devtolin(device)
  dev_t device;
{
	register struct tlist *tp;
	char **srch_dirs;	/* priority directories to search first */
	int found = 0;
	int dirno = 0;

	for (tp = tl; tp < &tl[tsize1]; tp++)
		if (device == tp->tdev)
			return(tp->tname);

	srch_dirs = def_srch_dirs;
	
	while ((!found) && (srch_dirs[dirno] != NULL)) {
		/* if /dev is one of the priority directories we should only
		   search its top level for now (set depth = MAX_SEARCH_DEPTH) */

		found = srch_dir(device, srch_dirs[dirno],
				 ((strcmp(srch_dirs[dirno], dev_dir) == 0) ?
				  MAX_SRCH_DEPTH : 1),
				 NULL);
		dirno++;
	}

	/* if not yet found search remaining /dev directory skipping the
	   priority directories */

	if (!found)
		found = srch_dir(device, dev_dir, 0, srch_dirs);

	/* if found then put it (without the "/dev/" prefix) in the tlist
	   structure and return the path name without the "/dev/" prefix */
	
	if (found) {
		if (tsize1 < TSIZE1) {
			tp->tdev = device;
			CPYN(tp->tname, file_name+5);
			tsize1++;
		}
		return(file_name+5);
	} else
	/* if not found put "?" in the tlist structure for that device and
	   return "?" */

		if (tsize1 < TSIZE1) {
			tp->tdev = device;
			CPYN(tp->tname, "?");
			tsize1++;
		}
		return("?");

}

static int
srch_dir(device, path, depth, skip_dirs)
  dev_t device;	/* device we are looking for */
  char *path;	/* current path */
  int depth;	/* current depth */
  char *skip_dirs[];	/* directories that don't need searched */
{
	DIR *fdev;
	struct dirent *d;
	int dirno = 0;
	int found = 0;
	int path_len;
	char *last_comp;
	struct stat sb;

	/* do we need to search this directory? */

	if ((skip_dirs != NULL) && (depth != 0))
		while (skip_dirs[dirno] != NULL)
			if (strcmp(skip_dirs[dirno++], path) == 0)
				return(0);


	/* open the directory */

	if ((fdev = opendir(path)) == NULL)
		return(0);

	/* initialize file name using path name */

	path_len = strlen(path);
	strcpy(file_name, path);
	last_comp = file_name + path_len;
	*last_comp++ = '/';

	/* start searching this directory */

	while ((!found) && ((d = readdir(fdev)) != NULL))
		if (d->d_ino != 0) {

			/* if name would not be too long append it to
			   directory name, otherwise skip this entry */

			if ((path_len + strlen(d->d_name) + 2) > MAX_DEV_PATH)
				continue;
			else
				strcpy(last_comp, d->d_name);

			/* if this directory entry has the device number we need,
			   then the name is found.  Otherwise if it's a directory
			   (not . or ..) and we haven't gone too deep, recurse. */

			if (lintodev(file_name+5) == device) {
				found = 1;
				break;
			} else if ((depth < MAX_SRCH_DEPTH) &&
					  (strcmp(d->d_name, ".") != 0) &&
					  (strcmp(d->d_name, "..") != 0) &&
					  (stat(file_name, &sb) != -1) &&
					  ((sb.st_mode & S_IFMT) == S_IFDIR))
				found = srch_dir(device, file_name, depth+1, skip_dirs);
		}
	closedir(fdev);
	return(found);
}
