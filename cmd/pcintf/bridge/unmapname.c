/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/unmapname.c	1.1"
#include	"system.h"
#include        "sccs.h"
SCCSID(@(#)unmapname.c	3.7	LCC);	/* Modified: 16:15:13 11/28/89 */

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


#define mdebug(X)       debug(0,X)
#define mdebug1(X)      debug(0,X)
#define mdebug2(X)      debug(0,X)
#define mdebug3(X)      debug(0,X)
#define mdebug4(X)      debug(0,X)
#define LOCAL           static          /* only for use in this file */

/* Globally accessable routines in this file:
int     unmapfilename();

/* Local routines */
LOCAL struct direct * find_mapped();
LOCAL ino_t           str2INum();
LOCAL                 str3cat();
LOCAL char *          xstrtok();

#ifndef NOLOG
/* names of these routines for the debug logs */
char *umfn = "unmapfilename";
char *fndm = "find_mapped";
char *s2in = "str2INum";
#endif

/* Global routines used */
extern int match();

/* Library routines used:
**	strpbrk()	declared in <string.h>
**	strspn()	declared in <string.h>
*/

/*
   introChars: Used in introducing the altered part of a mapped file name
*/
extern char     *introChars;
extern char     *iNumChars;

/*
   Conversion radix for inode numbers and it's 2nd and 3rd powers
*/
#define INUM_RADIX      (36L)           /* Default # of i-number chars */
#define IR_2		(INUM_RADIX * INUM_RADIX)
#define IR_3		(INUM_RADIX * INUM_RADIX * INUM_RADIX)
#define has_special(s)  (strpbrk(s, introChars))
#define isupper(c)      (((c) >= 'A') && ((c) <= 'Z'))
#define islower(c)      (((c) >= 'a') && ((c) <= 'z'))
#define isdigit(c)      (((c) >= '0') && ((c) <= '9'))
#define nil(x)          ((x) == 0)

/*
 * unmapfilename() -	Parses an input string containing pathnames returned 
 * 			by the Bridge into real UNIX pathnames.  Note the
 *			Bridge creates special file and directory names to 
 *			insure uniqueness on MS-DOS which alllows a file
 *			and extension eight and three characters respectively.
 *			If the string is OK it returns TRUE otherwise FALSE.
 */

/* 
 *	when hidden file support is enabled, the here is what the rules are
 *	for what this returns:
 *		if attrib != HIDDEN, then return either the non-hidden file
 *			or FILE_NOT_IN_DIR, set attrib to !HIDDEN.  
 *		if attrib == HIDDEN, 
 *			first normal files are attempted to be found,
 *			then their hidden counterparts, 
 *			then (if we don't find it), return FILE_NOT_IN_DIR
 *		At all times (except error), we set attrib correctly.
 *		If we want to find out if there's a hidden file of the same name
 *			as a non-hidden file, request the filename with the attrib=
 *			HIDDEN, and see if it gets set to HIDDEN on return (this will
 *			be used to avoid creating files that already have hidden
 *			counterparts.)  Note that already created non-hidden files will
 *			be found first.
 *			Eric P.
 */



int
#ifdef HIDDEN_FILES
unmapfilename(cwd_name,xlate_name,attrib)
int *attrib;		/* file attributes, in and out */
#else
unmapfilename(cwd_name,xlate_name)
#endif /* HIDDEN_FILES */
char *cwd_name;		/* ptr to current directory string */
char *xlate_name;	/* Points to pathname/file to be mapped */
{
#ifndef HIDDEN_FILES
	int *attrib;
#endif /* HIDDEN_FILES */
    register char *comp;    /* ptr to component of pathname */
    register char *newcomp; /* ptr to newly validated component */
    int ok = TRUE;          /* TRUE if mapping valid; FALSE if dup */
    int rel = FALSE;        /* TRUE if pathname is relative */

    char *parseptr;         /* ptr to parse buffer */
    char tmpname[MAX_PATH]; /* Holds name temporarily */
    char dir[MAX_PATH];     /* Directory to search in */
    char parsebuf[MAX_PATH];/* Temp buffer for string parsing */
    struct direct *dirp;    /* ptr to matched dir entry */
#ifdef HIDDEN_FILES
	int tmp_attrib = 0;			/* save attribs as we move down the filename */
	int retval = FILE_IN_DIR;	/* we set this if otherwise... */
#endif /* HIDDEN_FILES */

#ifndef HIDDEN_FILES
	attrib = &ok;	/* it has to be set to something... */
#endif /* HIDDEN_FILES */

    mdebug(("%s:dir=<%s>,xlate_name=<%s>,attrib=<0x%x>\n", umfn, cwd_name, xlate_name,*attrib));

#ifndef HIDDEN_FILES
    if (!has_special(xlate_name))
		return(TRUE);
#endif	/* HIDDEN_FILES */

    strcpy(tmpname,xlate_name);

    if (*xlate_name != '/')
    {
		rel++;
		parsebuf[0]=0;
		str3cat(parsebuf,"/",xlate_name); /* prepend / for xstrtok */
		dir[0]=0;
		str3cat(dir,cwd_name,"/");
		*xlate_name=0;
    } 
	else
    {
		strcpy(parsebuf,xlate_name);
		strcpy(xlate_name,"/"); /* xstrtok will drop first / */
		dir[0]=0; /* for mapdebugging */
    }

    parseptr = parsebuf;

    mdebug2(("%s:parsebuf=<%s>,xlate_name=<%s>\n", umfn, parsebuf, xlate_name));

    /* the following loop seems odd because of the semantics of
    *  xstrtok; it parses off tokens,returning a ptr to the
    *  start of the first token from its current string ptr,
    *  after zapping a null into the position where the first of
    *  its token separator chars was; if called with a null string
    *  ptr,it continues from where it last left off;
    */

    while(comp=xstrtok(&parseptr,"/"))
    {
	newcomp = comp;
	mdebug(("%s:comp=<%s>,xlate_name=<%s>,dir=<%s>\n",umfn,comp,xlate_name,dir));

		if (has_special(comp))
		{
	    	if (dirp = find_mapped(rel?dir:xlate_name,comp))
	    	{
				newcomp = dirp->d_name;
#ifdef HIDDEN_FILES
				if (newcomp[0] == '.')		/* it's a hidden file */
				{
					if (!(*attrib & HIDDEN)) /* if we aren't looking for them */
						retval = FILE_NOT_IN_DIR;
				}
				else
					tmp_attrib &= ~HIDDEN;	/* turn off hidden (temporarily) */
#endif /* HIDDEN_FILES */
				mdebug2(("%s:newcomp=<%s>,attrib=<%d>\n",umfn,newcomp,*attrib));
	    	}
		}

#ifdef HIDDEN_FILES
		/* no special chars, we have to check if it's hidden */
		else
		{
			tmpname[0] = '\0';
			str3cat(tmpname,(rel?dir:xlate_name),newcomp);
			mdebug2(("Checking for existence of <%s>\n",tmpname));
			if (access(tmpname,0) != 0)
			{
				if (*attrib & HIDDEN)	/* only if we ask for hidden attr */
				{
					mdebug1(("file <%s> doesn't exist, try hidden.\n",tmpname));
					*(newcomp-1) = '.';
					tmpname[0] = '\0';
					str3cat(tmpname,(rel?dir:xlate_name),newcomp-1);
					if (access(tmpname,0) == 0)
					{
						newcomp--;
						mdebug2(("<%s>exists,newcomp=<%s>\n",tmpname,newcomp));
						tmp_attrib |= HIDDEN;
					}
					else
					{
						mdebug2(("file <%s> doesn't exist\n",tmpname));
						retval = FILE_NOT_IN_DIR;
					}
				}
				else	/* it may be there, but we're not allowed to look */
					retval = FILE_NOT_IN_DIR;
			}
			else	/* it's there, and it's NOT hidden */
			{
				mdebug2(("file <%s> exists, not hidden\n",tmpname));
				tmp_attrib &= ~HIDDEN;
			}
		}
#endif /* HIDDEN_FILES */

	str3cat(xlate_name,newcomp,"/");
	if (rel) str3cat(dir,newcomp,"/");
	mdebug2(("%s:xlate_name %s,dir %s\n", umfn, xlate_name, dir));
    }

#ifdef HIDDEN_FILES
	*attrib = tmp_attrib;
#endif /* HIDDEN_FILES */

    mdebug2(("%s:before null-out\n",xlate_name));
    /* Strip off trailing "/" */
    if (*xlate_name)
	xlate_name[strlen(xlate_name)-1] = '\0';

#ifdef HIDDEN_FILES
    mdebug(("%s:returns %d, path=<%s>, xlate_name=<%s>, attrib=<0x%x>\n",
		retval, umfn, cwd_name, xlate_name, *attrib));
	return(retval);
#else
    return(TRUE);
#endif /* HIDDEN_FILES */
}

/* ---------------------------------------------------------------------------
   find_mapped search a directory for a mapped name
*/
LOCAL
struct direct *
find_mapped(dir,file)
char *dir;	/* directory in which to search */
char *file;	/* mapped name to be found */
{

    DIR *dirptr;                    /* handle on the dir being searched */
    struct direct *dirent;          /* handle on cur direntry */
    ino_t iNum;                     /* inode number of file wanted */
    register char *mapBegin;        /* First char of mapped part of name */
    char mappedname[MAX_COMP];

    mdebug1(("%s:dir %s:,file %s\n", fndm, dir, file));

    if (!(mapBegin = strpbrk(file,introChars)))
    {
	/* This was supposed to be a mapped name,but just in case
	*  someone goofed
	*/

	mdebug(("ERR:%s:dir %s,comp %s not mappedname\n", fndm, dir, file));
	return(NULL);
    }

    if ((dirptr = opendir(dir)) == NULL)
    {
		mdebug(("%s:can't open dir %s,errno=%d\n", fndm, dir,errno));
		return(NULL);
    }
    iNum = str2INum(mapBegin);
    mdebug2(("%s:want ino %ld\n", fndm, (long)iNum));
    /* try to find the given entry */
    while ((dirent = readdir(dirptr)))
    {
		mdebug3(("%s:test ino %ld,file %s\n",
	    	fndm, (long)dirent->d_ino, dirent->d_name));
		if (match(file,dirent->d_name,IGNORECASE))
		{
	    /* this one wasn't really a mapped name */
	    	mdebug4(("%s:isn't a mapped name\n", fndm));
	    	break;
		}
		if (dirent->d_ino == iNum)
		{
	    /* got a match,make sure it's the real one */
	    	strcpy(mappedname,dirent->d_name);
	    	mapfilename(dir,mappedname);
	    	mdebug4(("%s:%s maps to %s\n", fndm, dirent->d_name, mappedname));

	    	if (match(mappedname,file,IGNORECASE))
	    	{
			/* this is the one we really want */
				mdebug3(("%s:got it : %s %s %ld\n",
		    		fndm, file, mappedname, iNum));
				break;
	    	}
		}
    }

    closedir(dirptr);
    mdebug1(("%s:returns %s\n",fndm, dirent?dirent->d_name:"NULL"));
    return(dirent);
}

/* ---------------------------------------------------------------------------
   str2INum: Convert a string to an inode number
*/
LOCAL
ino_t
str2INum(iNumStr)
char *iNumStr;               /* Inode number string to translate */
{
    int             iNumDigit;          /* Numeric value of iNum char */
    long            iNum = 0;           /* The computed inode number */
    long            introVal;           /* Value of the introducer char */
    char            *introPos;          /* Points into introChars */

    /* Compute contribution to inode number from introducer character */
    mdebug4(("  %s(%s)\n", s2in, iNumStr));
    if (nil(introPos = strpbrk(introChars, iNumStr)))
    {
		mdebug1(("  error:none of (%s) in (%s)\n", introChars, iNumStr));
		return (ino_t) 0;
    }
    introVal = (long) (introChars - introPos) * IR_3;
    mdebug4(("  introVal=%ld\n", (long)introVal));

    /* Get iNum chars' contributions */
    while (*++iNumStr != '\0' && *iNumStr != '.')
    {
		if (isupper(*iNumStr))
	    	iNumDigit = *iNumStr - 'A';
		else if (islower(*iNumStr))
	    	iNumDigit = *iNumStr - 'a';
		else if (isdigit(*iNumStr))
	    	iNumDigit = *iNumStr - '0' + 26;
		else
		{
	    	mdebug4(("  %s ret 0 (illegal digit %c)\n", s2in, *iNumStr));
	    	return (ino_t) 0;
		}
		iNum *= INUM_RADIX;
		iNum += iNumDigit;
		mdebug4(("  %c = %d. : %ld\n", *iNumStr, iNumDigit, (long)iNum));
    }
    mdebug4(("  %s ret %ld\n", s2in, (long)(introVal + iNum)));

    return (ino_t) (introVal + iNum);
}


/*----------------------------------------------------------------------------
 *      str3cat         concatinate 3 strings
 */
LOCAL
str3cat(s1,s2,s3)
register char *s1;
register char *s2;
register char *s3;
{
    while (*s1++);s1--;     /* are now pointing at the end of s1 */
    while (*s2) *s1++ = *s2++;
    while (*s3) *s1++ = *s3++;
    *s1 = 0;
}


/*----------------------------------------------------------------------------
 * xstrtok: modified version of strtok from SYSV; differs in that instead of
 * using a static to remember where it left off, one passes it the address of
 * a variable which contains a pointer to the string to be scanned; this
 * variable is updated to point to the next place to start from 
 * uses strpbrk and strspn to break string into tokens on
 * sequentially subsequent calls.  returns NULL when no
 * non-separator characters remain.
 */
LOCAL
char *
xstrtok(stradr, sepset)
char	**stradr, *sepset;
{
    register char *p = *stradr;
    register char   *q, *r;


    if(p == 0)                  /* return if no tokens remaining */
	return(NULL);

    q = p + strspn(p, sepset);  /* skip leading separators */

    if(*q == '\0')              /* return if no tokens remaining */
		return(NULL);

    if((r = strpbrk(q, sepset)) == NULL)    /* move past token */
		*stradr = 0;            /* indicate this is last token */
    else
    {
		*r = '\0';
		*stradr = ++r;
    }
    return(q);
}
