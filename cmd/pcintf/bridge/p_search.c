/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_search.c	1.1"

#include	<system.h>
#include	<sccs.h>
SCCSID(@(#)p_search.c	3.28	LCC);	/* Modified: 16:31:14 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	<pci_types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<pwd.h>

#include	<string.h>
#include	<memory.h>

/*
**  MAXNAMLEN must be defined for xdir.h as well as p_search.c, and it
**  normally is in <dirent.h>, but xdir.h and dirent.h both typedef DIR.
**  So define it explicitly for now.
*/

#define	MAXNAMLEN	512
#include	<xdir.h>

#ifdef 	XENIX 	/* to avoid hard to find run-time barfs */
#define sio 	output
#endif	/* XENIX */

#define	sDbg(dbgArg)	debug(0x20, dbgArg)
#define	bDbg(dbgArg)	debug(0x20,dbgArg)

extern  void set_offset();
extern  void ftslash();
extern  void uppercase();
extern  void add_pid();         /* Updates process id of a search context */
extern  void add_mode();        /* Updates mode of a search context */
extern  void add_offset();      /* Updates rd/wrt pointer in srch vector */
extern  void add_pattern();     /* Updates search pattern in search vector */
extern  void add_attribute();   /* Updates file attribute in search vector */


extern  int xmtPacket();	/* Datalink layer: sends output frames */
extern  int btime();            /* Translate time into MS-DOS format */
extern  int bdate();            /* Translate date into MS-DOS format */
extern  int match();            /* Pattern matching function */
extern  int add_dir();          /* Adds a new directory context to table */
extern  int get_attr();         /* Rets the attribute of a search context */
extern  int attribute();        /* Build MS-DOS attribute byte */
extern  int match_dir();        /* Rets logical search id else -1 */
extern  int mapfilename();      /* Translate UNIX name into legal DOS name */
extern  int unmapfilename();    /* Unmaps PCI name back to UNIX filename */
extern	int get_vdescriptor();	/* Returns the vdesc assigned to given inode */
extern  int errno;


extern  char *myhostname();	/* Retrieve name of host */
extern  char *get_pattern();    /* Retrieve pattern from state vector */
extern  char *getpname();       /* Retrieve pathname from state vector */

extern  long snapshot();        /* Returns and stores dir offset of search */
extern	long get_dos_time();	/* Returns virtual DOS stamp for given file */

extern  DIR *swapin_dir();      /* Swap-in directory context */

extern  struct sio *getbufaddr();       /* Address of the read-ahead buffer */

extern  struct tm *localtime();         /* Load file date into tm structure */

extern  struct  passwd *getpwuid();     /* Returns pointer to passwd struct */


/*			    External Variables				*/



extern  int swap_how;           /* How to swap output packets */
extern  int brg_seqnum;         /* Sequence number of bridge frame */
extern  int request;            /* Request type of bridge frame */
extern  int outputframelength;  /* Length of last output frame */


extern  struct output *optr;             /* Pointer to last output frame sent */


extern  struct ni2 ndata;       /* Ethernet header */


extern  struct output out1;     /* Output frame buffers */

#ifdef	JANUS
int  doingfunny = FALSE;        /* boolean: dealing with funny name or not */
#endif	/* JANUS */

/* convert any string to a legal sized filename */
boil_name(blanched, raw, mode)
    register char       *blanched; /* where to put legal filename/pattern */
    register char       *raw;  /* filename/pattern (may be in illegal form) */
{
    int ii;

#ifdef	DEBUG
    char *ablanched = blanched;
    char *araw = raw;
#endif	/* DEBUG */

    bDbg(("boilname:in blanched %s, raw %s, mode 0x%x\n",
		ablanched,araw,mode));
	/*
	   a legal name has: 
		Eight char name and optional extension,
		which is: a dot followed by at most three letters
		(and ended by a null)
	*/

    if (mode == MAPPED)
    {
	for (ii = 0; ii < 8 && *raw != '\0' && *raw != '.'; ii++)
	    *blanched++ = *raw++;

	/* scan for a dot, if any */
	for (; *raw && *raw != '.'; raw++)
	    ; /*empty*/

	if (*raw == '.')
	    for (ii = 0; ii < 4 && *raw; ii++)
		*blanched++ = *raw++;

	*blanched = '\0';
    } 
	else
    {
#ifdef	BERK42FILE
#define	maxComponent	MAXDIRLEN
#else
#define	maxComponent	DIRSIZ
#endif
		for (ii = 0; ii < maxComponent && *raw != '\0'; ii++)
			*blanched++ = *raw++;
		*blanched = '\0';
    }
    bDbg(("boilname:out blanched %s, raw %s, mode 0x%x\n",
	ablanched,araw,mode));
}

char *
getdir_entry(responseptr, dirptr, pathcomponent, namecomponent, hiddenflg,
     subdirflg, mode, filstatptr
#ifdef	MULT_DRIVE
    , drvNum
#endif
    )
    char        *responseptr;       /* Pointer to response buffer */
    DIR         *dirptr;            /* Pointer to opendir() structure */
    char        *pathcomponent;     /* Pathname component of srch pattern */
    char        *namecomponent;     /* Filename component of srch pattern */
    int         hiddenflg;          /* Match on hidden files */
    int         subdirflg;          /* Match on subdirectorys */
    int         mode;               /* Match on Mapped/Unmapped filenames */
    struct stat *filstatptr;        /* Pointer to stat() structure */
#ifdef	MULT_DRIVE
    int         drvNum;
#endif
{

    register struct direct *direntryptr;   /* Pointer to opendir() entry */
    char mappedname[MAX_FILENAME];   	/* mapped version of candidate
				      					*  directory entries
				      					*/
    char statname[MAX_PATH];         /* Name of file to stat() */

#if defined(MULT_DRIVE) && defined(SCREWEDUP)
    chdir(pathcomponent);
#endif

    while ((direntryptr = readdir(dirptr)) != NULL)
    {
		if (!hiddenflg && direntryptr->d_name[0] == '.'
	  	&&  (!is_dot(direntryptr->d_name))
	  	&& (!is_dot_dot(direntryptr->d_name)) )
	    	continue;

		strcpy(mappedname,direntryptr->d_name);
		if (
#ifdef	JANUS
            !doingfunny &&
#endif
	    mode == MAPPED)
	    	mapfilename(pathcomponent,mappedname);
		if (match(mappedname, namecomponent, mode) == FALSE)
	    	continue;

#if defined(MULT_DRIVE) && defined(SCREWEDUP)
		if (stat(direntryptr->d_name, filstatptr) < 0)
	    	continue;
#else
    /* Construct pathname for stat() */
		strcpy(statname, pathcomponent);
		strcat(statname, "/");
		strcat(statname, direntryptr->d_name);

		if ((stat(statname, filstatptr)) < 0)
	    	continue;
#endif
		if ((!subdirflg) && ((filstatptr->st_mode & S_IFMT) == S_IFDIR))
	    	continue;

		strcpy(responseptr, mappedname);
		return responseptr;
    }
    return NULL;
}



/*
 *	format_response() -	Returns a filename in MAPPED or UNMAPPED form.
 */

int
format_response(directory, filename, mode, uid)
    char 	*directory;		/* Directory of file */
    char 	*filename;		/* Filename to map */
    int		mode;			/* Indicates Mapped/Unmapped format */
    unsigned short uid;			/* User id */
{
    register int        stringlen;      /* Length of filename to map */
    register int        mappedlen;      /* Length of mapped filename */
    register struct passwd *passptr;    /* Pointer to password struct */
    char *stringptr,
	 *userptr;
    char mappedname[MAXNAMLEN];


    sDbg(("format_response:in(dir=%s filename=%s mode=%d. uid=%d.)\n",
	directory, filename, mode, uid));
/* Send response in either Mapped or Unmapped or Funny form */
    if (mode == MAPPED)
    {
		mapfilename(directory, filename);
		sDbg(("format_response: mapped name: %s\n",filename));
		stringlen = strlen(filename);
		uppercase(filename, stringlen);
		ftslash(filename, stringlen);
    }
    else
    {
		stringlen = strlen(filename);
#ifdef	BERK42FILE
		if (stringlen >= MAXDIRLEN)
	    	stringlen = MAXDIRLEN;
#else
		if (stringlen >= DIRSIZ)
	    	stringlen = DIRSIZ;
#endif /* BERK42FILE */
		filename[stringlen] = '\0';
		ftslash(filename, stringlen);
		stringptr = userptr = filename+stringlen+1;
		if ((passptr = getpwuid((int) uid)) != NULL)
		{
			if (passptr->pw_name && (*passptr->pw_name))
		    	strcpy(stringptr, passptr->pw_name);
			else 
			{
				sDbg(("format_resp:pwname %x;uid %d.\n",
					passptr->pw_name,uid));
				sprintf(stringptr,"%d",uid);
			}
	
		} 
		else 
			sprintf(stringptr,"%d",uid);
		stringlen=stringlen+strlen(filename+stringlen+1)+1;
		filename[stringlen] = '\0';
		stringptr = filename+stringlen+1;

		strcpy(mappedname,filename);
		mapfilename(directory, mappedname);
		mappedlen = strlen(mappedname);
		uppercase(mappedname, mappedlen);
		ftslash(mappedname, mappedlen);
		strcpy(stringptr, mappedname);
		stringlen=stringlen+strlen(filename+stringlen+1)+1;

		sDbg(("format_resp:filename %s,username %s, mappedname %s\n",
		filename,userptr,filename+stringlen+1));
		sDbg(("format_resp:stringlen %d\n",stringlen));
    }
    sDbg(("format_response:out(dir=%s filename=%s mode=%d. uid=%d.)\n",
	directory, filename, mode, uid));
    return(stringlen);
}


struct	sio *
stat_n_statahead(saddr, dirptr, search_id, pathcomponent, namecomponent,
     mode, filstatptr
#ifdef	MULT_DRIVE
     , drvNum
#endif
     )
    struct sio  *saddr;                 /* Points to read-ahead output buff */
    DIR		*dirptr;		/* Pointer to opendir() structure */
    int		search_id;		/* Logical search identifier */
    char	*pathcomponent;		/* Pathname component */
    char        *namecomponent;         /* Filename cmpnt of search pattern */
    int		mode;			/* Mode is Mapped or Unmapped */
    struct stat *filstatptr;            /* Pointer to stat() buffer */
#ifdef	MULT_DRIVE
    int         drvNum;
#endif
{
    register int        hflg;           /* Hidden attributes */
    register int        sflg;           /* Hidden and subdir attributes */
    register int        stringlen;      /* Length of response string */
    register int	vdesc;		/* virtual desc for given open file */
    struct tm           *timeptr;       /* Pointer to localtime() struct */
    long		dos_time_stamp;	/* virtual DOS time for given file */

    memset(&saddr->hdr, 0, sizeof(struct header));

    hflg  = ((char)get_attr(search_id) & HIDDEN) ? TRUE : FALSE;
    sflg  = ((char)get_attr(search_id) & SUB_DIRECTORY) ? TRUE : FALSE;

/* Move next matching directory entry into output buffer */
    if (getdir_entry(saddr->text, dirptr, pathcomponent, namecomponent,
	hflg, sflg, mode, filstatptr
#ifdef	MULT_DRIVE
	,drvNum
#endif
	) == NULL)
    {
		sDbg(("statahead: NO_MORE_FILES\n"));
		saddr->hdr.res  = NO_MORE_FILES;
		saddr->hdr.stat = NEW;
		saddr->pre.select = BRIDGE;
		saddr->hdr.fdsc = search_id;
#ifndef BACKOUT
		/* the statahead must be the NO_MORE_FILES also */
		memcpy(getbufaddr(search_id), saddr,(int)(saddr->hdr.t_cnt + HEADER));
#endif
		del_dir(search_id);
		return saddr;
    }

/* Format output according to mode: Mapped or Unmapped */
    stringlen = format_response(pathcomponent, saddr->text, mode,
	filstatptr->st_uid);
#ifdef	JANUS
    if (doingfunny)
    {
		toofunny(saddr->text);
		stringlen = strlen(saddr->text);
		doingfunny = FALSE;
    }
#endif	/* JANUS */

/* Fill-in response frame and send it */
    saddr->hdr.res = SUCCESS;
    saddr->hdr.req = request;
    saddr->hdr.seq = brg_seqnum;
    saddr->pre.select = BRIDGE;
    saddr->hdr.stat = NULL;
    saddr->hdr.fdsc = search_id;
    saddr->hdr.t_cnt = stringlen + 1;
    saddr->hdr.offset = telldir(dirptr);
    saddr->hdr.mode = filstatptr->st_mode;
    saddr->hdr.f_size = filstatptr->st_size;
#ifdef HIDDEN_FILES
    saddr->hdr.attr = attribute(saddr->text,filstatptr);
#else
    saddr->hdr.attr = attribute(filstatptr);
#endif /* HIDDEN_FILES */

    /* if file is open (appears in virtual file cache), use virtual DOS stamp */
    if ((vdesc = get_vdescriptor (filstatptr->st_ino)) >= 0) {
        dos_time_stamp = get_dos_time (vdesc);
		timeptr = localtime (&dos_time_stamp);
    } 
	else												/* else use */
		timeptr = localtime (&(filstatptr->st_mtime));	/* UNIX time stamp */

    saddr->hdr.date = bdate(timeptr);
    saddr->hdr.time = btime(timeptr);
    snapshot(search_id);

    optr = (struct output *)saddr;

    outputframelength = xmtPacket(optr, &ndata, swap_how);
    sDbg(("about to do statahead search\n"));
    saddr = getbufaddr(search_id);
    memset(&saddr->hdr, 0, sizeof(struct header));

/* Move next matching directory entry into stat ahead buffer */
    if (getdir_entry(saddr->text, dirptr, pathcomponent, namecomponent,
	hflg, sflg, mode, filstatptr
#ifdef	MULT_DRIVE
	,drvNum
#endif
	) == NULL)
    {
		sDbg(("statahead: NO_MORE_FILES\n"));
		saddr->hdr.res    = NO_MORE_FILES;
		saddr->hdr.stat   = NEW;
		saddr->pre.select = BRIDGE;
		saddr->hdr.fdsc   = search_id;
		saddr->hdr.offset = telldir(dirptr);
		del_dir(search_id);			/* delete the context */
		return NULL;
    }

    sDbg(("building stata response\n"));
    stringlen = format_response(pathcomponent, saddr->text, mode,
	filstatptr->st_uid);

/* Fill-in stat-ahead response and return */
    saddr->hdr.res = SUCCESS;
    saddr->hdr.req = request;
    saddr->hdr.seq = brg_seqnum;
    saddr->pre.select = BRIDGE;
    saddr->hdr.stat = NULL;
    saddr->hdr.fdsc = search_id;
    saddr->hdr.t_cnt = stringlen + 1;
    saddr->hdr.offset = telldir(dirptr);
    saddr->hdr.mode = filstatptr->st_mode;
    saddr->hdr.f_size = filstatptr->st_size;
#ifdef HIDDEN_FILES
    saddr->hdr.attr = attribute(saddr->text,filstatptr);
#else
    saddr->hdr.attr = attribute(filstatptr);
#endif /* HIDDEN_FILES */

    /* if file is open (appears in virtual file cache), use virtual DOS stamp */
    if ((vdesc = get_vdescriptor (filstatptr->st_ino)) >= 0) 
	{
        dos_time_stamp = get_dos_time (vdesc);
		timeptr = localtime (&dos_time_stamp);
    } 
	else						/* else use */
		timeptr = localtime (&(filstatptr->st_mtime));	/* UNIX time stamp */

    saddr->hdr.date = bdate(timeptr);
    saddr->hdr.time = btime(timeptr);
    return NULL;
}

/* 
 * If the search pattern for a search first call does not contain any
 * wildcard characters we take a fast path and just stat the specified     
 * file.  No search context is created, and a subsequent search next     
 * will fail.  (As it would have anyway.)     
 *     
 */     
struct sio *
fast_path(saddr, pathcomp, namepat, mode, attr, pid, request)
struct sio *saddr;
char *pathcomp, *namepat;
int mode, attr;
int pid, request;
{
    char fulltemp[MAX_PATH], nametemp[200];
    struct stat statbuf;
    int stringlen;
    int vdesc;			/* virtual descriptor for given open file */
    struct tm *timeptr;
    long dos_time_stamp;	/* virtual DOS time stamp for given open file */
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */
    
    strcpy(nametemp,namepat);
    if (mode != UNMAPPED) {
#ifdef HIDDEN_FILES
		attrib = attr;
		if (unmapfilename(pathcomp,nametemp,&attrib) == DUP_FILE_IN_DIR)
#else
		if (!unmapfilename(pathcomp,nametemp))
#endif /* HIDDEN_FILES */
		{
	    	saddr->hdr.res = PATH_NOT_FOUND;
	    	sDbg(("fast_path: can't unmap %s, %s\n",pathcomp,nametemp));
	    	return saddr;
		}
	sDbg(("Unmap nametemp: %s\n",nametemp));
    }
    strcpy(fulltemp,pathcomp);
    strcat(fulltemp,"/");
    strcat(fulltemp,nametemp);
    sDbg(("fast_path: filename: %s\n",fulltemp));
    if (stat(fulltemp,&statbuf)) 
	{
		*strrchr(fulltemp,'/') = 0;
		if (fulltemp[0] && stat(fulltemp,&statbuf))
	    	saddr->hdr.res = PATH_NOT_FOUND;
		else
	    	saddr->hdr.res = NO_MORE_FILES;
		sDbg(("fast_path: Can't stat %s (%d)\n",fulltemp,errno));
		return saddr;
    }
    else 
	{
		/* if directory then if (attr not set) then rtn NO_MORE_FILES */
		if (statbuf.st_mode & S_IFDIR)
			if (!(attr & SUB_DIRECTORY))
				{
					saddr->hdr.res = NO_MORE_FILES;
					return saddr;
				}
	
	}

/* Fill-in response frame */
    
    strcpy(nametemp,namepat);
    stringlen = format_response(pathcomp,nametemp,mode,
	(unsigned short) statbuf.st_uid);
    memcpy(saddr->text,nametemp,stringlen+1);
    saddr->hdr.res = SUCCESS;
    saddr->hdr.t_cnt = stringlen + 1;
    /* If this was a SEARCH he could change the context and then do a
     * NEXT_SEARCH with a different pattern, so we might need the
     * search context.  PC-DOS TREE does this.  With FIND_FIRST he
     * would find it more difficult to do this.
     */

    if (request == SEARCH) 
	{
	/*    saddr->hdr.fdsc = add_dir(pathcomp, namepat, mode, attr, pid); */
	    saddr->hdr.fdsc = -1;
	    saddr->hdr.offset = 65535L;	/* flag to pci_snext offset */
    } 
	else 
	{
	    saddr->hdr.fdsc = -1;
	    saddr->hdr.offset = 0;
    }
    saddr->hdr.mode = statbuf.st_mode;
    saddr->hdr.f_size = statbuf.st_size;
#ifdef HIDDEN_FILES
    saddr->hdr.attr = attribute(saddr->text,&statbuf);
#else
    saddr->hdr.attr = attribute(&statbuf);
#endif /* HIDDEN_FILES */

    /* if file is open (appears in virtual file cache), use virtual DOS stamp */
    if ((vdesc = get_vdescriptor (statbuf.st_ino)) >= 0) {
        dos_time_stamp = get_dos_time (vdesc);
	timeptr = localtime (&dos_time_stamp);
    } else						/* else use */
	timeptr = localtime (&(statbuf.st_mtime));	/* UNIX time stamp */

    saddr->hdr.date = bdate(timeptr);
    saddr->hdr.time = btime(timeptr);
    return saddr;
}


/*
 * The MS-DOS Search First/ Search Next & Find-First/Find-Next commands
 * use a "read ahead" algorithm which causes the transaction response of
 * the next Find-Next/Search-Next to be built before the request arrives.
 * Since the UNIX server must support simultaneous search contexts
 * the frame to follow is built and stored in the search context state vector.
 */

struct	sio	*
pci_sfirst(filename, request, mode, attr, pid
#ifdef	MULT_DRIVE
    ,drvNum
#endif
    )
    char	*filename;		/* Search pattern */
    int		request;		/* Distinguishes SEARCH/FIND */
    int		mode;			/* Mode of search (Mapped/Unmapped) */
    int		attr;			/* MS-DOS file attribute */
    int         pid;                    /* Id of DOS process calling srch */
#ifdef	MULT_DRIVE
    int         drvNum;
#endif
{
    register int        search_id;
    register struct sio *saddr;         /* Points to read-ahead output buff */
    register char       *slashptr;      /* Pointer to last slash in path */
    DIR                 *dirptr;        /* Pointer to opendir() structure */
    struct stat         filstat;        /* Stat structure */
    char                pathcomp[MAX_PATH];
    char                namepat[MAX_PATTERN];
#ifdef HIDDEN_FILES
	int attrib;
#endif /* HIDDEN_FILES */

/* Translate MS-DOS search pattern to UNIX */
    saddr  = (struct sio *)&out1;
    if (mode != UNMAPPED) cvt2unix(filename);
    else bkslash(filename);

#ifdef	JANUS
    doingfunny = fromfunny(filename); /* this translates funny names, as */
#endif  /* JANUS            well as autoexec.bat, con, ibmbio.com, ibmdos.com  */

    sDbg(("pci_sfirst:filename=%s\n", filename));
/* Return uname() as MS-DOS local volume */
    if (attr == VOLUME_LABEL)
    {

#if	(!defined(IX370) || defined(LOCUS))
		strcpy(saddr->text,myhostname());
#else	/* IX370 && !LOCUS */
		strcpy(saddr->text, "IX/370");
#endif	/* IX370 && !LOCUS */

		saddr->hdr.res = SUCCESS;
		saddr->hdr.stat = NEW;
		saddr->hdr.t_cnt = strlen(saddr->text) + 1;
		saddr->hdr.attr = VOLUME_LABEL;
		sDbg(("pci_sfirst:Volume=%s\n", saddr->text));
		return saddr;
    }

/* Split search pattern into directory component and filename component */
    if ((slashptr = strrchr(filename, '/')) == NULL)
    {
		boil_name(namepat, filename, mode);
		strcpy(pathcomp, CurDir);
    }
    else
    {
		boil_name(namepat, slashptr + 1, mode);
		if (slashptr == filename)
	    	strcpy(pathcomp, "/");
		else
		{
	    	strncpy(pathcomp, filename, slashptr-filename);
	    	pathcomp[slashptr-filename] = '\0';
		}
    }
    
    if (mode != UNMAPPED) {
#ifdef HIDDEN_FILES
		attrib = attr;
		if (unmapfilename(CurDir,pathcomp,&attrib) == DUP_FILE_IN_DIR)
#else
		if (!unmapfilename(CurDir,pathcomp))
#endif /* HIDDEN_FILES */
		{
	    	saddr->hdr.res = PATH_NOT_FOUND;
	    	sDbg(("pci_sfirst: could not unmap: %s\n",pathcomp));
	    	return saddr;
        }
    }

    sDbg(("pci_sfirst:p=%s n=%s\n", pathcomp, namepat));

    
    if (
#ifdef JANUS
	 !doingfunny &&
#endif
	 !wildcard(namepat,strlen(namepat))
       )
        return fast_path(saddr, pathcomp, namepat, mode, attr, pid, request);

    /* check for read and execute (search) permission of directory */
    if (access(pathcomp, 05) != 0)
    {
		saddr->hdr.res = PATH_NOT_FOUND; /* means invalid path */
		sDbg(("pci_sfirst: No access to directory.\n"));
		return saddr;
    }

/* Is it really a directory? */
    stat(pathcomp, &filstat);
    if ((filstat.st_mode & S_IFMT) != S_IFDIR)
    {
		saddr->hdr.res = PATH_NOT_FOUND; /* means invalid path */
		sDbg(("pci_sfirst: Directory component isn't a directory\n"));
		return saddr;
    }

/*
 * There used to be a notice of SPAM here. The notice said that there
 * was no way to tell from a DOS command line when a context should be
 * removed. I differ. Contexts are removed when (a) a process exits or
 * (b) NO_MORE_FILES is returned on a given context.
 *
 * As a result, all search/find firsts will create new contexts where
 * before they would have re-used a context if the directory was the same.
 * The only problem one can run into is some program performing many
 * search/find firsts without completing the searchs on every context
 * and never exiting.
 */

    if ((search_id = add_dir(pathcomp, namepat, mode, attr, pid)) < 0)
    {
    	saddr->hdr.res = PATH_NOT_FOUND;
		sDbg(("pci_sfirst:could not add_dir :%d\n", search_id));
		return saddr;
    }

    if ((dirptr = swapin_dir(search_id)) == NULL)
    {
		saddr->hdr.res = FAILURE;
		del_dir(search_id);		/* delete context */
		sDbg(("pci_sfirst:could not swapin_dir(%d)\n", search_id));
		return saddr;
    }

    saddr = (struct sio *)&out1;

    sDbg(("pci_sfirst:about to statahead\n"));
/* Send first matching filename and then build next response frame */
    if ((saddr = stat_n_statahead(saddr, dirptr, search_id, pathcomp,
	namepat, mode, &filstat
#ifdef	MULT_DRIVE
	, drvNum
#endif
       )) == NULL)
		return NULL;
    else
		return saddr;
}



/*
 * The "search next" command follows the "search" command utilizing: 
 * path, state, addr, optr, and toggle as state variables.
 * Search next commands are issued by MS-DOS until a NO_MORE_FILES
 * response is sent.
 */

struct	sio	*
pci_snext(filename, stringlen, request, search_id, offset, attr
#ifdef	MULT_DRIVE
    , drvNum
#endif
    )
    char        *filename;      /* Search pattern */
    int         stringlen;      /* Length of filename string */
    int         request;        /* Distinguishes between SEARCH/FIND */
    int         search_id;      /* Logical search context identifier */
    long        offset;         /* Offset into directory */
    int         attr;           /* MS-DOS filename search attribute */
#ifdef	MULT_DRIVE
    int         drvNum;
#endif
{
    int             hflg;           /* MS-DOS filename attributes */
    int             sflg;           /* MS-DOS filename attributes */
    int             srchmode;       /* Mode of a search (MAPPED/UNMAPPED) */
    int		    vdesc;	    /* virtual descriptor for given open file */
    long	    dos_time_stamp; /* virtual DOS time stamp for given file */
    char            *slashptr;
    char            *pathname;      /* Pointer to search pathname string */
    char            *pattern;       /* Pointer to search pattern string */
    char            pathcmp[MAX_PATH];
    char            nameptt[13];    /* file name/pattern for matching */
    unsigned char   save_resp;      /* To remember response */
    register struct sio *saddr;     /* Points to read-ahead output buffer */
    register DIR    *dirptr;        /* Pointer to opendir() structure */
    struct tm       *timeptr;       /* Pointer to localtime() struct */
    struct stat         filstat;

/* Is there an existing search context? */
    if ((dirptr = swapin_dir(search_id)) == NULL)
    {
		saddr = (struct sio *)&out1;
		saddr->hdr.res = NO_MORE_FILES;
		if (offset == 65535L) 
		{
			saddr->hdr.fdsc = search_id;
			saddr->hdr.offset = 65535L;
		}
		sDbg(("pci_snext:could not swapin_dir(%d)\n", search_id));
		return saddr;
    }

    if ((srchmode = get_mode(search_id)) != UNMAPPED)
	cvt2unix(filename);
    else
	bkslash(filename);

/* Search-Next calls can update a search context */

    if (request == NEXT_SEARCH)
    {
    /*
     * here is where we check for a "fast pathed" search next.
     */
    if (offset == 65535L) 
	{
		char bar[32];
		sDbg(("pci_snext: search_next after fastpath search_first\n"));
		add_offset(search_id, 0L);
		hflg = ((char)get_attr(search_id) & HIDDEN) ? TRUE : FALSE;
		sflg = ((char)get_attr(search_id) & SUB_DIRECTORY) ? TRUE:FALSE;
		srchmode = get_mode(search_id);
		pattern = get_pattern(search_id);
		pathname = getpname(search_id);
		getdir_entry(bar, dirptr, pathname, pattern, hflg, sflg,
		srchmode, &filstat
#ifdef	MULT_DRIVE
			, drvNum
#endif
			);
		offset = telldir(dirptr);
     }

    /*
     * If text count indicates there is data, examine input text and form a
     * pathname for search directory and filename/search pattern.
     */
	if (stringlen)
	{
#ifdef	MULT_DRIVE
	    filename = fnQualify(filename, CurDir);
#endif	/* MULT_DRIVE */
	    if ((slashptr = strrchr(filename, '/')) != NULL)
	    {
			boil_name(nameptt, slashptr + 1, srchmode);
			if (slashptr == filename)
		    	strcpy(pathcmp, "/");
			else
			{
		    	strncpy(pathcmp, filename, slashptr - filename);
		    	pathcmp[slashptr-filename] = '\0';
			}
	    }
#ifndef	MULT_DRIVE
	    else
	    {
			boil_name(nameptt, filename, srchmode);
			strcpy(pathcmp, getpname(search_id));
	    }
#endif
	    stringlen = strlen(nameptt);
	}
	if (stringlen)
	    sDbg(("pci_snext:pc=%s nc=%s\n", pathcmp, nameptt));

	if (same_context(search_id, (stringlen) ? nameptt : "",
	    offset, attr) == 0)
	{
	    if (stringlen)
			add_pattern(search_id, nameptt);
	    add_offset(search_id, offset);
	    add_attribute(search_id, attr);
	    srchmode = get_mode(search_id);
	    saddr = (struct sio *)&out1;
	    sDbg(("pci_snext:call stata\n"));

	    if ((saddr = stat_n_statahead(saddr, dirptr, search_id,
		pathcmp, nameptt, srchmode, &filstat
#ifdef	MULT_DRIVE
		, drvNum
#endif
	       )) == NULL)
	    {
			return NULL;
	    }
	    return saddr;
	}
    }

/* The request == NEXT_FIND, or is a same context NEXT_SEARCH */


/* Transfer read-ahead frame into an output buffer and send */

    saddr = getbufaddr(search_id);
#if !defined(UDP42) && !defined(UDP41C)
    (void) memcpy((char *)&out1, (char *)saddr, 
					(int)(saddr->hdr.t_cnt + HEADER));
#else   
    (void) memcpy((char *)&out1.pre, (char *)&saddr->pre,
					(int)(saddr->hdr.t_cnt + HEADER));
#endif  /* !UDP42 && !UDP41C */
    out1.hdr.seq = (char)brg_seqnum;
    out1.hdr.req = (char)request;
    set_offset(search_id, out1.hdr.offset);
    optr = &out1;
    save_resp = saddr->hdr.res;

    outputframelength = xmtPacket(optr, &ndata, swap_how);

    memset(&saddr->hdr, 0, sizeof(struct header));

/* When did not have a successful find, no use continuing search */
    if (save_resp != SUCCESS)
    {
#ifdef OLD_WAY
	sDbg(("pci_snext:Not doing stata\n"));
	saddr->hdr.res = save_resp;
	saddr->hdr.req = request;
	saddr->hdr.seq = brg_seqnum;
	saddr->hdr.stat = NEW;
	saddr->pre.select = BRIDGE;
	saddr->hdr.fdsc = search_id;
	saddr->hdr.offset = telldir(dirptr);
#else
	/* 
	No longer using the above code (when OLD_WAY is defined):
	Now instead, the directory search context is deleted.
	The server was found to be growing very large since the del_dir's
	were not done until a process exit occured.  If the process did a 
	lot of searches in different contexts it allocated lots of contexts
	including the packet buffers for stat aheads.  Now the contexts
	are reused as soon as nothing is found in a search.  We don't need
	near as many of them.  Gregc  03-mar-86
	*/
	/*
	I believe that the above code was to fix a badly written
	tree program that continued to do searching after a NO_MORE_FILES.
	With this code removed, now instead of getting more "NO_MORE_FILES"
	on subsequent searches, the response will be "FAILURE". The previous
	way was how DOS treated the situation, but it was extremly wasteful.
	A better way for remembering that a search context is at the end,
	and returning NO_MORE_FILES, is needed. (on the DOS side?)
	In the meantime, this deleting of search contexts should work ok for
	most programs.   Peet 04-mar-86
	*/

	del_dir(search_id);
#endif
	return NULL;
    }

/* Get search context variables */
    hflg = ((char)get_attr(search_id) & HIDDEN) ? TRUE : FALSE;
    sflg = ((char)get_attr(search_id) & SUB_DIRECTORY) ? TRUE:FALSE;
    srchmode = get_mode(search_id);
    pattern = get_pattern(search_id);
    pathname = getpname(search_id);
    sDbg(("pci_snext:own stata\n"));

    if (getdir_entry(saddr->text, dirptr, pathname, pattern, hflg, sflg,
	srchmode, &filstat
#ifdef	MULT_DRIVE
	, drvNum
#endif
       ) == NULL)
    {
		sDbg(("pci_snext:own stata finds: NO_MORE_FILES\n"));
		saddr->hdr.res = NO_MORE_FILES;
		saddr->hdr.req = request;
		saddr->hdr.seq = brg_seqnum;
		saddr->hdr.stat = NEW;
		saddr->pre.select = BRIDGE;
		saddr->hdr.fdsc = search_id;
		saddr->hdr.offset = telldir(dirptr);
		return NULL;
    }

    sDbg(("pci_snext:does format_resp\n"));
    stringlen = format_response(pathname, saddr->text, srchmode,
	filstat.st_uid);

/* Fill-in response header */
    saddr->hdr.res = SUCCESS;
    saddr->hdr.req = request;
    saddr->hdr.seq = brg_seqnum;
    saddr->hdr.stat = NULL;
    saddr->pre.select = BRIDGE;
    saddr->hdr.fdsc = search_id;
    saddr->hdr.t_cnt = stringlen + 1;
    saddr->hdr.offset = telldir(dirptr);
    saddr->hdr.mode = filstat.st_mode;
    saddr->hdr.f_size = filstat.st_size;
#ifdef HIDDEN_FILES
    saddr->hdr.attr = attribute(saddr->text,&filstat);
#else
    saddr->hdr.attr = attribute(&filstat);
#endif /* HIDDEN_FILES */

    /* if file is open (appears in virtual file cache), use virtual DOS stamp */
    if ((vdesc = get_vdescriptor (filstat.st_ino)) >= 0) {
        dos_time_stamp = get_dos_time (vdesc);
	timeptr = localtime (&dos_time_stamp);
    } else						/* else use */
	timeptr = localtime (&(filstat.st_mtime));	/* UNIX time stamp */

    saddr->hdr.date = bdate(timeptr);
    saddr->hdr.time = btime(timeptr);
    return NULL;
}
