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

#ident	"@(#)mbus:cmd/ckperms/ckperms.c	1.3"

static char ckperms_copyright[] = "Copyright 1986,1987,1988 Intel Corp. 463025";

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<filehdr.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<ar.h>
#include	"defs.h"

struct	passwd	*getpwnam ();
struct	group   *getgrnam ();
extern	char	*malloc ();
extern	int	errno;

static char     options[] = "i:t:n:l:cgvCS";
extern char	*optarg;
extern int	optind;

/*	option flags	*/

int 	iflag,
	gflag,								
	vflag,
	tflag,
	cflag,
	Cflag,
	Sflag,
	nflag,
	lflag,
	errflag = 0;

char	*iname[MAXID],	/* package id name	*/
	ttype[MAXTP],
	*rname,		/* relative path name	*/
	*lfile;		/* log file 		*/

FILE	*fp,		/* perms file pointer   */
	*lfp;		/* log file pointer	*/

/*	global	vars	*/

struct	perm	 perms;		/* perms record			*/
char	buf[BUFSIZE];		/* token buffer for scanner */
int	tp;			/* buf count in token buffer */
char	*fts = "xsracbpde";	/* file types */
char	*special = "cbp";	/* special files */
char	*crfl = "edcbp";	/* file types which may require creation */
char	*argbuf;
char	*it;
int	nid = 0;
int	tno = 0;
int	nfield = 0;	/* counter for no of fields in a perm rec */
int	np = 0;		/* no of pathnames per record		*/
int	done = 0;	/* flag indicating eof		*/
int	lineno = 0;
int	err = 0;


main(argc, argv)
int	argc;
char	**argv;
{
	int	c;
	int	ind = 0;
	char	*temp;
	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'i':
			iflag++;
			it = optarg;
			ind = 0;
			do {
				if ((argbuf = malloc (ARGSIZ)) == NULL) {
					fprintf (stderr, "ckperms: not enough memory\n");
					exit (2);
				}
				nid++;
				getarg();
				iname[ind++] = argbuf;
			}
			while (*it || ind >= MAXID);
			if (ind >= MAXID) {
				fprintf (stderr, "ckperms: Invalid Opts Too many idnames\n");
				giveusage ();
				exit (2);
			}
			break;

		case 'c':
			cflag++;
			break;

		case 'g':								
			gflag++;
			break;

		case 't':
			tflag++;
			it = optarg;
			ind = 0;
			do {
				if ((argbuf = malloc (ARGSIZ)) == NULL) {
					fprintf (stderr, "ckperms: not enough memory\n");
					exit (2);
				}
				tno++;
				getarg();
				temp = argbuf;
				if (*temp && *(++temp) != NULL) {
					fprintf (stderr, "ckperms: invalid args\n");
					giveusage ();
					exit (2);
				}

				ttype[ind++] = *argbuf;
			}
			while (*it || ind >= MAXTP);
			if (ind >= MAXTP) {
				fprintf (stderr, "ckperms: Invalid Opts Too many file types\n");
				giveusage ();
				exit (2);
			}
			break;
		case 'v':		
			vflag++;
			break;
		case 'C':
			Cflag++;
			break;
		case 'S':
			Sflag++;
			break;
		case 'n':
			nflag++;
			rname = optarg;
			break;

		case 'l':
			lflag++;
			lfile = optarg;
			break;

		default:
			errflag++;
			break;
		}
	}
	if (errflag || (argc <= optind) || (Sflag && !Cflag)) {
		fprintf(stderr,"ckperms: Invalid options\n");
		giveusage ();
		exit (2);
	}

	sp (argv[optind]); 
	exit (err);
		
}

giveusage ()
{
	fprintf (stderr, "Usage:  ckperms [-i idlist] [-c] [-g] [-v] [-C [-S]] [-t flist] [-n pathname] [-llogfile] filename\n");
}


getarg()
{
	char *argx;
	argx = argbuf;
	while(*it && *it != ',' && *it != ' ')
		*argx++ = *it++;
	*argx = '\0';

	while( *it && ( *it == ',' || *it == ' ') )
		it++;
}
/*
	This routine scans for tokens from the input file and returns
	the token code. The actual string is returned in "buf"
*/

scan ()
{
	int	c;
	tp = 0;		/* init buf pointer on every call */
	while (1) {	
		c = getc (fp);
		switch (c)  {
		case NL:
			lineno++;
			return (NewLine);
		case EOF:
			return (EndOfFile);
		case COMMNT:
			skipchars ();
			break;
		case BACKSL:	
			if (doback () == 1)
				return (Error);
			break;
		default:
			if (isspace (c)) {
				while (isspace (c) && c != NL)
					c = getc(fp);
				ungetc (c, fp);
			} else {
				ungetc (c, fp);
				tp = 0;
		 		return (getoken ());
		    	}
			break;
		}
	}
	
}	
	


skipchars ()
{
int	c;
	c = getc (fp);
	while ((c != NL) && (c != EOF))
		c = getc (fp);
	if (c == NL)
		lineno++;
}


doback ()
{
	
	switch (getc (fp)) {
		case 	NL:
			lineno++;
		     	return (0);
		default:	/* no char other than nl of eof permitted */
			return (1);
	}
}

getoken ()
{
	int	c;
	while (1) {
		switch (c = getc (fp)) {
		case NL:
			ungetc (c, fp);
			buf[tp] = '\0';
			return (Token);

		case BACKSL:	
			if (doback () == 1)
				return (Error);
			buf[tp] = '\0';
			return (Token);
			
		default:	
			if (isspace (c)) { /* isspace -> white space other than
					                 nl or Eof */
				buf[tp] = '\0';	
				return (Token);
			}
			buf[tp++] = c;
		 	break;
		}
		if (tp > BUFSIZE)
			return (Error);
	}
}
/*
	routine to skip a record in the input file
*/

skiprec ()
{
	int	tok;
		
	tok = scan ();
	while (tok != NewLine && tok != EndOfFile)
		tok = scan ();
	return (tok);
}


sp (fname)
char	*fname;
{
	int	skip;
	int	ret;
	int	i;
	if ((fp = fopen (fname, "r")) == NULL) {
		fprintf (stderr, "Ckperms: cannot open file %s\n", fname);
		err = 2;
		return;
	}
	if (lflag) {
		if ((lfp = fopen (lfile, "w")) == NULL) {
			fprintf (stderr, "Ckperms: cannot open log file %s\n", lfile);
			err = 2;
			return;
		}
	}

/*	initializations		*/
	
	for (i=0; i<CP_MAXLINK; i++) 
		perms.p_path[i] = malloc (BUFSIZE + 1);
	(void) clearec ();
	
	while (!done) {
		skip = 0;
		switch (scan()){ 
		case Token: { /* a valid token string is found */

		     	switch (nfield) { /* nfield is a variable which 
					     identifies the field in record */
			case ID:
				if (do_id ()) {
					skip++;
					if (lflag)
						log (STRING, "Not same idname  - record skipped..");
				}
				break;
			case FT:
				if (ret = do_ft ()) {  
					skip++;
					if (lflag && ret == 2)
						log (STRING, "Not same file type - record skipped..");
				}
				break;
			case PERM:
			 	if (do_perm ()) 
					skip++;
				break;
			case UID:
				if (do_owner ()) 
					skip++;
				break;
			case GID:
				if (do_group ()) 
					skip++;
				break;
			case SIZE:
				if (do_size ()) 
					skip++;
				break;
			case CSUM1:
			case CSUM2:
				if (do_ck ()) 
					skip++;
				break;
			case LINK:
				if (do_links ()) 
					skip++;
				break;
			case PATH: 
				if (np > CP_MAXLINK) {
					errmsg (DERR, INCLN, "Too many pathnames", NULL);
					skip++;
					break;
				}
				do_paths (np++);
				break;
			default :
				errmsg (FERR, INCLN, "Internal Error ", NULL);
				return; 
			}
			if (skip) {
				if (skiprec () == EndOfFile) 
					done++;
				clearec ();
				skip = 0;
			} else {
				if (nfield < TFIELDS) 
					nfield++;
			}
			break;
			}
		case NewLine:
			process_rec ();
			clearec ();
			break;
		case EndOfFile:
			done++;
			break;
		case Error:
			errmsg(DERR, INCLN, "Syntax error", NULL);
			if (skiprec () == EndOfFile)
				done++;
			clearec ();
			break;
		default:
			errmsg (FERR, INCLN, "Internal Error", NULL);
			return;
		}
	}
	fclose (fp);
	if (lflag) 
		fclose (lfp);
}

/*
	clear the record reset nfield and np
*/
clearec ()
{
	nfield = 0;
	np = 0;
	perms.p_id[0] = '\0'; 
	perms.p_ftype = '\0';
	perms.p_perm = perms.p_owner = perms.p_group = perms.p_size = 0;
	perms.p_ck1_maj = perms.p_ck2_min = perms.p_nolink = 0;
	perms.p_sflag = perms.p_cflag = 0;
	strcpy (perms.p_path[0], "");
	
}
/*
	if I opt not specified process all ids, otherwise skip a record
	whose idname does not match the given idname.
*/

do_id ()
{
	int	i;
	if (iflag) {
		for (i=0; i<nid; i++) {
			if (strcmp (iname[i], buf) == 0)
				break;
		}
		if (i >= nid)   
			return (1);
	}
	strcpy (perms.p_id, buf);
	return (0);
		
}

/*
	if valid file type then 
		if not t opt process all records
		else return code to skip record
	else return error code.  
*/

do_ft ()
{
	if (strlen (buf) == 1) {
		if (strchr (fts, buf[0]) != NULL) {
			if (tflag && strchr (ttype, buf[0]) == NULL) 
				return (2);
			perms.p_ftype = buf[0];
			return (0);
		}
	}
	errmsg (DERR, INCLN, "Invalid File Type", NULL);
	return (1);
}

/*
	check if permissions are octal nos. 
	if valid perms store result otherwise return error.
*/

do_perm ()
{
	
	register  c, i;
	char	*s;
	s = buf;
	for (i = 0; (c = *s) >= '0' && c <= '7'; s++)
		i = (i << 3) + (c - '0');
	if (*s) {
		errmsg(DERR, INCLN, "Invalid mode permissions", NULL);
		return (1);
	}
	perms.p_perm = i;
	return (0);
}

/*
	if numeric owner then if owner too large return error.
	If ascii owner then get the numeric owner.
	store numeric owner.
*/

do_owner ()
{
	int	c, alpha = 0;
	char	c1, *s;
	ushort	i, j;
	struct  passwd *pwd;

	s = buf;
	while(c = *s++) {
		if(!isdigit(c)) {
			alpha++;
			break;
		}
	}
	s = buf;
	if (!alpha) {
		i = j = 0;
		while(c1 = *s++) {
			i = c1 - '0' + 10 * i;
			if (i < j) {
				errmsg (DERR, INCLN, "owner id too large", NULL);
				return (1);
    			}
			j = i;
		}
		perms.p_owner = i;
		return (0);
	}
	if ((pwd = getpwnam(buf)) == NULL) {
		errmsg (DERR, INCLN, "Invalid user id ", NULL);
		return (1);
	}
	perms.p_owner = pwd->pw_uid;
	return (0);
}


	
/*
	if numeric group then if grp no too large return error
	If ascii group then get the numeric group. 
	store group id.
*/

do_group ()
{
	int	c, alpha = 0;
	char	c1, *s;
	ushort	i, j;
	struct  group *g;

	s = buf;
	while(c = *s++) {
		if(!isdigit(c)) {
			alpha++;
			break;
		}
	}
	s = buf;
	if (!alpha) {
		i = j = 0;
		while(c1 = *s++) {
			i = c1 - '0' + 10 * i;
			if (i < j) {
				errmsg (DERR, INCLN, "group id too large", NULL);
				return (1);
    			}
			j = i;
		}
		perms.p_group = i;
		return (0);
	}
	if ((g = getgrnam(buf)) == NULL) {
		errmsg (DERR, INCLN, "Invalid group id ", NULL);
		return (1);
	}
	perms.p_group  = g->gr_gid;
	return (0);
}

/*
	if the size field is "-" then setflag in perms rec to
	ignore size field. 
	if Invalid size field then return error.
	Otherwise store size field.
*/

do_size ()
{
	char c, *s;
	long	sz;
	if (buf[0] == '-' && buf[1] == '\0') {
			perms.p_sflag = F;
			return (0);
	}
	perms.p_sflag = T;
	s = buf;
	sz = 0;
	while (c = *s++) {
		if (!isdigit (c)) {
			errmsg (DERR, INCLN, "Invalid size field", NULL);
			return (1); 
		}
		sz = c - '0' + 10 * sz;
	}
	perms.p_size = sz;
	return (0);
}

/*
	nfield = 6 -> first check sum field.
	nfield = 7 -> second chksum field.
	if first chksum then 
		if "-"  setflg in perms rec to ignore chksum.
		if invalid chksum then return error.
		otherwise store chksum field.
	if second chksum (block size) then
		if "-"  and first checksum flag not set then error
		if invalid chksum return error.
		otherwise store field.
*/

do_ck ()
{
	char c, *s;
	long	ck;
	if (buf[0] == '-' && buf[1] == '\0') {
		if (nfield == 6) 
			perms.p_cflag = F;
		else  
		     	if (perms.p_cflag != F)  {
				errmsg (DERR, INCLN, "Invalid checksum field", NULL);
				return (1);
			} 
		return (0);
	}
	if (perms.p_cflag == F) {
		errmsg (DERR, INCLN, "Invalid checksum field", NULL);
		return (1);
	}
	perms.p_cflag = T;
	s = buf;
	ck = 0;
	while (c = *s++) {
		if (!isdigit (c)) {
			errmsg (DERR, INCLN, "Invalid checksum field", NULL);
			return (1); 
		}
		ck = c - '0' + 10 * ck;
	}
	if (nfield == 6)
		perms.p_ck1_maj = ck;
	else
		perms.p_ck2_min = ck;
	return (0);
}

/*
	if invalid link return error. 
	otherwise store no of links.
*/

do_links ()
{
	char c, *s;
	ushort ln;
	s = buf;
	ln = 0;
	while (c = *s++) {
		if (!isdigit (c)) {
			errmsg (DERR, INCLN, "Invalid link field", NULL);
			return (1); 
		}
		ln = c - '0' + 10 * ln;
	}
	if (ln == 0 || ln > CP_MAXLINK) {
		errmsg (DERR, INCLN, "Invalid link field", NULL);
		return (1);
	}
	perms.p_nolink = ln;
	return (0);
}

/*
	if a relative path specified in command lin opt and
	input path is not an absolute path name then 
	append relative pathname to input pathname.
*/


do_paths (npaths)
int	npaths;
{

	if (nflag && *buf != '/') {
		sprintf (perms.p_path[npaths],"%s%s%s",rname, "/", buf);
		return ;
	} else {
		if (nflag && *buf == '/') 
			errmsg (WARN, INCLN, "absolute pathname with 'n' opt");
	}
	strcpy (perms.p_path[npaths], buf);
}
/*
	this pertains to the record as a whole and interdependencies in
	fields are checked for validity before the record is processed.
*/

process_rec ()
{
	int	i, ck1, ck2;
	char	ftyp;
	ushort  ino;
	int		dev;		
	short	bypass;		
	dev_t	rdev;
	int	mode, nexist = 0;
	char	str[2*BUFSIZE]; 
	struct stat stbuf, spbuf;

/*	
	check if the entries in the perms rec are okay		
*/
	if (perms.p_id[0] == '\0') /* a null record */
		return;
	if (strcmp (perms.p_path[0], "") == NULL) {
		errmsg (DERR, SAMLN, "Invalid pathname", perms.p_path[0]);
		return;
	}
	ftyp = perms.p_ftype;
	if (np != perms.p_nolink) {
		errmsg (DERR, SAMLN, "Invalid no of pathnames", perms.p_path[0]);
		return;
	}

	if (ftyp == 'd' && perms.p_nolink > 1) {
		errmsg (DERR, SAMLN, "Invalid link count for directory", perms.p_path[0]);
		return;
	}

/*
	check if block or char special file then check sum field != '-' and 
	size field neglected
	also check that for a pipe special file both the checksum and 
	size field are neglected.
*/
	if (ftyp == 'b' || ftyp == 'c') {
		if (perms.p_cflag == F || perms.p_sflag == T) {
			errmsg (DERR, SAMLN, "Invalid size/checksum field", perms.p_path[0]);
			return;
		}
	} else {
		if ((ftyp == 'p' || ftyp == 'd') && (perms.p_cflag == T || perms.p_sflag == T)) {
			errmsg (DERR, SAMLN, "Invalid size/checksum field", perms.p_path[0]);
			return;
		}
	}

	
	
	if (Cflag) {
		switch (ftyp) {
			case 'd':	mode = S_IFDIR;
					break;
			case 'c':	mode = S_IFCHR;
					break;
			case 'b':	mode = S_IFBLK;
					break;
			case 'p':	mode = S_IFIFO;
					break;
			default:	mode = S_IFREG;
					break;
		}
	
		if (stat (perms.p_path[0], &stbuf) == -1) { 
			if (strchr (crfl, ftyp) == NULL) {
				errmsg (PERR, SAMLN, "File does not exist", perms.p_path[0]);
				return;
			}
		} else
			nexist = 1;
	
		/* check if ft in perms rec corressponds to existing file   */ 
	
		if (nexist && ftyp != 'e' && ftyp != 's' && ftyp != getft (stbuf.st_mode, perms.p_path[0]))  {
			errmsg (PERR, SAMLN, "File type mismatch", perms.p_path[0]);
		}
		dev = 0;
		bypass = 1;
		switch (ftyp) {
			case 'b':
			case 'c':
					bypass = 0;
					if (perms.p_cflag == F) {
						errmsg (PERR, SAMLN, "Cannot creat special file", perms.p_path[0]);
						return;
					}
					dev = ((perms.p_ck1_maj&0xFF) << 8) | (perms.p_ck2_min&0xFF);
					if (nexist)
					{
						if (stbuf.st_rdev == (dev_t)dev)
							bypass++;
						else
						{
							for (i=0; i<perms.p_nolink; i++)  /* do for all links */
							{
								if (unlink(perms.p_path[i])==-1) 
								{
									errmsg (PERR, SAMLN, "Cannot unlink special file", perms.p_path[0]);
									return;
								}
							}
						}
					}
			case 'e':
			case 'p':
					if (!(nexist && bypass)) { 
						if (mknod (perms.p_path[0], (mode | perms.p_perm), dev)==-1) {
							errmsg (PERR, SAMLN, "Cannot creat special file", perms.p_path[0]);
							return;
						}
		     		}
					break;
			case 'd':
					if (!nexist)
					{
						if (mkdir (perms.p_path[0], mode) == -1) {
							errmsg (PERR, SAMLN, "Cannot creat directory", perms.p_path[0]);
							return;
						} 
					}
					break;
			default :
					if (nexist && Sflag && ftyp == 's') {
						if (stat ("/bin/strip", &spbuf) == -1) {
							errmsg (PERR, SAMLN, "/bin/strip not found", perms.p_path[0]);
							return;
						}
						sprintf (str, "/bin/strip '%s' > /dev/null", perms.p_path[0]);
						if (system (str) != 0) {
							errmsg (PERR, SAMLN, "Unable to strip file", perms.p_path[0]);
							return;
						}
					}
					break;
		}
		if (chmod (perms.p_path[0], perms.p_perm) == -1) {
			errmsg (PERR, SAMLN, "cannot change mode", perms.p_path[0]);
			return;
		}
		if (chown (perms.p_path[0], perms.p_owner, perms.p_group) == -1) {
			errmsg (PERR, SAMLN, "cannot change owner/group", perms.p_path[0]);
			return;
		}
		if (perms.p_nolink > 1) {
			for (i=1; i<np; i++) {
				if (link (perms.p_path[0], perms.p_path[i]) == -1) {
					if (errno == EEXIST) {
						if (stat (perms.p_path[i], &spbuf) == -1) {
							errmsg (PERR, SAMLN, "Cannot stat link", perms.p_path[0]);
							return;
						}
						if (spbuf.st_ino != stbuf.st_ino) 
							errmsg (WARN, SAMLN, "link file exists", perms.p_path[0]);
					} else {
						errmsg (PERR, SAMLN, "cannot create link", perms.p_path[0]);
					}
				}
			}
		}
	} else {
		for (i=0; i<perms.p_nolink; i++) { /* do for all links */

			if (stat (perms.p_path[i], &stbuf) == -1) {
				errmsg (PERR, SAMLN, "file does not exist", perms.p_path[i]);
				continue;
			}
			if (i == 0) {
				ino = stbuf.st_ino;
				rdev = stbuf.st_rdev;
			}
			else 
				if (stbuf.st_ino != ino || stbuf.st_rdev != rdev) 
					errmsg (PERR, SAMLN , "Invalid link to pathname", perms.p_path[i]);
			ftyp = getft (stbuf.st_mode, perms.p_path[i]);
			if (perms.p_ftype != 'e' && perms.p_ftype != ftyp) 
				errmsg (PERR, SAMLN, "Invalid file type", perms.p_path[i]);
		
			if (perms.p_perm  != (stbuf.st_mode&CP_PMASK)) 
				errmsg (PERR, SAMLN, "Invalid mode", perms.p_path[i]);
			
			if (perms.p_owner != stbuf.st_uid)  
				errmsg (PERR, SAMLN, "Invalid owner", perms.p_path[i]);
			
			if (perms.p_group != stbuf.st_gid)  
				errmsg (PERR, SAMLN, "Invalid group", perms.p_path[i]);
			
			if (cflag && !perms.p_sflag)  {
				if (perms.p_size != stbuf.st_size) 
					errmsg (PERR, SAMLN, "Size mismatch", perms.p_path[i]);
			}
	
			/*	check checksum;  */
			if (perms.p_cflag == T && ftyp != 'e' && ftyp != 'd') {
				if (strchr (special, ftyp) != NULL) {
					ck1 = major (stbuf.st_rdev);
					ck2 = minor (stbuf.st_rdev);
					if (perms.p_ck1_maj != ck1 || perms.p_ck2_min != ck2) 
						errmsg (PERR, SAMLN, "checksum mismatch", perms.p_path[i]);
				} else {
					if (cflag) {
						if (getcksum (&ck1, &ck2, i) == 1) {
							return;
						}
						if (perms.p_ck1_maj != ck1 || perms.p_ck2_min != ck2) 
							errmsg (PERR, 0, "checksum mismatch", perms.p_path[i]);
					}
				}
			}
			if (ftyp == 'd') {
				if (perms.p_nolink != 1)
					errmsg (PERR, SAMLN, "link mismatch", perms.p_path[i]);
			}
			else {
				if (perms.p_nolink != stbuf.st_nlink) 
					errmsg (PERR, SAMLN, "link mismatch", perms.p_path[i]);
			}
			
		}
	}
	/*
	 * 	Check v flag or g flag
	 *	For printing a list of pathnames.
	 */
	if (vflag)  							
		for (i=0; i<perms.p_nolink; i++)   /* do for all links */
			fprintf (stdout,"%s\n", perms.p_path[i]);
	else
		if (gflag) 
			switch (perms.p_ftype) {
				case 'b':	
				case 'c':	
				case 'd':	
				case 'e':	
				case 'p':	
							break;
				default:	
					for (i=0; i<perms.p_nolink; i++)   /* do for all links */
						fprintf (stdout,"%s\n", perms.p_path[i]);
					break;
			}

	if (lflag)
		log (RECORD, NULL); 
		
}

/*
	This executes the utility "sum" to return the checksum for the file
*/
getcksum (ck1, ck2, index)
int	*ck1, *ck2;
int	index;
{
	FILE	*fptr;
	char	tsbuf[BUFSIZE];
	struct	stat	spbuf;
	char	str[2*BUFSIZE];

	if (stat ("/bin/sum", &spbuf) == -1) {
		errmsg (PERR, SAMLN, "/bin/sum not found", NULL);
		return (1);
	}

	sprintf (str, "sum '%s'", perms.p_path[index]);

	if ((fptr = popen (str, "r")) != NULL) {
		if (fgets (tsbuf, BUFSIZE, fptr) != NULL) {
			if (strcmp (tsbuf,"") == NULL) {
				errmsg (PERR, SAMLN, "Invalid sum of file", perms.p_path[index]);
				return (1);
			}
			if (sscanf (tsbuf, "%d %d", ck1, ck2) == -1) {
				errmsg (PERR, SAMLN, "Invalid sum of file", perms.p_path[index]);
				return (1);
			}
		}
	}
	pclose (fptr);
	return (0);
}

/*

	print out appropriate message and adjust line no.
*/


errmsg (code, lc, str, path)
int	code;
int	lc;
char 	*str;
char	*path;
{
	int	tlno;
	char	lstr[BUFSIZE];

	tlno = lineno;
	if (lc)
	   tlno++; 
	if (code == WARN) {
		sprintf (lstr, "%s %s", "Warning issued - ", str);
		fprintf (stderr, "%3d %s %s %s %s\n", tlno, "ckperms:", "Warning", str, path);
	}
	else {
		sprintf (lstr, "%s %s", "Error - ", str);
		fprintf (stderr, "%3d %s %s %s\n", tlno, "ckperms:", str, path);
		if (code == PERR)
			err = 3;
		if (code == FERR)
			err = 2;
		if (code == DERR)
			err = 1;
	}
	if (lflag) 
		log (lc, lstr);
	
}
/*
	log the actual characteristics of the file as it exists after all processing.
	the checksum and size fields are recorded only if the original input
	file recorded these fields as being non empty.
	The path names of the links are also logged in to be the same as
	the input file
*/

log (code, str)
int	code;
char	*str;
{
	int	i, ck1, ck2, nlink;
	char	ftyp;
	struct	stat	stbuf;
	int	tlno;
	tlno = lineno;
	if (code == STRING || code == INCLN){
		if (code == INCLN)
			tlno++; 
		fprintf (lfp, "%3d %s\n", tlno, str);
		return;
	}
	if (stat(perms.p_path[0], &stbuf) == -1) { 
		errmsg (WARN, SAMLN, "stat failed in logging", perms.p_path[0]);
		return;
	}
	ftyp = perms.p_ftype;
	if (ftyp != 'e')  
		ftyp = getft(stbuf.st_mode, perms.p_path[0]);
	fprintf (lfp,"%s	%c %4o %4d %4d     ",
		perms.p_id, 
		ftyp,
		stbuf.st_mode&CP_PMASK,
		stbuf.st_uid,
		stbuf.st_gid);

	if (perms.p_sflag == F || strchr (special, ftyp) != NULL || ftyp == 'd')
		fprintf (lfp, "    %c     ", '-');
	else
		fprintf (lfp, "%5d     ", stbuf.st_size); 
	if (perms.p_cflag == F || ftyp =='p' || ftyp == 'd')
		fprintf (lfp, "    %c     %c  ", '-', '-');
	else {
		if (strchr (special, perms.p_ftype) == NULL) {
			if (getcksum (&ck1, &ck2, 0) == 1) {
				errmsg (WARN, 0, "Unable to get cksum in logging..", perms.p_path[0]);
				return;
			}
		} else {
			ck1 =  major (stbuf.st_rdev);
			ck2 = minor (stbuf.st_rdev);
		}
		fprintf (lfp, "%5d %5d ", ck1, ck2);
	}
	nlink = stbuf.st_nlink;
	if (ftyp == 'd')
		nlink = 1;
	fprintf (lfp,"%2d\\\n", nlink);
	
		for (i=0; i<(perms.p_nolink-1); i++)
			fprintf (lfp,"							%s\\\n",
			perms.p_path[i]);		
		fprintf (lfp,"							%s\n",
			perms.p_path[i]);		
}
