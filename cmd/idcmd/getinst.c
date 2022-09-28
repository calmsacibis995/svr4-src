/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:getinst.c	1.3"

/* Get an entry from
 * type = MDEV: Master device file. This file will contain the devices for
 *		all INSTALLed Driver Software Packages (DSPs).
 * type = MTUN: Master tunable parameter file. This file will contain the
 *		tunable parameter file for all INSTALLed DSPs.
 * type = BDEV: Master device file used to BUILD the Kernel.
 * type = BTUN: Master tunable parameter file used to BUILD the Kernel.
 * type = SDEV: System device file used to BUILD the Kernel.
 * type = STUN: System tunable file used to BUILD the Kernel.
 * type = SASN: System assign file used to BUILD the Kernel.
 * The entry is
 * name = FIRST: first entry.
 * name = NEXT: next entry.
 * name = RESET: reset file pointer to beginning of file.
 * name = device/parameter name.
 * P points to an mdev, mtun, sdev, stun, sasn stucture.
 * Return  0 if EOF or can not locate device.
 *	  -1 if can not open file.
 *	  -2 if can not get major number(s).
 *	   1 if success.
 * Calling functions can tell getinst the location of the Master
 * and System files by:
 * - declaring "extern char pathinst[];".
 * - writing a string to pathinst.
 */

#include <stdio.h>
#include <ctype.h>
#include "inst.h"

static line(), same();
static char buf[100];
char pathinst[80] = "";		/* directory containing Master and System files */

FILE *md = NULL,		/* Master device file */
     *mt = NULL,		/* Master tunable file */
     *bd = NULL,		/* booted Master device file */
     *bt = NULL,		/* booted Master tunable file */
     *sd = NULL,		/* booted System device file */
     *st = NULL,		/* booted System tunable file */
     *sa = NULL;		/* booted System assign file */

extern int getmajors();

getinst( type, name, p )
short type, *p;
char *name;
{
	
	struct multmaj mm;	/* used for multiple majors */
	short start, end;	/* start and end range for multiple majors */

	switch (type) {
	case MDEV:
		if (md == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, INSTALL, MDEVICE);
			else
				sprintf(buf, "%s/%s", pathinst, MDEVICE);
			if ((md = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(md, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(md) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(md) == 0)
					return(0);
		sscanf(buf, "%s %s %s %s %s %s %hd %hd %hd",
			((struct mdev *) p)->device,
			((struct mdev *) p)->mask,
			((struct mdev *) p)->type,
			((struct mdev *) p)->handler,
			mm.brange,
			mm.crange,
			&((struct mdev *) p)->min,
			&((struct mdev *) p)->max,
			&((struct mdev *) p)->chan);

		/* convert the major numbers, read as a string         */
		/* into a number (if single major) or a start/end pair */
		/* if multiple majors are specified.                   */

		if (getmajors(mm.brange, &start, &end)) {
			if (start != end && (strchr(((struct mdev *)p)->type, 'M') == NULL)) {
				fprintf(stderr, "Multiple major range specified for %s device but no 'M' in mdevice type field\n", ((struct mdev *)p)->device);
				return(0);
			}
			if (start == end)
				((struct mdev *)p)->blk = start;
			((struct mdev *)p)->blk_start = start;
			((struct mdev *)p)->blk_end   = end;
		}
		else
			return(-2);

		if (getmajors(mm.crange, &start, &end)) {
			if (start != end && (strchr(((struct mdev *)p)->type, 'M') == NULL)) {
				fprintf(stderr, "Multiple major range specified for %s device but no 'M' in mdevice type field\n", ((struct mdev *)p)->device);
				return(0);
			}
			if (start == end)
				((struct mdev *)p)->chr = start;
			((struct mdev *)p)->char_start = start;
			((struct mdev *)p)->char_end   = end;
		}
		else
			return(-2);

		return(1);
	case MTUN:
		if (mt == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, INSTALL, MTUNE);
			else
				sprintf(buf, "%s/%s", pathinst, MTUNE);
			if ((mt = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(mt, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(mt) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(mt) == 0)
					return(0);
		sscanf(buf, "%s %ld %ld %ld",
			((struct mtun *) p)->oudef,
			&((struct mtun *) p)->def,
			&((struct mtun *) p)->min,
			&((struct mtun *) p)->max);
		return(1);
	case BDEV:
		if (bd == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, BUILD, MDEVICE);
			else
				sprintf(buf, "%s/%s", pathinst, MDEVICE);
			if ((bd = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(bd, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(bd) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(bd) == 0)
					return(0);
		sscanf(buf, "%s %s %s %s %s %s %hd %hd %hd",
			((struct mdev *) p)->device,
			((struct mdev *) p)->mask,
			((struct mdev *) p)->type,
			((struct mdev *) p)->handler,
			mm.brange,
			mm.crange,
			&((struct mdev *) p)->min,
			&((struct mdev *) p)->max,
			&((struct mdev *) p)->chan);

		/* convert the major numbers, read as a string         */
		/* into a number (if single major) or a start/end pair */
		/* if multiple majors are specified.                   */

		if (getmajors(mm.brange, &start, &end)) {
			if (start != end && (strchr(((struct mdev *)p)->type, 'M') == NULL)) {
				fprintf(stderr, "Multiple major range specified for %s device but no 'M' in mdevice type field\n", ((struct mdev *)p)->device);
				return(0);
			}
			if (start == end)
				((struct mdev *)p)->blk = start;
			((struct mdev *)p)->blk_start = start;
			((struct mdev *)p)->blk_end   = end;
		}
		else
			return(-2);

		if (getmajors(mm.crange, &start, &end)) {
			if (start != end && (strchr(((struct mdev *)p)->type, 'M') == NULL)) {
				fprintf(stderr, "Multiple major range specified for %s device but no 'M' in mdevice type field\n", ((struct mdev *)p)->device);
				return(0);
			}
			if (start == end)
				((struct mdev *)p)->chr = start;
			((struct mdev *)p)->char_start = start;
			((struct mdev *)p)->char_end   = end;
		}
		else
			return(-2);

		return(1);
	case BTUN:
		if (bt == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, BUILD, MTUNE);
			else
				sprintf(buf, "%s/%s", pathinst, MTUNE);
			if ((bt = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(bt, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(bt) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(bt) == 0)
					return(0);
		sscanf(buf, "%s %ld %ld %ld",
			((struct mtun *) p)->oudef,
			&((struct mtun *) p)->def,
			&((struct mtun *) p)->min,
			&((struct mtun *) p)->max);
		return(1);
	case SDEV:
		if (sd == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, BUILD, SDEVICE);
			else
				sprintf(buf, "%s/%s", pathinst, SDEVICE);
			if ((sd = fopen(buf,"r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(sd, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(sd) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(sd) == 0)
					return(0);
			sscanf(buf, "%s %c %hd %hd %hd %hd %lx %lx %lx %lx",
				((struct sdev *) p)->device,
				&((struct sdev *) p)->conf,
				&((struct sdev *) p)->units,
				&((struct sdev *) p)->ipl,
				&((struct sdev *) p)->type,
				&((struct sdev *) p)->vector,
				&((struct sdev *) p)->sioa,
				&((struct sdev *) p)->eioa,
				&((struct sdev *) p)->scma,
				&((struct sdev *) p)->ecma);
		return(1);
	case STUN:
		if (st == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, BUILD, STUNE);
			else
				sprintf(buf, "%s/%s", pathinst, STUNE);
			if ((st = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(st, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(st) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(st) == 0)
					return(0);
		sscanf(buf, "%s %ld",
			((struct stun *) p)->name,
			&((struct stun *) p)->value);
		return(1);
	case SASN:
		if (sa == NULL) {
			if (*pathinst == NULL)
				sprintf(buf, "%s/%s/%s", ROOT, BUILD, SASSIGN);
			else
				sprintf(buf, "%s/%s", pathinst, SASSIGN);
			if ((sa = fopen(buf, "r")) == NULL)
				return(-1);
		}
		if (*name != *NEXT) {
			fseek(sa, 0L, 0);
			if (*name == *RESET)
				return(1);
		}
		if (line(sa) == 0)
			return(0);
		if (*name != *FIRST && *name != *NEXT)
			while (!same(name))
				if (line(sa) == 0)
					return(0);
		sscanf(buf, "%s %s %hd %ld %hd",
			((struct sasn *) p)->device,
			((struct sasn *) p)->major,
			&((struct sasn *) p)->minor,
			&((struct sasn *) p)->low,
			&((struct sasn *) p)->blocks);
		return(1);
	}
}

/* Read a line. Skip lines beginning with '*'.
 * Return 0 if EOF. Return 1 otherwise.
 */
static
line(fp)
FILE *fp;
{
	for (;;) {
		if (fgets(buf, 100, fp) == NULL)
			return(0);
		if (*buf != '*')
			return(1);
	}
}

/* Check if 'name' is the same string that begins in column 1 of 'buf'.
 * 'Name' must be null terminated. The first field of 'buf' which is being
 * compared must be followed by white space (this doesn't include '\0').
 * Return 1 if field 1 of 'buf' matches name, and 0 otherwise.
 */
static
same(name)
char *name;
{
	char *b;

	for (b = buf; !isspace(*b) && *name != NULL; b++, name++)
		if (*b != *name)
			return(0);
	if (isspace(*b) && *name == '\0')
		return(1);
	return(0);
}
