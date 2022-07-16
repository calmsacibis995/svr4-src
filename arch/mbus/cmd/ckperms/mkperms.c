/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/ckperms/mkperms.c	1.3"

static char mkperms_copyright[] = "Copyright 1986,1987,1988 Intel Corp. 463026";

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/errno.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<ar.h>
#include	<filehdr.h>
#include	"defs.h"

extern	char	*malloc ();
extern	int	errno;

static char     options[] = "i:C";
extern char	*optarg;
extern int	optind;

struct hlist {
	ushort	ino;
	int	nlink;
	dev_t	rdev;
	struct	perm *pm;
	struct 	hlist *hfw;
} *hlist [MAXINOD];

/* a simple hash function	*/

#define	hash (x) 	(x % MAXINOD)

#define	special "pbce"					

/*	flags for perms rec 	*/

#define		F	0x01
#define		T	0x02

		

/*	option flags	*/

int 	iflag,
	cflag,
	errflag = 0;

char	*iname, *rname;
int	err = 0;
struct	stat	stsbuf,*stbuf;	
char	path[BUFSIZE];

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'i':
			iflag++;
			iname = optarg;
			break;
		case 'C' :
			cflag++;
			break;
		default:
			errflag++;
			break;
		}
	}
	if (errflag || (argc != optind) || !iflag) {
		fprintf(stderr,"mkperm: Invalid options\n");
		giveusage ();
		exit (2);
	}
	mk (); 
		
}

giveusage ()
{
	fprintf (stderr, "Usage:  mkperms -i idname [-n pathname] [-C]\n");
}



mk ()
{
	int	i;
	struct	hlist *ptr;
	char	tsbuf[BUFSIZE];
	char	*tstr;
	
	stbuf = &stsbuf;	
	for (i=0; i<MAXINOD; i++)  { /* initialize hash table */
		if ((hlist[i] = (struct hlist *) malloc (sizeof (struct hlist))) == NULL)
			errmsg (FERR,  "Not enough memory", "");
		hlist[i]->hfw = NULL;
	}

	while  (fgets (tsbuf,BUFSIZE, stdin) != NULL) {
		if (strcmp (tsbuf,"") == NULL)
			errmsg (FERR, "Invalid Input", "");

		strcpy (path, tsbuf);
		tstr = path + strlen (path) - 1;
		*tstr = NULL;
		
		if (stat (path, stbuf) == -1) {
			errmsg (FERR, "cannot stat file", path);
		} else
			prfile ();

	}
/*
	all non null entries in the hash table implies that the link
	hasn't been found. Issue a warning and print what's left
*/

	for (i=0; i< MAXINOD; i++) { 
		for (ptr=hlist[i]->hfw; ptr != NULL; ptr= ptr->hfw) {
			errmsg (WARN,  "link count mismatch", ptr->pm->p_path[0]);
			permout (ptr);
		}
	}
		
}


errmsg (code, str, errpath)
int	code;
char 	*str;
char 	*errpath;
{
	if (code == WARN) 
		fprintf (stderr, "%s %s %s: %s\n", "mkperms:", "Warning", str, errpath);
	else {
		fprintf (stderr, "%s %s: %s\n","mkperms:", str, errpath);
		exit (1);
	}
	
}

/*
	calculates the check sum for a file by calling the "sum" utility
*/

docksum (ck1, ck2)
int	*ck1, *ck2;
{
	FILE	*fptr;
	char	tsbuf[BUFSIZE];
	struct	stat	spbuf;
	char	str[2*BUFSIZE];

	if (stat ("/bin/sum", &spbuf) == -1) {
		errmsg (WARN,  "/bin/sum not found", "");
		*ck1 = *ck2 = 0;
		return;
	}

	sprintf (str, "/bin/sum '%s'", path);

	if ((fptr = popen (str, "r")) != NULL) {
		if (fgets (tsbuf, BUFSIZE, fptr) != NULL) {
			if (strcmp (tsbuf,"") == NULL) {
				errmsg (WARN,  "Invalid sum of file", path);
				*ck1 = *ck2 = 0;
			}
			if (sscanf (tsbuf, "%d %d", ck1, ck2) == -1) {
				errmsg (WARN,  "Invalid sum of file", path);
				*ck1 = *ck2 = 0;
			}
		}
	}
	pclose (fptr);
}
/*
	routine to fill a hash record with all the relevant info.

*/

struct hlist *	
fillperm ()
{
	struct	hlist	*fw;
	int	ck1, ck2;
	
	if ((fw = (struct hlist *)malloc (sizeof(struct hlist))) == NULL)
		return (NULL);
	fw->ino = stbuf->st_ino;
	fw->nlink = stbuf->st_nlink;
	fw->rdev = stbuf->st_rdev;
	if ((stbuf->st_mode&S_IFMT) == S_IFDIR) 
		fw->nlink = 1;
	if ((fw->pm = (struct perm *)malloc (sizeof (struct perm))) == NULL) {
		(void) free (fw);
		return (NULL);
	}
	
	strcpy (fw->pm->p_id, iname);
	fw->pm->p_ftype = getft (stbuf->st_mode, path);
	fw->pm->p_perm = stbuf->st_mode&CP_PMASK;
	fw->pm->p_owner = stbuf->st_uid;
	fw->pm->p_group = stbuf->st_gid;
	fw->pm->p_sflag = fw->pm->p_cflag = F;
	if (fw->pm->p_ftype == 'r' && !stbuf->st_size)		
		fw->pm->p_ftype = 'e';
	if (!cflag || strchr (special, fw->pm->p_ftype) != NULL || fw->pm->p_ftype == 'd')
		fw->pm->p_sflag = T;
	else 
		fw->pm->p_size = stbuf->st_size;
	if (fw->pm->p_ftype == 'b' || fw->pm->p_ftype == 'c') {
		ck1 = major (stbuf->st_rdev);
		ck2 = minor (stbuf->st_rdev);
	} else {
		if (!cflag || fw->pm->p_ftype == 'p' || fw->pm->p_ftype == 'd' || fw->pm->p_ftype == 'e')		
			fw->pm->p_cflag = T;
		else 
			docksum (&ck1, &ck2);
	}
	if (fw->pm->p_cflag == F) {
		fw->pm->p_ck1_maj = ck1;
		fw->pm->p_ck2_min = ck2;
	}
	fw->pm->p_nolink = 0;
	if ((fw->pm->p_path[fw->pm->p_nolink] = malloc (BUFSIZE)) == NULL) {
		(void) free (fw->pm);
		(void) free (fw);
		return (NULL);
	}
	strcpy (fw->pm->p_path[fw->pm->p_nolink++], path);
	return (fw);
	
}
/*
	routine to include a record into the hashtable if the 
	linkcount has not yet been satisfied. If linkcount is satisfied
	then print the record and clear the entry in the hash list
*/

process_hl ()
{
	struct	hlist	*fw, *rw;
	rw = hlist[stbuf->st_ino % MAXINOD];
	fw = rw->hfw;
	for (; (fw != NULL && (fw->ino != stbuf->st_ino || fw->rdev != stbuf->st_rdev)); fw = fw->hfw) 
		rw = fw;
	
	if (fw != NULL) { /* inode found in hash list */
		if (fw->pm->p_nolink >= CP_MAXLINK) {
			errmsg (WARN,  "Too many links", fw->pm->p_path[0]);
		}
		if ((fw->pm->p_path[fw->pm->p_nolink] = malloc (BUFSIZE)) == NULL) {
			errmsg (WARN,  "Not enough memory", fw->pm->p_path[0]);
			permout (fw);
			rw->hfw = fw->hfw;
			(void) free (fw->pm);
			(void) free (fw);
			return ;
		}
		strcpy (fw->pm->p_path[fw->pm->p_nolink++], path);
		if (fw->pm->p_nolink == fw->nlink) {
			permout (fw);
			rw->hfw = fw->hfw;
			(void) free (fw->pm);
			(void) free (fw);
		}
		return;
	}
	if ((fw = fillperm ()) == NULL) 
		return;
	else {
		rw->hfw = fw;
		fw->hfw = NULL;
	}
	return;

}

prfile ()
{
	struct	hlist	*pret;
		
	if (stbuf->st_nlink > 1 && ((stbuf->st_mode&S_IFMT) != S_IFDIR)) {
		process_hl() ;
		return;
	}
	pret = fillperm ();
	if (pret == NULL)  {
		errmsg (FERR, "Not enough memory", "");
		return ;
	}
	permout (pret);
	(void) free (pret);
	return;
}
/*
	prints out a perms record to standard out.
*/

permout (ptr)
struct	hlist *ptr;
{
	int	i;
	fprintf (stdout ,"%s	%c %4o %4d %4d     ",
		ptr->pm->p_id, 
		ptr->pm->p_ftype,
		ptr->pm->p_perm, 
		ptr->pm->p_owner,
		ptr->pm->p_group);
	if (ptr->pm->p_sflag == T)
		fprintf (stdout, "    %c     ", '-');
	else
		fprintf (stdout, "%5d     ", ptr->pm->p_size); 
	if (ptr->pm->p_cflag == F)
		fprintf (stdout, "%5d %5d ", ptr->pm->p_ck1_maj, ptr->pm->p_ck2_min);
	else
		fprintf (stdout, "    %c     %c  ", '-', '-');
	fprintf (stdout,"%2d\\\n", ptr->nlink);
	
		for (i=0; i<(ptr->pm->p_nolink-1); i++)
			fprintf (stdout,"							%s\\\n",
			ptr->pm->p_path[i]);		
		fprintf (stdout,"							%s\n",
			ptr->pm->p_path[i]);		
}
