/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/mapfile.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)mapfile.c	3.5	LCC);	/* Modified: 13:08:36 7/14/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<string.h>
#include	<fcntl.h>
#include	"xdir.h"

#define	mdebug(X)	debug(0,X)
#define	mdebug1(X)	debug(0,X)
#define	mdebug2(X)	debug(0,X)

extern long dir_offset();
extern char *scan_illegal();
char *getnextcomp();

#ifndef DOSEXEC
extern int match();
unsigned long undirxlate();
char *xstrtok();
#endif /* !DOSEXEC */

#define	LBRACE	'{'
#define	RBRACE	'}'
#define	LBRACE_STR "{"
#define	RBRACE_STR "}"

#define	has_special(s) ((strchr(s,LBRACE)) && (strchr(s,RBRACE)))

#ifndef DOSEXEC
struct direct *
find_mapped(dir,file)
char *dir;	/* directory in which to search */
char *file;	/* mapped name to be found */
{

	DIR *dirptr;			/* handle on the dir being searched */
	struct direct *dirent;		/* handle on cur direntry */
	ino_t offset;			/* inode number of file wanted */
	register char *charptr;		/* pos of LBRACE in file name */
	char mappedname[MAX_COMP];

	mdebug1(("find_mapped:dir %s:,file %s\n",dir,file));

	if (!(charptr = strchr(file,LBRACE))) {
		/* this was supposed to be a mapped name,but just in case
		*  someone goofed
		*/

		mdebug(("ERR:%s:dir %s,comp %s not mappedname\n",
			"find_mapped",dir,file));
		return(NULL);
	}

	if ((dirptr = opendir(dir)) == NULL) {
		mdebug(("find_mapped:cant open dir %s\n",dir));
		return(NULL);
	}
	offset = undirxlate(charptr+1);
	/* try to find the given entry */
	while ((dirent = readdir(dirptr))) {
		mdebug2(("%s:want %ld,got ino %ld,file %s\n",
			"find_mapped",offset,dirent->d_ino,dirent->d_name));
		if (match(file,dirent->d_name,IGNORECASE)) {
			/* this one wasn't really a mapped name */
			break;
		}

		if (dirent->d_ino != offset) continue;

		/* got a match,make sure it's the real one */
		strcpy(mappedname,dirent->d_name);
		mapfilename(dir,mappedname);

		if (match(mappedname,file,IGNORECASE)) {
			/* this is the one we really want */
			break;
		}
	}
	
	closedir(dirptr);
	mdebug1(("%s:returns %s\n","find_mapped",
		dirent?dirent->d_name:"NULL"));
	return(dirent);
}
#endif /* !DOSEXEC */

/*
 * mapfilename() -	Takes as input a UNIX file or pathname and parses 
 *			it into a string where components with greater
 *			significance than MS-DOS will allow are mapped
 *			into unique displayable strings.
 */

int
mapfilename(pathname,resname)
char *pathname;		/* Current working directory pathname */
char *resname;		/* Pathname or file to translate */
{
	register char *comp;		/* ptr to parsed pathname component */
	register char *newcomp;		/* ptr to possibly remapped comp name */
#define	MAXFILENAME	MAXDIRLEN+1
	char file[MAXFILENAME];		/* Holds file */
	char dirbuf[MAX_PATH];		/* Directory to search */
	char *dir = dirbuf;
	char parsebuf[MAX_PATH];	/* tmp buffer for string parsing */
	char *parseptr = parsebuf;	/* ptr to parse buffer */

	static char *nullstr = "";
	static char *slash = "/";
	char *respart = slash;		/* default for abs filenames */
	char *dirpart = nullstr;	/* default for abs filenames */
	char *parsepart = nullstr;	/* default for abs filenames */


	mdebug(("%s:name %s,path %s\n",
		"mapfilename",resname,pathname));

	if (*resname != '/') {
		parsepart = slash;
		dirpart = pathname;
		respart = nullstr;
	} 

	*parseptr = 0;str3cat(parseptr,parsepart,resname);
	*dir = 0;str3cat(dir,dirpart,"/");
	strcpy(resname,respart);

	/* Parse string as pathname (component-at-a-time) */
	mdebug(("%s:resname %s,dir %s, parse %s\n",
		"mapfilename",resname,dir,parseptr));

	while (comp = getnextcomp(&parseptr)) {
		mdebug(("%s:name %s,dir %s,comp %s\n",
			"mapfilename",resname,dir,comp));
		newcomp = comp;
		if (illegal_dosname(comp)) {
			if (mapit(dir,comp,file) == -1)
			{
			    mdebug(("unmapfilename:mapit failed %s, %s, %s\n",
				dir, comp, file));
			    return -1;
			}
			newcomp = file;
			mdebug(("%s:newcomp %s\n",
				"mapfilename",newcomp));
		}
		str3cat(resname,newcomp,"/");
		str3cat(dir,comp,"/");
		mdebug(("%s:name %s,dir %s,comp %s, newcomp %s\n",
			"mapfilename",resname,dir,comp,newcomp));
	}
	if (*resname && (strcmp(resname,"/") != 0)) {
		/* for everything but a simple "/", we have added an extra slash
		*  on the end of the name. Strip it off
		*/

		if (comp = strrchr(resname,'/')) *comp='\0';
	}
	mdebug(("mapfilename:return 0,name %s,pathname %s\n",
		resname,pathname));
	return(0);
}

int
illegal_dosname(file)
char *file;
{
register char *extptr;
register char *chrptr;

	if (is_dot(file) || is_dot_dot(file)) return(FALSE);

	extptr = strrchr(file,'.');
	if (((chrptr = scan_illegal(file)) == FALSE) &&
		(scan_uppercase(file) == FALSE) &&
		((extptr == NULL)
		?   (strlen(file) <= DOS_NAME)
		:   (extptr-file <= DOS_NAME) &&
			(strlen(extptr+1) <= DOS_EXT) &&
			(strchr(file,'.') == extptr))) {
		return(FALSE);
	}
	return(TRUE);
}


mapit(dir,comp,mappedname)
char *dir;		/* directory path to the file to be mapped */
char *comp;		/* component name of the file */
char *mappedname;	/* place to stash the resulting new name */
{
	char *chrptr = NULL;		/* ptr to first illegal character */
	char *extptr = NULL;		/* ptr to file extension */
#undef	OFFSET_LEN
#define	OFFSET_LEN 6
	long offset;
	char offsetstr[OFFSET_LEN+1];/* Holds dir offset string temporarily */
	char extension[6];		/* Holds file extension */
	char file[MAXFILENAME];

	mdebug1(("mapit:dir %s,comp %s\n",
		dir,comp));
	strcpy(file,comp);

	/*
	 * When component is not a valid MS-DOS file:
	 * append LBRACE xlate(inode number) RBRACE,followed by the
	 * file extension if there is one.
	 */


	if ((offset = dir_offset(dir,file)) < 0) {
		mdebug(("mapit:FAILS:name %s,path %s\n",
			file,dir));
		strcpy(mappedname,"MAPITERR");
		return(-1);
	}
	dirxlate(offsetstr,sizeof offsetstr,offset);


	/* Get extension and truncate at any illegal character */
	if (extptr = strrchr(file,'.')) {
		strncpy(extension,extptr,4);
		extension[4] = 0;
		if (chrptr = scan_illegal(extension)) *chrptr = 0;
	}

	/* Determine where to put special char and offset string */
	if (chrptr = scan_illegal(file)) *chrptr = 0;
	file[strcspn(file,".")] = 0;
	file[DOS_NAME - (strlen(offsetstr)+2)] = 0;
	*mappedname='\0';
	str3cat(mappedname,file,LBRACE_STR);
	str3cat(mappedname,offsetstr,RBRACE_STR);

	/* Append extension if there is one */
	if (extptr) strcat(mappedname,extension);
	/* get rid of any uppercase chars in original filename;
	*  otherwise, if someone else calls mapfilename later with
	*  a name it has already built and that name has uppercase, 
	*  it will decide it is not a valid DOSNAME and come 
	*  through here again.
	*/

	cvt2unix(mappedname);
	mdebug1(("mapit:returns %s\n",mappedname));
}


#ifndef DOSEXEC

/*
 * unmapfilename() -	Parses an input string containing pathnames returned 
 * 			by the Bridge into real UNIX pathnames.  Note the
 *			Bridge creates special file and directory names to 
 *			insure uniqueness on MS-DOS which alllows a file
 *			and extension eight and three characters respectively.
 *			If the string is OK it returns TRUE otherwise FALSE.
 */

int
unmapfilename(pathname,name)
char *pathname;		/* ptr to current directory string */
char *name;		/* Points to pathname/file to be mapped */
{
	register char *comp;	/* ptr to component of pathname */
	register char *newcomp; /* ptr to newly validated component */
	int ok = TRUE;		/* TRUE if mapping valid; FALSE if dup */
	int rel = FALSE;	/* TRUE if pathname is relative */

	char *parseptr;		/* ptr to parse buffer */
	char tmpname[MAX_PATH];		/* Holds name temporarily */
	char dir[MAX_PATH];	/* Directory to search in */
	char parsebuf[MAX_PATH];	/* Temp buffer for string parsing */
	struct direct *dirp;	/* ptr to matched dir entry */

	mdebug(("unmapfilename:%s,%s\n",pathname,name));

	if (has_special(name)) {

		strcpy(tmpname,name);

		if (*name != '/') {
			rel++;
			parsebuf[0]=0;
			str3cat(parsebuf,"/",name); /* prepend / for xstrtok */
			dir[0]=0;
			str3cat(dir,pathname,"/");
			*name=0;
		} else {
			strcpy(parsebuf,name);
			strcpy(name,"/"); /* xstrtok will drop first / */
			dir[0]=0; /* for mapdebugging */
		}

		parseptr = parsebuf;

		mdebug(("%s:parsebuf %s,name %s\n",
			"unmapfilename",parsebuf,name));

		/* the following loop seems odd because of the semantics of
		*  xstrtok; it parses off tokens,returning a ptr to the
		*  start of the first token from its current string ptr,
		*  after zapping a null into the position where the first of
		*  its token separator chars was; if called with a null string
		*  ptr,it continues from where it last left off;
		*/

		while(comp=xstrtok(&parseptr,"/")) {
			newcomp = comp;
			mdebug(("%s:comp %s,name %s,dir %s\n",
				"unmapfilename",comp,name,dir));
			
			if (has_special(comp)) {
				if (dirp = find_mapped(rel?dir:name,comp)) {
					newcomp = dirp->d_name;
					mdebug(("%s:newcomp %s\n",
						"unmapfilename",newcomp));
#ifdef	DUPNAMECHK
					if (dupcheck(rel?dir:name,newcomp)) {
					    ok = DUP_FILE_IN_DIR;
					    mdebug((
					       "%s:dup name %s,comp %s\n",
					       "unmapfilename",name,newcomp);
					}
#endif	/* DUPNAMECHK */
				}
			}
			str3cat(name,newcomp,"/");
			if (rel) str3cat(dir,newcomp,"/");
			mdebug(("%s:name %s,dir %s\n",
				"mapfilename",name,dir));
		}
		mdebug(("%s:before null-out\n",name));
		/* Strip off trailing "/" */
		if (*name) name[strlen(name)-1] = '\0';
	}
	mdebug(("%s:returns %d, path %s, name %s\n",
		"unmapfilename",ok,pathname,name));
	return(ok);
}
#endif /* !DOSEXEC */

str3cat(s1,s2,s3)
register char *s1;
register char *s2;
register char *s3;
{
	while (*s1++);s1--;	/* are now pointing at the end of s1 */
	while (*s2) *s1++ = *s2++;
	while (*s3) *s1++ = *s3++;
	*s1 = 0;
}

/*
 * dir_offset() -	Returns the offset within a directory of a file.
 *			If file is not found -1 is returned.
 */

long
dir_offset(pathname,file)
	char *pathname;
	register char *file;
{
	register long offset;			/* Offset of file within directory structure */

	register DIR *dirptr;

	register struct direct *fileptr;
	extern int errno;

	mdebug1(("diroffset:path %s,file %s\n",pathname,file));
	/* Open directory for search */
	if ((dirptr = opendir(pathname)) == 0) {
		mdebug(("dir_offset:cant open %s, errno %d\n",
			pathname,errno));
		return(-1);
	}
	
	/* Search through directory for specified file */
	while (fileptr = readdir(dirptr)) {
		mdebug2(("checking %s against %s\n",
			file,fileptr->d_name));
		if (strncmp(file,fileptr->d_name,sizeof fileptr->d_name) == 0) {
			offset = fileptr->d_ino;
			closedir(dirptr);
			mdebug1(("diroffset:returns 0x%lx\n",offset));
			return(offset);
		}
	}
	closedir(dirptr);
	mdebug(("dir_offset:file %s not in dir %s\n",
		file,pathname));
	return(-1);
}

#ifndef DOSEXEC
/*
 * dupcheck() - 	Returns TRUE if it detects a mapping collision.
 */

int
dupcheck(pathname,file)
char *pathname;
char *file;
{
	register DIR *dirptr;

	register struct direct *fileptr;
	char mappedname[MAX_COMP];	/* mapped version of candidate
					*  directory entries
					*/

	/* Search directory for colliding file */
	if ((dirptr = opendir(pathname)) == NULL) return(FALSE);
	while (fileptr = readdir(dirptr)) {
		strcpy(mappedname,fileptr->d_name);
		mapfilename(pathname,mappedname);
		if (strncmp(file,mappedname,sizeof mappedname) == 0) {
			closedir(dirptr);
			return(TRUE);
		}
	}
	closedir(dirptr);
	return(FALSE);
}
#endif /* !DOSEXEC */


#define	MAXDIGITS	8
#ifndef	HEX
#define	MRADIX	26
static char *tab = "abcdefghijklmnopqrstuvwxyz";
#else	/* HEX */
#define	MRADIX	16
static char *tab = "0123456789abcdef";
#endif	/* HEX */

extern char *strchr();

dirxlate(destptr,nbytes,val)
char *destptr;
int nbytes;
unsigned long val;
{
	char	buf[MAXDIGITS+1];
	register char *bp;
	register char *p;
	register long qval = val;

	buf[MAXDIGITS] = 0;	/* null terminate */

	/* Develop the digits of the value */
	p = bp = buf + MAXDIGITS;
	do {
		*--bp = tab[qval % MRADIX];
		qval /= MRADIX;
	} while (qval != 0);
	strncpy(destptr,bp,nbytes);
}

#ifndef DOSEXEC

unsigned long undirxlate(str)
char *str;
{
	register unsigned long n;
	int i;
	register char c;
	register char *p = str;

	if ((n = strindex(tab,c = *p++))< 0) return(0);

	while ((c = *p++) && ((i = strindex(tab,c)) >= 0)) {
		n *= MRADIX;
		n += i;
	}
	return(n);
}


static
int strindex(tab,c)
char *tab;
char c;
{
	register char *p;
	int n = -1;
	
	if ((p = strchr(tab,c)) ) n = p-tab;
	return(n);
}

/*
 * xstrtok: modified version of strtok from SYSV; differs in that instead of
 * using a static to remember where it left off, one passes it the address of
 * a variable which contains a pointer to the string to be scanned; this
 * variable is updated to point to the next place to start from 
 * uses strpbrk and strspn to break string into tokens on
 * sequentially subsequent calls.  returns NULL when no
 * non-separator characters remain.
 */

extern int strspn();
extern char *strpbrk();
char *
xstrtok(stradr, sepset)
char	**stradr, *sepset;
{
	register char *p = *stradr;
	register char	*q, *r;


	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		*stradr = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		*stradr = ++r;
	}
	return(q);
}

#endif /* !DOSEXEC */

char *
getnextcomp(stradr)
char **stradr;
{
	register char *p = *stradr;
	register char *q = *stradr;


	mdebug1(("%s:stradr %#x,p >>%s<<\n",
		"getnextcomp",stradr,p));
	if (p == 0) {
		mdebug2(("%s:p NULL,ret NULL\n",
			"getnextcomp"));
		return(NULL);	/* no tokens left */
	}
	while (*p == '/') p++;	/* ignore leading slashes */
	q = p;		/* q points at the front of the returned string */

	mdebug2(("%s:q >>%s<<\n","getnextcomp",q));

	while (*p && (*p != '/') ) p++;

	mdebug2(("%s:p %#x >>%s<<\n","getnextcomp",p,p));

	/* p now points either at a null or the first non-leading slash */
	if (*p) {
		/* p points at the slash */
		*p = '\0';	/* zap the slash with a null */
		*stradr = ++p; 	/* and update pointer to next spot */
	} else {
		/*  p points at the null at the end of the string */
		*stradr = 0;		/* flag last trip */
		if (p == q) {
			/* didn't find anything */
			mdebug1(("%s:no comp found,ret NULL\n",
				"getnextcomp"));
			return(NULL);
		}
	}
	mdebug1(("%s:*stradr (%#x) >>%s<< q >>%s<<\n",
		"getnextcomp",*stradr,*stradr,q));
	return(q);		/* return pointer to start of component */
}
