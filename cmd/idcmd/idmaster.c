/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idmaster.c	1.3"

/* Add or update 'mdevice' with the contents of 'Master'
 * or delete an entry from 'mdevice'.
 * Update refers to replacing the
 * device entry but keeping the major numbers the same.
 *
 * exit codes
 * ----------
 * 0		- success
 * 1		- error
 *
 * options
 * -------
 * -d device	- delete
 * -a		- add
 * -u		- update
 * -i dir	- input directory containing 'Master'
 * -o dir	- output directory containing 'mdevice'
 *
 * Comments
 * --------
 * Both 'Master' and 'mdevice' may have blank lines or lines beginning
 * with an '*'. The contents of 'mdevice' are always preserved.
 * Only the first non-comment line of 'Master' is added to 'mdevice'
 * (i.e. comment lines are not added).
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "inst.h"
#include "defines.h"

/* operations */
#define	ADD		1
#define	DELETE  	2
#define UDATE		3  /* UPDATE */

/* error messages */
#define MUSAGE		"Usage: idmaster (-a)|(-u)|(-d dev) [-i dir] [-o dir]\n"
#define DEVFILE		"incorrect number of entries in device Master file"
#define MOPEN		"can not open '%s' for mode '%s'"
#define NOUPDATE	"device does not exist and therefore can not be updated"
#define NODELETE	"device does not exist and therefore can not be deleted"
#define DUPLICATE	"device already exists in mdevice"
#define BMAJOR		"there are no free block major numbers available"
#define CMAJOR		"there are no free character major numbers available"
#define GETINST		"can not locate or open 'mdevice'"
#define MEMPTY		"Master file does not contain non-blank entry"
#define GETMAJOR	"Error encountered getting major number(s) for %s mdevice entry"
#define UNIQUE		"'u' flag is used but block and char major ranges are not the same size"
#define BUNLINK		"Cannot unlink old version of <%s>: '%s'"
#define BLINK		"Cannot link new version of <%s>: '%s'.\nNew mdevice in <%s>"

#define TRUE		1
#define FALSE		0

/* for multiple majors */
#define CH		1
#define BL		2

int bdevices[BLSIZE];	/* bdevices[n] = 1 if blk major # n is used */
int cdevices[CSIZE];	/* cdevices[n] = 1 if chr major # n is used */
struct mdev st_mast;	/* store contents of Master */
struct mdev st_mdev;	/* store contents of mdevice */
struct multmaj mm;	/* temporary storage for multiple major number range */
char master[90];	/* path name of new Master file to be added */
char mdevice[90];	/* path name of mdevice in which Master will be appended */
char temp[128];		/* path name of temporary mdevice file */
char *root = ROOT;	/* path name of root directory for ID/TP */
char device[15];	/* name of device being removed or updated */
char buf[100];		/* store input lines */

/* flags */
int tmpexist = 0;	/* temp file name created, possibly opened */
int debug;		/* debug flag */
int iflag;		/* -i flag specified */
int oflag;		/* -o option specified */
int flag;		/* indicates add, remove, update */
int found = FALSE;	/* has mdevice entry been found */

FILE *open1();
extern char *optarg;
extern char pathinst[];	/* path name for Master & System files used by getinst */

extern int errno;
extern char *sys_errlist[];


main(argc, argv)
int argc;
char *argv[];
{
	int m, blk, chr;
	FILE *fin, *fout;
	int blk_start, char_start;
	int howmany_b, howmany_c;

	while ((m = getopt(argc, argv, "?#ad:ui:o:")) != EOF)
		switch (m) {
		case 'd':
			if (flag > 0)
				error(MUSAGE);
			flag = DELETE;
			strcpy(device, optarg);
			break;
		case 'a':
			if (flag > 0)
				error(MUSAGE);
			flag = ADD;
			break;
		case 'u':
			if (flag > 0)
				error(MUSAGE);
			flag = UDATE;
			break;
		case '#':
			debug++;
			break;
		case 'i':
			iflag++;
			sprintf(master, "%s/%s", optarg, MASTER);
			break;
		case 'o':
			oflag++;
			sprintf(mdevice, "%s/%s", optarg, MDEVICE);
			sprintf(pathinst, "%s", optarg);
			sprintf(temp, "%s/ID%d", optarg, getpid());
			break;
		case '?':
			fprintf(stderr, MUSAGE);
			exit(1);
		}

	if (flag == 0)
		error(MUSAGE);

	/* get input and output path names */
	if (!iflag)
		sprintf(master, "./%s", MASTER);
	if (!oflag) {
		/* construct real mdevice file */
		sprintf(mdevice, "%s/%s/%s", root, INSTALL, MDEVICE);
		/* construct temporary mdevice file */
		sprintf(buf, "%s/%s", root, INSTALL);
		sprintf(temp, "%s/ID%d", buf, getpid());
	}

	if (debug)
		fprintf(stderr, "input = %s\noutput = %s\ntemp = %s\n",
			master, mdevice, temp);

	switch (flag) {
	case ADD:
		/* read new Master device file */
		getmaster();

		/* assign major device numbers */
		if (INSTRING(st_mast.type, MULTMAJ)) {
			find_mm_major(st_mast.device, &blk_start, &char_start,
				&howmany_b, &howmany_c);
			mm_assign(blk_start, char_start, howmany_b, howmany_c);
		}
		else {
			findmajor(st_mast.device, &blk, &chr);
			assign(blk, chr);
		}

		/* add new device entry to mdevice */
		fout = open1(mdevice, "a");
		wrtmdev(&st_mast, fout);
		fclose(fout);
		break;

	case UDATE:
		getmaster();
		strcpy(device, st_mast.device);
		/* fall through */

	case DELETE:
		/* temp file name created and about to be opened */
		tmpexist = 1;

		fout = open1(temp, "w");
		fin = open1(mdevice, "r");

		while (fgets(buf, 100, fin) != NULL) {

			/* pass comments and blank lines */
			if (*buf == '*' || blank()) {
				fputs(buf, fout);
				continue;
			}

			/* read tokens into structure */
			rdmdev(buf, &st_mdev);

			/* write out if not the same */
			if (strcmp(device, st_mdev.device))
				wrtmdev(&st_mdev, fout);

			/* if the same & update, add new entry */
			else {
				found = TRUE;

				if (flag == UDATE) {
					if(!(INSTRING(st_mast.type, MULTMAJ)) &&
					   !(INSTRING(st_mdev.type, MULTMAJ))) {
						/* reuse old majors */

						findmajor("", &blk, &chr);
						if (strchr(st_mdev.type, 'b') != NULL)
							blk = st_mdev.blk;
						if (strchr(st_mdev.type, 'c') != NULL)
							chr = st_mdev.chr;

						/* assign old major numbers */
						assign(blk, chr);
					}
					if((INSTRING(st_mast.type, MULTMAJ)) ||
					   (INSTRING(st_mdev.type, MULTMAJ))) {
						find_mm_major(st_mast.device, &blk_start,
						&char_start,&howmany_b,&howmany_c);
						mm_assign(blk_start, char_start,
							howmany_b, howmany_c);
					}

					/* write new Master entry to mdevice */
					wrtmdev(&st_mast, fout);
				}
			}
		}
		fclose(fin);
		fclose(fout);

		if (!found)
			if (flag == UDATE)
				error(NOUPDATE);
			else
				error(NODELETE);

		/* erase old mdevice and rename new one */
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);

		if (unlink(mdevice) < 0) {
			sprintf(buf, BUNLINK, mdevice, sys_errlist[errno]);
			error(buf);
		}
		if (link(temp, mdevice) < 0) {
			sprintf(buf, BLINK, mdevice, sys_errlist[errno], temp);
			error(buf);
		}
		unlink(temp);
		break;
	}

	exit(0);
}


/* read new Master device file */

getmaster()
{
	FILE *fp;

	fp = open1(master, "r");

	while (fgets(buf, 100, fp) != NULL) {

		if (debug)
			fprintf(stderr, "Master='%s'\n", buf);

		/* pass comments and blank lines */
		if (*buf == '*' || blank())
			continue;

		if (rdmdev(buf, &st_mast) != 9)
			error(DEVFILE);


		fclose(fp);
		return;
	}

	error(MEMPTY);
}


/* assign major numbers to device */
assign(blk, chr)
int blk, chr;
{
	if (strchr(st_mast.type, 'b') != NULL)
		st_mast.blk = blk;
	else
		st_mast.blk = 0;
	if (strchr(st_mast.type, 'c') != NULL)
		st_mast.chr = chr;
	else
		st_mast.chr = 0;
}

mm_assign(b_start, c_start, howmany_bl, howmany_ch)
int b_start, c_start;
int howmany_bl, howmany_ch;
{
	if (strchr(st_mast.type, 'b') != NULL) {
		st_mast.blk_start = b_start;
		st_mast.blk_end = st_mast.blk_start + (howmany_bl - 1);
		if (howmany_bl == 1)
			st_mast.blk = st_mast.blk_start;
	}
	else
		st_mast.blk = st_mast.blk_start = st_mast.blk_end = 0;

	if (strchr(st_mast.type, 'c') != NULL) {
		st_mast.char_start = c_start;
		st_mast.char_end = st_mast.char_start + (howmany_ch - 1);
		if (howmany_ch == 1)
			st_mast.chr = st_mast.char_start;
	}
	else
		st_mast.chr = st_mast.char_start = st_mast.char_end = 0;

}

/* find highest major number in mdevice */

findmajor(name, b, c)
char *name;
int *b, *c;
{
	register int i;
	struct mdev d;
	int count_b, count_c;
	int retvalue;

	if (getinst(MDEV, RESET, 0) == -1)
		error(GETINST);

	/* collect block and character major numbers */
	while ((retvalue = getinst(MDEV, NEXT, &d)) > 0) {
		if (!strcmp(d.device, name))
			error(DUPLICATE);
		/* find out how many char majors the mdevice entry has */

		if (strchr(d.type, 'c') != NULL) {
			count_c = d.char_end - d.char_start + 1;
			for(i = 0; i < count_c; i++)
				cdevices[d.char_start + i]++;
		}

		/* find out how many block majors this entry has */
		if (strchr(d.type, 'b') != NULL) {
			count_b = d.blk_end - d.blk_start + 1;
			for(i = 0; i < count_b; i++)
				bdevices[d.blk_start + i]++;
		}
	}
	if (retvalue == -2) {
		sprintf(buf, GETMAJOR, d.device);
		error(buf);
	}

	/* if same (unique) block and char major numbers are required, */
	/* try to find the same free numbers in both tables.         */
	if((strchr(st_mast.type, 'u') != NULL) &&
		(strchr(st_mast.type, 'c') != NULL) &&
		(strchr(st_mast.type, 'b') != NULL)) {
			for (i = 0; i < CSIZE; i++) {
				if (cdevices[i] == 0) {
					if (i < BLSIZE && bdevices[i] == 0) {
						*b = *c = i;
						break;
					}
					else if (i >= BLSIZE)
						error(BMAJOR);
				}
			}
			if (i == CSIZE)
				error(CMAJOR);
	}
	else {
	
		/* find first free block major number */
		for (i = 0; i < BLSIZE; i++)
			if (bdevices[i] == 0) {
				*b = i;
				break;
			}
		if (i == BLSIZE)
			error(BMAJOR);
		
		/* find first free character major number */
		for (i = 0; i < CSIZE; i++)
			if (cdevices[i] == 0) {
				*c = i;
				break;
			}
		if (i == CSIZE)
			error(CMAJOR);
	}
	
	if (debug)
		fprintf(stderr, "findmajor: device=%s b=%d c=%d\n", name, *b, *c);
}

int getnumber();

/* find multiple majors to assign to device */

find_mm_major(name, b, c, howmany_bl, howmany_ch)
char *name;
int *b, *c;
int *howmany_bl, *howmany_ch;
{
	register int i;
	struct mdev d;
	int count_b, count_c;
	int retvalue;

	if (getinst(MDEV, RESET, 0) == -1)
		error(GETINST);

	/* collect block and character major numbers */
	while ((retvalue = getinst(MDEV, NEXT, &d)) > 0) {
		if (flag == ADD)
			if (!strcmp(d.device, name))
				error(DUPLICATE);


		/* find out how many char majors the mdevice entry has */

		if (strchr(d.type, 'c') != NULL) {
			count_c = d.char_end - d.char_start + 1;
			/* if UPDATE re-use or free-up char major numbers */
			if (strcmp(d.device,name))
				for(i = 0; i < count_c; i++)
					cdevices[d.char_start + i]++;
		}

		/* find out how many block majors this entry has */
		if (strchr(d.type, 'b') != NULL) {
			count_b = d.blk_end - d.blk_start + 1;
			/* if UPDATE re-use or free-up block major numbers */
			if (strcmp(d.device,name))
				for(i = 0; i < count_b; i++)
					bdevices[d.blk_start + i]++;
		}
	}

	if (retvalue == -2) {
		sprintf(buf, GETMAJOR, d.device);
		error(buf);
	}

		/* find first free block major in set of n, where n >= 1 */

		{
		int save_bl, save_ch;
		static match = 0;

		*howmany_ch = st_mast.char_end - st_mast.char_start + 1;
		*howmany_bl = st_mast.blk_end - st_mast.blk_start + 1;

		/* if device requires unique block and character major  */
		/* ranges, then try to match the starting range numbers */
		if((strchr(st_mast.type, 'u') != NULL) &&
			(strchr(st_mast.type, 'c') != NULL) &&
			(strchr(st_mast.type, 'b') != NULL)) {
			if (*howmany_ch != *howmany_bl)
				error(UNIQUE);
			save_ch = 0;
			while (!match) {
				save_ch = getnumber(cdevices, save_ch,
					*howmany_ch, CSIZE, CH);
				save_bl = getnumber(bdevices, save_ch,
					*howmany_bl, BLSIZE, BL);
				if (save_ch == save_bl)
					match++;
				else
					save_ch = save_bl;
			}
			if (match)
				*b = *c = save_ch;
		}
		else {
			if(strchr(st_mast.type, 'b') != NULL) {
				save_bl = 0;
				save_bl = getnumber(bdevices, save_bl,
					*howmany_bl, BLSIZE, BL);
				*b = save_bl;
			}

			if(strchr(st_mast.type, 'c') != NULL) {
				save_ch = 0;
				save_ch = getnumber(cdevices, save_ch,
					*howmany_ch, CSIZE, CH);
				*c = save_ch;
			}
		}
	}
	if (debug)
		fprintf(stderr, "findmajor: device=%s b=%d c=%d\n", name, *b, *c);
}

getnumber(table, start, rqsize, tsize, typeflag)
int *table;
int start;
int rqsize, tsize;
int typeflag;
{
	register i, j;
	int count;
	int ret_val;

	for (i = start; i < tsize; i++) {
		if (table[i] == 0) {
			ret_val = i;
			/* find the remaining rqsize - 1 majors */
			for (j = i+1, count = 1; count < rqsize && j < tsize;
				j++, count++) {
				if (table[j] != 0)
					break;
			}
			if (j == tsize) {
				if (typeflag == CH)
					error(CMAJOR);
				else
					error(BMAJOR);
			}
			if (count == rqsize)
				break;
			i = j;	/* reset for loop index variable */
		}
	}
	if (i == tsize || (ret_val + rqsize - 1) > (tsize - 1)) {
		if (typeflag == CH)
			error(CMAJOR);
		else
			error(BMAJOR);
	}
	return(ret_val);
}

/* Return 1 if line contains only white space and 0 otherwise. */

blank()
{
	char *p;

	for (p = buf; *p != NULL; p++)
		if (*p != ' ' && *p != '\t' && *p != '\n')
			return(0);
	return(1);
}



/* open a file */

FILE *
open1(file, mode)
char *file, *mode;
{
	FILE *fp;

	if (debug)
		fprintf(stderr, "open '%s' for mode '%s'\n", file, mode);

	if ((fp = fopen(file, mode)) == NULL) {
		sprintf(buf, MOPEN, file, mode);
		error(buf);
	}
	return(fp);
}



void checkflags(s, type)
char *s;
int type;
{
	char *ptr;
	char *strchr();

	for ( ; *s != NULL; s++) {
		if (type == MASK) {
			if ((ptr = strchr(MASK_FLAGS, *s)) == NULL) {
				sprintf(buf, MASK_ERROR, *s);
				error(buf);
			}
		}
		else if (type == TYPE) {
			if ((ptr = strchr(TYPE_FLAGS, *s)) == NULL) {
				sprintf(buf, TYPE_ERROR, *s);
				error(buf);
			}
		}
	}
}


/* read buf into mdev structure */

rdmdev(buf, d)
char *buf;
struct mdev *d;
{
	int n;
	short start, end;

	n = sscanf(buf, "%14s %9s %19s %6s %19s %19s %hd %hd %hd",
		d->device, d->mask, d->type, d->handler,
		mm.brange, mm.crange, &d->min, &d->max, &d->chan);

	/* verify that the flags specified in the second and third fields
	 * of Master file are valid according to defines.h.
	 */

	checkflags(d->mask, MASK);
	checkflags(d->type, TYPE);

	if(getmajors(mm.brange, &start, &end)) {
		if (start == end)
			d->blk = start;
		if((start != end) && !(INSTRING(d->type, MULTMAJ))) {
			sprintf(buf, MMERROR, "block");
			error(buf);
		}
		d->blk_start = start;
		d->blk_end = end;
	}
	else
		error("getmajors failed: called from rdmdev (brange) %s",d->device);

	if(getmajors(mm.crange, &start, &end)) {
		if(start == end)
			d->chr = start;
		if((start != end) && !(INSTRING(d->type, MULTMAJ))) {
			sprintf(buf, MMERROR, "char");
			error(buf);
		}
		d->char_start = start;
		d->char_end = end;
	}
	else
		error("getmajors failed: called from rdmdev (crange) %s",d->device);

	return(n);
}


/* write mdev structure to file */

wrtmdev(d, fp)
struct mdev *d;
FILE *fp;
{
	short fblk = 0, fchar = 0;

	if (INSTRING(d->type, MULTMAJ)) {
		if(d->blk_start != d->blk_end)
			fblk++;
		if(d->char_start != d->char_end)
			fchar++;

		if(fblk && !fchar)
			fprintf(fp,"%s\t%s\t%s\t%s\t%hd-%hd\t%hd\t%hd\t%hd\t%hd\n",
				d->device, d->mask, d->type, d->handler,
				d->blk_start, d->blk_end, d->chr,
				d->min, d->max, d->chan);
		else if(!fblk && fchar)
			fprintf(fp,"%s\t%s\t%s\t%s\t%hd\t%hd-%hd\t%hd\t%hd\t%hd\n",
				d->device, d->mask, d->type, d->handler,
				d->blk, d->char_start,d->char_end,
				d->min, d->max, d->chan);
		else if(fblk && fchar)
			fprintf(fp,"%s\t%s\t%s\t%s\t%hd-%hd\t%hd-%hd\t%hd\t%hd\t%hd\n",
				d->device, d->mask, d->type, d->handler,
				d->blk_start, d->blk_end, d->char_start,d->char_end,
				d->min, d->max, d->chan);
		else

			fprintf(fp, "%s\t%s\t%s\t%s\t%hd\t%hd\t%hd\t%hd\t%hd\n",
				d->device, d->mask, d->type, d->handler,
				d->blk, d->chr, d->min, d->max, d->chan);
	}
	else
		fprintf(fp, "%s\t%s\t%s\t%s\t%hd\t%hd\t%hd\t%hd\t%hd\n",
			d->device, d->mask, d->type, d->handler,
			d->blk, d->chr, d->min, d->max, d->chan);
}


/* print error message */

error(msg)
char *msg;
{
	/* check if temporary file created and erase */
	if (tmpexist)
		unlink(temp);

	fprintf(stderr, "idmaster: %s\n", msg);

	exit(1);
}
