#ident	"@(#)mapname.c	1.2	92/07/24	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/mapname.c	1.1"

#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)mapname.c	3.8	LCC);	/* Modified: 14:51:48 11/27/89 */

/****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

#include	"pci_types.h"
#include	<string.h>
#include	<fcntl.h>
#include	"xdir.h"


#define mdebug(X)       debug(0,X)
#define mdebug1(X)      debug(0,X)
#define mdebug2(X)      debug(0,X)
#define mdebug3(X)      debug(0,X)
#define LOCAL           static          /* only for use in this file */

/* Globally accessable routines in this file:
int     mapfilename();

/* Local routines */
LOCAL int       illegal_dosname();
LOCAL int       mapit();
LOCAL           str4cat();
LOCAL ino_t     get_inode();
LOCAL char *    getnextcomp();
LOCAL int       iNum2Str();

/* Global routines used */
extern char *scan_illegal();
extern int  cover_illegal();

/*
   introChars: Used in introducing the altered part of a mapped file name
   introChars: Used in introducing the altered part of a mapped file name
*/
char    *introChars = "'~^[";
char    *iNumChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static  char * nullstr = "";

/*
   Conversion radix for inode numbers and it's 2nd and 3rd powers
*/
#define INUM_RADIX      (36L)           /* Default # of i-number chars */
#define IR_2		(INUM_RADIX * INUM_RADIX)
#define IR_3		(INUM_RADIX * INUM_RADIX * INUM_RADIX)


/*
 * mapfilename() -	Takes as input a UNIX file or pathname and parses 
 *			it into a string where components with greater
 *			significance than MS-DOS will allow are mapped
 *			into unique displayable strings.
 */

int
mapfilename(cwd_name,xlate_name)
char *cwd_name;		/* Current working directory pathname */
char *xlate_name;		/* Pathname or file to translate */
{
    register char *comp;            /* ptr to parsed pathname component */
    register char *newcomp;         /* ptr to possibly remapped comp name */
#define	MAXFILENAME	MAXDIRLEN+1
    char file[MAXFILENAME];         /* Holds file */
    char dirbuf[MAX_PATH];          /* Directory to search */
    char *dir = dirbuf;
    char parsebuf[MAX_PATH];        /* tmp buffer for string parsing */
    char *parseptr = parsebuf;      /* ptr to parse buffer */

    static char *nullstr = "";
    static char *slash = "/";
    char *respart = slash;          /* default for abs filenames */
    char *dirpart = nullstr;        /* default for abs filenames */
    char *parsepart = nullstr;      /* default for abs filenames */


    mdebug(("%s:xlate_name=(%s) path=(%s)\n", 
		"mapfilename",xlate_name,cwd_name));

    if (*xlate_name != '/')
    {
	    parsepart = slash;
	    dirpart = cwd_name;
	    respart = nullstr;
    }

    *parseptr = 0;str4cat(parseptr,parsepart,xlate_name,nullstr);
    *dir = 0;
	str4cat(dir,dirpart,(dirpart[strlen(dirpart)-1]=='/')?nullstr:slash,nullstr);
    strcpy(xlate_name,respart);

    /* Parse string as pathname (component-at-a-time) */
    mdebug(("%s:xlate_name=(%s) dir=(%s)  parse=(%s)\n", 
		"mapfilename",xlate_name,dir,parseptr));

    while (comp = getnextcomp(&parseptr))
    {
	mdebug(("%s:xlate_name=(%s) dir=(%s) comp=(%s)\n",
	    "mapfilename",xlate_name,dir,comp));
	newcomp = comp;
#ifdef HIDDEN_FILES
	if IS_HIDDEN(newcomp)
	{
		UNHIDE(newcomp);
		mdebug(("<%s> was hidden: translated to <%s>\n",comp,newcomp));
	}
#endif /* HIDDEN_FILES */

	if (illegal_dosname(newcomp))	/* ERP changed from comp to newcomp */
	{
	    if (mapit(dir,comp,file))	
	    {
		mdebug(("mapfilename:mapit failed: dir=<%s>, newcomp=<%s>, file=<%s>\n",
		    dir, newcomp, file));
		return -1;
	    }
	    newcomp = file;
	    mdebug(("%s:newcomp<%s>\n",
		    "mapfilename",newcomp));
	}
	str4cat(xlate_name,newcomp,"/",nullstr);
	str4cat(dir,comp,"/",nullstr);
	mdebug(("%s:xlate_name=(%s) dir=(%s) comp=(%s) newcomp=(%s)\n",
		"mapfilename",xlate_name,dir,comp,newcomp));
    }
    if (*xlate_name && (strcmp(xlate_name,"/") != 0))
    {
	/* for everything but a simple "/", we have added an extra slash
	*  on the end of the name. Strip it off
	*/

	if (comp = strrchr(xlate_name,'/'))
	    *comp='\0';
    }
    mdebug(("mapfilename:return 0,xlate_name=(%s) cwd_name=(%s)\n",
		xlate_name,cwd_name));
    return(0);
}

/*----------------------------------------------------------------------------
 *      illegal_dosname -
 */
LOCAL
int
illegal_dosname(file)
char *file;
{
    register char *extptr;

    if (is_dot(file) || is_dot_dot(file)) return(FALSE);

    extptr = strrchr(file,'.');
    if ((scan_illegal(file) == FALSE) &&
	(scan_uppercase(file) == FALSE)          &&
	((extptr == NULL)
	    ?   (*file && strlen(file) <= (size_t)DOS_NAME)
	    :   ((extptr-file) && extptr-file <= DOS_NAME)        &&
		(strlen(extptr+1) <= (size_t)DOS_EXT)    &&
		(strchr(file,'.') == extptr) ) )
    {
	    return(FALSE);
    }
    return(TRUE);
}


/*----------------------------------------------------------------------------
 *      mapit -	Maps the component of the fully specified file name.
 *		Return FALSE for success, and TRUE for failure when
 *		the file does not exist.
 */
LOCAL
int
mapit(dir,comp,mappedname)
char *dir;		/* directory path to the file to be mapped */
char *comp;		/* component name of the file */
char *mappedname;	/* place to stash the resulting new name */
{
    char *extptr = NULL;        /* ptr to file extension */
#undef	OFFSET_LEN
#define	OFFSET_LEN 6
    ino_t iNum;
    char istr[OFFSET_LEN+1];    /* Holds string constructed from inode */
    char extension[6];          /* Holds file extension */
    char file[MAXFILENAME];
#ifdef HIDDEN_FILES
	char *filePtr = file;
#endif /* HIDDEN_FILES */

    mdebug1(("mapit:dir(%s) comp(%s)\n", dir,comp));
#ifdef HIDDEN_FILES
    strcpy(filePtr,comp);
#else 
    strcpy(file,comp);
#endif /* HIDDEN_FILES */

    /*
     * When component is not a valid MS-DOS file:
     * append QUOTE xlate(inode number) ,followed by the
     * file extension if there is one. Any illegal
     * char is replaced with an underscore.
     */
#ifdef HIDDEN_FILES
    if ((iNum = get_inode(dir,filePtr)) == (ino_t)-1)
#else
    if ((iNum = get_inode(dir,file)) == (ino_t)-1)
#endif /* HIDDEN_FILES */
    {
	mdebug1(("mapit: get_inode failed\n"));
	return TRUE;
    }
    iNum2Str( iNum, istr);
#ifdef HIDDEN_FILES
	if IS_HIDDEN(filePtr)			/* we don't want a leading underscore */
		UNHIDE(filePtr);
    cover_illegal(filePtr);    /* replace illegal chars with underscore */
#else 
    cover_illegal(file);    /* replace illegal chars with underscore */
#endif /* HIDDEN_FILES */

    /* Get extension */
#ifdef HIDDEN_FILES
    if (extptr = strrchr(filePtr,'.'))
#else 
    if (extptr = strrchr(file,'.'))
#endif /* HIDDEN_FILES */
    {
	    strncpy(extension,extptr,4);
	    extension[4] = '\0';
	    *extptr = '\0';
    }
    else
	    extension[0] = '\0';

   /* shorten base name so inode string will fit */
#ifdef HIDDEN_FILES
    filePtr[DOS_NAME - strlen(istr)] = '\0';
#else 
    file[DOS_NAME - strlen(istr)] = '\0';
#endif /* HIDDEN_FILES */
    *mappedname = '\0';
#ifdef HIDDEN_FILES
    str4cat(mappedname, filePtr, istr, extension);
#else 
    str4cat(mappedname, file, istr, extension);
#endif /* HIDDEN_FILES */

   /* get rid of any uppercase chars in original filename;
    *  otherwise, if someone else calls mapfilename later with
    *  a name it has already built and that name has uppercase,
    *  it will decide it is not a valid DOSNAME and come
    *  through here again.
    */

    cvt2unix(mappedname);
    mdebug1(("mapit:returns(%s)\n",mappedname));
    return FALSE;
}

/*----------------------------------------------------------------------------
 *      str4cat -       concatinate 4 strings
 */
LOCAL
str4cat(s1,s2,s3,s4)
register char *s1;
register char *s2;
register char *s3;
register char *s4;
{
    while (*s1++);s1--;         /* are now pointing at the end of s1 */
    while (*s2) *s1++ = *s2++;
    while (*s3) *s1++ = *s3++;
    while (*s4) *s1++ = *s4++;
    *s1 = 0;
}

/*----------------------------------------------------------------------------
 * get_inode() -        Returns the inode of a file.
 *			If file is not found (ino_t)-1 is returned.
 */

LOCAL
ino_t
get_inode(pathname,file)
char *pathname;
register char *file;
{
    register ino_t iNum;       /* The inode number of the file */

    register DIR *dirptr;

    register struct direct *fileptr;
    extern int errno;

    mdebug2(("get_inode:path(%s) file(%s)\n",pathname,file));
    /* Open directory for search */
    if ((dirptr = opendir(pathname)) == 0)
    {
    	mdebug(("get_inode:cant open<%s>, errno<%d>\n", pathname,errno));
	   		return((ino_t)-1);
    }

    /* Search through directory for specified file */
    while (fileptr = readdir(dirptr))
    {
	mdebug3(("    checking %s against %s\n", file,fileptr->d_name));
#if !defined(SYS5_3) && !defined(SYS5_4)
	if (strncmp(file,fileptr->d_name,sizeof fileptr->d_name) == 0)
#else	/* defined(SYS5_3) || defined(SYS5_4) */
	if (strcmp(file, fileptr->d_name) == 0)
#endif	/* !defined(SYS5_3) && !defined(SYS5_4) */
	{
	    iNum = fileptr->d_ino;
	    closedir(dirptr);
	    mdebug1(("get_inode:returns %ld\n",(long)iNum));
	    return(iNum);
	}
    }
    closedir(dirptr);
    mdebug(("get_inode:file %s not in dir %s\n", file, pathname));
    return((ino_t)-1);
}

/*----------------------------------------------------------------------------
 * getnextcomp -
 */

LOCAL
char *
getnextcomp(stradr)
char **stradr;
{
    register char *p = *stradr;
    register char *q = *stradr;


    mdebug3(("%s:stradr %#x,p >>%s<<\n", "getnextcomp",stradr,p));
    if (p == 0)
    {
	mdebug2(("%s:p NULL,ret NULL\n", "getnextcomp"));
	return(NULL);   /* no tokens left */
    }
    while (*p == '/') p++;  /* ignore leading slashes */
    q = p;                  /* q points at the front of the returned string */

    mdebug3(("%s:q >>%s<<\n","getnextcomp",q));

    while (*p && (*p != '/') ) p++;

    mdebug3(("%s:p %#x >>%s<<\n","getnextcomp",p,p));

    /* p now points either at a null or the first non-leading slash */
    if (*p)
    {
	/* p points at the slash */
	*p = '\0';      /* zap the slash with a null */
	*stradr = ++p;  /* and update pointer to next spot */
    } else
    {
	/*  p points at the null at the end of the string */
	*stradr = 0;            /* flag last trip */
	if (p == q)
	{
	    /* didn't find anything */
	    mdebug1(("%s:no comp found,ret NULL\n", "getnextcomp"));
	    return(NULL);
	}
    }
    mdebug3(("%s:*stradr (%#x) >>%s<< q >>%s<<\n",
	    "getnextcomp",*stradr,*stradr,q));
    return(q);              /* return pointer to start of component */
}

/* ---------------------------------------------------------------------------
   iNum2Str - Convert an inode number to a string
*/
LOCAL
int
iNum2Str(iNum, iNumBuf)
ino_t		iNum;			/* Inode number to translate */
char		*iNumBuf;		/* The result buffer */
{
    char         *iNumFill = iNumBuf;   /* Fill the result buffer */
    char         *introGet;             /* Pick up introducer char */
    long         lNum = (long) iNum;    /* Long version of inode number */

    /* Introducer character encodes (iNum / (INUM_RADIX ** 3)) */
    for (introGet = introChars; lNum > IR_3; lNum -= IR_3, introGet++)
    {
	/* There are a limited number of introducer characters */
	if (*introGet == '\0')
	    break;
    }
    /* Inode number too big - file is off limits */
    if (*introGet == '\0')
    {
	*iNumBuf = '\0';
	return 0;
    }
    /* Put introducer character in buffer */
    *iNumFill++ = *introGet;

    /* Expand (iNum % (INUM_RADIX ** 3)) to a <= 3 digit "number" */

    /* Most significant digit */
    if (lNum >= IR_2)
    {
	*iNumFill++ = iNumChars[lNum / IR_2];
	lNum %= IR_2;
    }

    /* Medium significant digit */
    if (lNum >= INUM_RADIX || iNumFill - iNumBuf > 1)
    {
	*iNumFill++ = iNumChars[lNum / INUM_RADIX];
	lNum %= INUM_RADIX;
    }

    /* Put in least significant digit and terminate string */
    *iNumFill++ = iNumChars[lNum];
    *iNumFill = '\0';

    /* Return length of string */
    return iNumFill - iNumBuf;
}
