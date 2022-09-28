/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:ypserv_ancil.c	1.5.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static	char sccsid[] = "@(#)ypserv_ancil.c 1.13 88/02/08 Copyr 1984 Sun Micro";
#endif

#include <dirent.h>
#include "ypsym.h"
#include "ypdefs.h"
USE_YPDBPATH
USE_DBM

static bool onmaplist();
extern unsigned int strlen();
extern int strcmp();
extern int isvar_sysv();
extern char *strncpy();
extern int yp_getkey();

/*
 * This constructs a file name from a passed domain name, a passed map name,
 * and a globally known YP data base path prefix.
 */
void
ypmkfilename(domain, map, path)
	char *domain;
	char *map;
	char *path;
{
	if ( ((int)strlen(domain) + (int)strlen(map) + ypdbpath_sz + 3) 
	    > (MAXNAMLEN + 1) ) {
		fprintf(stderr, "ypserv:  Map name string too long.\n");
	}

	strcpy(path, ypdbpath);
	strcat(path, "/");
	strcat(path, domain);
	strcat(path, "/");
	strcat(path, map);
}

/*
 * This checks to see whether a domain name is present at the local node as a
 *  subdirectory of ypdbpath
 */
bool
ypcheck_domain(domain)
	char *domain;
{
	char path[MAXNAMLEN + 1];
	struct stat filestat;
	bool present = FALSE;

	strcpy(path, ypdbpath);
	strcat(path, "/");
	strcat(path, domain);

	if (stat(path, &filestat) != -1) {
		if ( (filestat.st_mode & S_IFDIR))
				present = TRUE;
		}
	return(present);
}

/*
 * This generates a list of the maps in a domain.
 */
int
yplist_maps(domain, list)
	char *domain;
	struct ypmaplist **list;
{
	DIR *dirp;
	struct dirent *dp;
	char domdir[MAXNAMLEN + 1];
	char path[MAXNAMLEN + 1];
	char map_key[YPMAXMAP + 1];
	int error;
	char *ext;
	struct ypmaplist *map;
	int namesz;

	*list = (struct ypmaplist *) NULL;

	if (!ypcheck_domain(domain) ) {
		return (YP_NODOM);
	}
	
	(void) strcpy(domdir, ypdbpath);
	(void) strcat(domdir, "/");
	(void) strcat(domdir, domain);

	if ( (dirp = opendir(domdir) ) == NULL) {
		return (YP_YPERR);
	}

	error = YP_TRUE;
	
	for (dp = readdir(dirp); error == YP_TRUE && dp != NULL;
	    dp = readdir(dirp) ) {
		/*
		 * If it''s possible that the file name is one of the two files
		 * implementing a map, remove the extension (dbm_pag or dbm_dir)
		 */
		namesz =  (int) strlen(dp->d_name);

		if (namesz < sizeof (dbm_pag) - 1)
			continue;		/* Too Short */

		ext = &(dp->d_name[namesz - (sizeof (dbm_pag) - 1)]);

		if (strcmp (ext, dbm_pag) != 0 )
			continue;		/* No dbm file extension */

		*ext = '\0';
		ypmkfilename(domain, dp->d_name, path);
		
		/*
		 * At this point, path holds the map file base name (no dbm
		 * file extension), and dp->d_name holds the map name.
		 */
		if (ypcheck_map_existence(path) &&
		    !onmaplist(dp->d_name, *list)) {

			if ((map = (struct ypmaplist *) malloc(
			    sizeof (struct ypmaplist)) ) == NULL) {
				error = YP_YPERR;
				break;
			}

			map->ypml_next = *list;
			*list = map;
			namesz = (int) strlen(dp->d_name);

			if (namesz <= YPMAXMAP) {
				if (yp_getkey(dp->d_name, map_key, MAXALIASLEN) < 0) {
fprintf(stderr,"yplist_maps: getkey failed for %s\n", dp->d_name);
					error = YP_YPERR;
					break;
				} else
					(void) strcpy(map->ypml_name, map_key);
			} else {
				if (yp_getkey(dp->d_name, map_key, MAXALIASLEN) < 0) {
fprintf(stderr,"yplist_maps: getkey failed for %s\n", dp->d_name);
					error = YP_YPERR;
					break;
				} else if (strcmp(dp->d_name, map_key) == 0) {
					(void) strncpy(map->ypml_name, dp->d_name,
					    (unsigned int) namesz);
					map->ypml_name[YPMAXMAP] = '\0';
				} else {
					(void) strcpy(map->ypml_name, map_key);
				}
			}
		}
	}
	
	closedir(dirp);
	return(error);
}
		
/*
 * This returns TRUE if map is on list, and FALSE otherwise.
 */
static bool
onmaplist(map, list)
	char *map;
	struct ypmaplist *list;
{
	struct ypmaplist *scan;

	for (scan = list; scan; scan = scan->ypml_next) {

		if (strcmp(map, scan->ypml_name) == 0) {
			return (TRUE);
		}
	}

	return (FALSE);
}
