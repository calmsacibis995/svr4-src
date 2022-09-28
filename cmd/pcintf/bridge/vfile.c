/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/vfile.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)vfile.c	3.44	LCC);	/* Modified: 16:33:29 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	"pci_types.h"
#include	<errno.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<string.h>
#include	<fcntl.h>

#ifdef  RLOCK   /* record locking */
#include <rlock.h>			/* record locking defs */

extern struct vFile	*validFid();	/* file handle validator */
extern int	rmvOpen();		/* Remove an open from o f table */
extern int 	rstLocks();		/* Remove process's locks from a file */
void 		delete_file();
#endif   /* RLOCK */

#ifdef 	ATT3B2
typedef unsigned short u_short;
#endif

typedef struct vFile	vFile;

#define	NON_ENTRY	((unsigned int)-1)
#define	ANY_INUM	0

#define	RSRV_FCB	5		/* Default number of preserved FCBs */
#define	RSRV_UDESC	5		/* Descriptors reserved for dossvr */

#define	CACHE_SIZE_ENV	"VFCACHE"	/* vfCacheSize environment variable */
#define	RSRV_FCB_ENV	"VFFCB"		/* rsrvFCBs env var */

unsigned		vfCacheSize	/* Number of slots in vfCache */
			 = MAXFILES;
static unsigned		rsrvFCBs	/* Max # slots availabe for FCBs */
			 = RSRV_FCB;
static unsigned		maxUDesc	/* Maximum available UNIX descriptors */
			 = MAXFILES;
static int		nFCBOpens	/* Number of FCB entries in vfCache */
			 = 0;

#define vfDbg(debugArgs)        debug(0x100, debugArgs)

/*			Global variables and functions			*/

extern void
	free();

extern	int
	swap_how,		/* How to swap output packets */
	brg_seqnum,		/* Sequence number of bridge frame */
	request;		/* Request type of bridge frame */

extern	int print_desc[NPRINT];	   /* Print file descriptors */
extern	char *print_name[NPRINT];  /* Print file names */

struct vFile 
	*vfCache;      /* State table data on open files */

extern	long
	time(),
	lseek();

extern	int 
	errno;			/* Error response from system calls */

extern	char
	cwd[],
	*fnQualify(),
	*morememory(),
	*memory();

int
	in_cache,       /* Number of files currently in cache table */
	desc_used;      /* Number of UNIX descriptors currently used */

struct tm
	*localtime();


/*			File Management routines
 *
 *
 * The bridge supports MS-DOS' view of file handles which allows application
 * programs to have up to 100 files opened simutaneouly.  Since UNIX allows
 * a maximum of 20 open files at a time a caching scheme is used to provide
 * MS-DOS with the appropriate view.
 */

/*
    vfInit: Initialize virtual file table
*/

vfInit()
{
register vFile		*initSlot;
int			fdsc;
extern char		*getenv();
char			*vfEnv;		/* Value of File env variable */
char			*envParse;	/* Parse vfEnv with strtol() */
unsigned		envVal;		/* Values parsed out of environment */
unsigned		nUDesc;		/* # of available UNIX descriptors */

	/* Override default parameters with environment variables */
	if ((vfEnv = getenv(CACHE_SIZE_ENV)) != 0
	&&  (envVal = atoi(vfEnv)) > 0)
		maxUDesc = vfCacheSize = envVal;

	if ((vfEnv = getenv(RSRV_FCB_ENV)) != 0
	&&  (envVal = atoi(vfEnv)) > 0)
		rsrvFCBs = envVal;

	/* Find out number of available UNIX descriptors */
	nUDesc = uMaxDescriptors();

	/*
	   Limit UNIX file descriptor usage so as not to
	   run out of descriptors for internal server use.
	*/
	/* ASSERT(nUDesc > RSRV_UDESC); */
	if (maxUDesc > nUDesc - RSRV_UDESC)
		maxUDesc = nUDesc - RSRV_UDESC;

	/* Allocate vfCache array */
	vfCache = (vFile *) calloc(vfCacheSize, (unsigned) sizeof(vFile));
	if (vfCache == 0)
		fatal("vfInit: Can't calloc(%u, %u) for vfCache.",
			vfCacheSize, (unsigned) sizeof(vFile));

	/* Initialize vfCache array */
	for (initSlot = vfCache; initSlot < &vfCache[vfCacheSize]; initSlot++)
		initSlot->flags = VF_INIT;

#ifdef	JANUS
	vfPreset(0, 0, "/dev/tty");
	vfPreset(1, 1, "/dev/tty");
	vfPreset(2, 2, "/dev/tty");
#ifdef OLD_PART
	fdsc = open("/dev/dsk/dos", O_RDWR);
	if (fdsc != -1)
	    vfPreset(3, fdsc, "/dev/dsk/dos");
	else
	{
	    /* Try alternate filename for dos "partition" */
	    fdsc = open("/usr/merge/dosdisk", O_RDWR);
	    if (fdsc != -1)
	        vfPreset(3, fdsc, "/usr/merge/dosdisk");
	    else
	    {
		/* Do not complain about dos partition not available. */
		/* Just use dev/null instead */
		fdsc = open("/dev/null", O_RDWR);
		vfPreset(3, fdsc, "/dev/null");
	    }
	}
#endif  /* OLD_PART */
#endif	/* JANUS */
}


#ifdef	JANUS

/*
   vfPreset: Pre-initialize an open file into the file cache

	This is not very general, it is intended only to
	preset standard in/out/err for janus use.
*/

vfPreset(vDesc, uDesc, fName)
int
	vDesc,				/* Preset this virtual descriptor.. */
	uDesc;				/* ..to this Unix descriptor */
char
	*fName;				/* Nme of pre-opened file */
{
register struct vFile
	*vfSlot;			/* Virtual file cache entry */
struct stat
	vfpStatb;			/* File info */
	int	omode;			/* Open mode */

	/* Get file info */
	if (fstat(uDesc, &vfpStatb) < 0)
		return;

	log("vfPreset: adding %s to vfCache: vDesc %d  uDesc %d\n", fName,
			vDesc, uDesc);

	/* Get pointer to file cache slot of interest */
	vfSlot = &vfCache[vDesc];

	/* Initialize entry */
	vfSlot->uDesc = uDesc;
	vfSlot->flags =
#ifdef RLOCK   /* record locking */
			FF_NOLOCK |    	/* Don't lock this puppy */
#endif  /* RLOCK */
			VF_INUSE | VF_CERTAIN | VF_PRESET;  
	do
		omode = fcntl(uDesc, F_GETFL, 0);
	while (omode == -1 && errno == EINTR);
	omode &= VF_OMODE;
	vfSlot->flags |= omode;

#ifdef RLOCK   /* record locking */
	/* Compute bitwise access permission flags */
	if(omode == O_RDONLY)
		vfSlot->flags |= FF_READ;
	else if (omode == O_WRONLY)
		vfSlot->flags |= FF_WRITE;
	else
		vfSlot->flags |= (FF_READ | FF_WRITE);

	vfSlot->sessID = (word)(getpid());	/* current svr pid */
	vfSlot->uniqueID = 0L;		/* file's inode and svr pid */
	vfSlot->shareIndex = -1;	/* entry into global open file table */
#endif  /* RLOCK */

	vfSlot->iNum = vfpStatb.st_ino;
	vfSlot->pathName = fName;
	vfSlot->dosPid = -1;
	vfSlot->rwPtr = 0L;
	in_cache++;
	desc_used++;
}
#endif	/* JANUS */


/*
 * Add_file -		adds a new file to the file table.  It returns a
 *			virtual file descriptor for MS-DOS' use across 
 *			the bridge.
 */

int
add_file(adescriptor, name, mode,
#ifdef  RLOCK   /* record locking */
				 share,
#endif   /* RLOCK */
					inode, pid, unlink_on_sig)

int	adescriptor;			/* Unix file descriptor */
char	*name;				/* File name */
int	mode;				/* Unix file open mode */
#ifdef  RLOCK   /* record locking */
int	share;				/* share mode for file access */
#endif   /* RLOCK */
ino_t	inode;				/* Inode number of file */
int	pid;				/* DOS "process id" */
int	unlink_on_sig;			/* Unlink file on signal? */
{
register unsigned int
	i;				/* Count cache entries */
register struct vFile
	*vdslot;			/* Scan cache entries */
struct stat
	filstat;

#ifdef  RLOCK   /* record locking */
int	altmode;			/* mode as altered by addOpen */
word	rwshare;			/* r/w & share bits for file access */
int	shareslot;			/* Global open file table index */
#endif   /* RLOCK */

   vfDbg(("add_file: udesc: %d \"%s\" md: %#x  i: %d pid: %d uos=%d\n",
	   adescriptor, name, mode, inode, pid, unlink_on_sig));

/* If no room in table, delete the oldest ephemeral cache entry */
    if (in_cache >= vfCacheSize) {
	if ((i = close_file(oldest_ephem(), ANY_INUM)) < 0) {
#ifdef  RLOCK   /* record locking */
/* strictly speaking, rLockerr should only be set by the record */
/* locking routines, but this appears to be harmless. */
	    rlockErr = errno;
#endif   /* RLOCK */
	    close(adescriptor);
	    return -TOO_MANY_FILES;
	}
	vdslot = &vfCache[i];
    } else {
	/* Find next available table entry */
	i =0;
	vdslot = vfCache;
	while ((vdslot->flags & VF_INUSE) && i < vfCacheSize) {
		i++;
		vdslot++;
	}
    }

    if (i >= vfCacheSize) {
	vfDbg(("add_file: out of entries\n"));
#ifdef  RLOCK   /* record locking */
	    rlockErr = ENOMEM;
#endif   /* RLOCK */
	close(adescriptor);
	return -TOO_MANY_FILES;
    }

/* Store the UNIX descriptor, file and pathname, time, mode, and r/w pointer */
    vdslot->uDesc = adescriptor;
    vdslot->lastUse = time((long *)0);
    vdslot->dosPid = pid;
    vdslot->iNum = inode;
    vdslot->rwPtr = 0L;
    vdslot->flags = VF_INUSE
		  | (mode & VF_OMODE)
		  | (unlink_on_sig ? VF_UNLINK 
#ifdef  RLOCK  	/* r/l flag to prevent record locking on a file */
		  		     | FF_SPOOL
	/*  NOTE on FF_SPOOL: Be aware that the value of FF_SPOOL is equal to
	   the O_APEND mode used in opens for opening a file for write with 
	   the pointer at the end of the file.  In Starlan this was used for
	   spool files; here I am using it for PRINT and TEMP (see p_create.c)
	   files.  Future opens which may use these flags could open files for
	   append which should not be. This is probably not a problem here (?)
	   but if it is you'll be glad I wrote this.
	*/
#endif  /* RLOCK */
		  				: 0);
 
#ifdef RLOCK
#ifdef JANUS
	/* If a fake_open on autoexec.bat - don't put in global o\f tab */
	if(share == -1)
	{
 	   vdslot->flags |= FF_SPOOL;
	   share = SHR_DOS;  	/* reset to dos compat for FF_NOLOCK below */
	}
#endif  /* JANUS */
#endif  /* RLOCK */

	fstat(adescriptor, &filstat);	/* get the lastest story from gory */
	vdslot->vdostime = filstat.st_mtime;	/* initialize this */

/* Allocate string to hold full path name and save fully qualified name */
#ifndef MULT_DRIVE
    name = fnQualify(name, cwd);
#endif /* ~MULT_DRIVE */
    vdslot->pathName = memory(strlen(name) + 1);
    (void) strcpy(vdslot->pathName, name);

#ifdef  RLOCK   /* record locking */

	/* set up global open file table entry */
	
	altmode = mode & VF_OMODE;

	/* compute the open and share file accesses */
	rwshare = RW_SHARE(share, mode);

	vdslot->sessID = (word)(getpid());	/* current svr pid */
	vdslot->uniqueID = 0L;		/* file's inode and svr pid */
	vdslot->shareIndex = -1;	/* entry into global open file table */
	vdslot->shareMode = share;

	/* tmp and spool files don't go into the global o/f table */
	if ((vdslot->flags & FF_SPOOL) == 0) 
	{
		/* Add the file to the global open file table (or a header) */
		shareslot = addOpen(adescriptor, &filstat, (long)vdslot->sessID,
		    (long) pid, &altmode, (int) rwshare);
		/* This field is unused, and sUniqID() is no longer available */
		/* since it is internal to the record locking library */
	/*	vdslot->uniqueID = sUniqID(&filstat); */
		vdslot->uniqueID = 0L;
		vdslot->shareIndex = shareslot;

		if (shareslot == -1) {
			close(adescriptor);
    			in_cache++;  	/* compensates for dec in delete_file */
			delete_file(i);
			/* rlockErr is set */
			log("add_file: OPEN DISALLOWED: rlockErr = %d\n",
					rlockErr);
			return -SHARE_VIOLATION;
		}
	}

	/* Don't lock a tmp or spool file, or an FCB old-sytle opened file */
	if ((vdslot->flags & FF_SPOOL) 

     /* || (SHR_BITS(rwshare) == SHR_DOS) /* dos compat opens are inefficient */
     /* If you want efficiency leave the above line in and disallow locks
	for dos compat, which some say is in line with the dos manual */
     /* Otherwise exclude the above condition and allow dos compat locking
	as dos really allows its users to. */

	|| (SHR_BITS(rwshare) == SHR_FCB)) {
		vdslot->flags |= FF_NOLOCK;
	}

	/* If addOpen() reduced access, record new modes and reopen */
	if(altmode != (mode & VF_OMODE)) {
		close(adescriptor);
		vdslot->flags = VF_INUSE | altmode | (mode & FF_SPOOL);
#if	defined(RLOCK) && defined(LOCUS)
		vdslot->uDesc = dosOpen(vdslot->pathName,
					vdslot->flags & FF_REOPEN, 0, share);
#else
		do
			vdslot->uDesc = open(vdslot->pathName,
						vdslot->flags & FF_REOPEN);
		while (vdslot->uDesc == -1 && errno == EINTR);
		if (vdslot->uDesc == -1)
			vdslot->uDesc = -ACCESS_DENIED;
#endif
		if(vdslot->uDesc < 0) {   
			rlockErr = errno;
			rmvOpen(vdslot->shareIndex);
			return vdslot->uDesc;
		}
	}

	/* Compute bitwise access permission flags */
	if(altmode == O_RDONLY)
		vdslot->flags |= FF_READ;
	else if (altmode == O_WRONLY)
		vdslot->flags |= FF_WRITE;
	else
		vdslot->flags |= (FF_READ | FF_WRITE);

#endif   /* RLOCK */

/* Keep count of inuse entries in cache */
    in_cache++;
    vfDbg(("ADD_FILE: added %d; in_cache: %d\n", i, in_cache));
    return i;
}


/*
 * Delete_file -		deletes a file from the open file table.
 */

void
delete_file(vdescriptor)
register unsigned int vdescriptor;
{
    register struct vFile
	*vdslot;

    if (vdescriptor >= vfCacheSize)  {
	vfDbg(("delete_file: %d out of range\n", vdescriptor));
	return;
    }

    vdslot = &vfCache[vdescriptor];

/* If not currently in use, do nothing */
    if (!(vdslot->flags & VF_INUSE)) {
	vfDbg(("delete_file: %d not in use\n", vdescriptor));
	return;
    }

    /* Keep count of FCB opens */
    if (vdslot->flags & VF_EPHEM)
	nFCBOpens--;

/* Clear actual desctriptor, name, time, and current read/write pointer */
    vdslot->uDesc = -1;
    vdslot->flags = VF_INIT;
    if (vdslot->pathName)
	free(vdslot->pathName);
    vdslot->pathName = (char *)0;
    vdslot->lastUse = 0;
    vdslot->dosPid = 0;
    vdslot->rwPtr = 0;
    vdslot->iNum = 0;
#ifdef RLOCK   /* record locking */
    vdslot->uniqueID = 0L;
    vdslot->sessID = (word)0;
    vdslot->shareIndex = (-1);
    vdslot->lockCount = 0;   
#endif  /* RLOCK */

/* Decrement count of files in cache */
    in_cache--;
    vfDbg(("delete_file: %d; in_cache: %d\n", vdescriptor, in_cache));
}



/*
   oldest_swappable:	Return pointer to oldest descriptor that can be
			inactivated (other than vdExcept) or 0 if none.
 */

int
oldest_swappable(vdExcept)
register unsigned int
	vdExcept;		/* Pointer to reserved entry */
{
register struct vFile
	*vdslot;
int
	oldest = -1;		/* Pointer to oldest file in page table */
register unsigned int
	i;			/* Loop counter for page table searches */
long
	oldtime = 0x7fffffff;	/* Oldest element time - init to max future */

/* Search file cache for oldest unlocked entry */
    for (i = 0, vdslot = vfCache; i < vfCacheSize; i++, vdslot++) {
	/* Ignore reserved, unused, locked and inactive entries */
	if (i == vdExcept || !(vdslot->flags & VF_INUSE)
#ifdef RLOCK  /* record locking */
	/* if record is locked, don't return its value - not swappable */
	|| (vdslot->lockCount != 0)
#endif  /* RLOCK */
	|| (vdslot->flags & VF_PRESET) || (vdslot->flags & VF_INACTV))
	    continue;

	if (vdslot->lastUse < oldtime) {
	    oldtime = vdslot->lastUse;
	    oldest = i;
	}
    }

    vfDbg(("oldest_swappable(%d) ==> %d\n", vdExcept,  oldest));
    return oldest;
}


/*
   oldest_ephem:  Return cache index of oldest ephemeral entry
*/

int
oldest_ephem()
{
register int
	oldest = -1;		/* Index of oldest file in cache */
register unsigned int
	i;			/* Loop counter for cache searches */
register long
	oldtime = 0x7fffffff;	/* Oldest entry date - init to max future */
register struct vFile
	*vdslot;

    /* Scan cache finding */
    for (i = 0, vdslot = vfCache; i < vfCacheSize; i++, vdslot++)
 	/* Unless slot is in use for a non-sticky FCB open, ignore it */
	if ((vdslot->flags & VF_INUSE)
	&&  (vdslot->flags & VF_EPHEM)
	&&  !(vdslot->flags & VF_STKYFCB))
		/* If this slot is older, remember it */
		if (vdslot->lastUse < oldtime) {
		    oldtime = vdslot->lastUse;
		    oldest = i;
		}

    vfDbg(("oldest_ephem() ==> %d\n", oldest));
    return oldest;
}



/*
 * swap_out() -	causes an actual UNIX file descriptor to be paged out.
 */

int
swap_out(vdescriptor)
register unsigned int
	vdescriptor;
{
register struct vFile
	*vdslot;

    if (vdescriptor >= vfCacheSize) {
	vfDbg(("swap_out: %d out of range\n", vdescriptor));
	return -1;
    }

    vdslot = &vfCache[vdescriptor];
    vfDbg(("swap_out: %d; %s\n", vdescriptor, vdslot->pathName));

#ifdef RLOCK  /* record locking */
    /* don't swap out a locked file */
    if (vdslot->lockCount != 0) {
	vfDbg(("swap_out: locked\n"));
	return -1;
    }
#endif   /* RLOCK */

    /* This is how it was...
    if (vdslot->flags & VF_PRESET < 0) {
    */

    /* This is what I think he meant... */
    /* '&' has precedence over '<' */
    if ((vdslot->flags & VF_PRESET) != 0) {
	vfDbg(("swap_out: preset file can't be swapped\n"));
	return -1;
    }

    /* Already inactive? */
    if (vdslot->flags & VF_INACTV)
	return vdescriptor;

    vdslot->rwPtr = lseek(vdslot->uDesc, 0L, 1);
    close(vdslot->uDesc);
    vdslot->uDesc = -1;
    vdslot->flags |= VF_INACTV;
    desc_used--;

    return vdescriptor;
}



/*
 * swap_in() -	causes a virtual file to be open.  It returns an actual
 * 		UNIX file descriptor.
 */


int
swap_in(vdescriptor, inode)
register unsigned int
	vdescriptor; 			/* Virtual MS-DOS file descriptor */
register u_short
	inode;				/* Inode number of file */
{ 
register int
	adescriptor;			/* Actual UNIX file descriptor */
struct stat
	filstat;
register struct vFile
	*vdslot;

int newMode;

    if (vdescriptor >= vfCacheSize) {
	vfDbg(("swap_in: %d out of range\n", vdescriptor));
	return NO_FDESC;
    }

    vdslot = &vfCache[vdescriptor];

/* Is there a file entry? */
    if (!(vdslot->flags & VF_INUSE)
    ||  (inode != 0 && (inode != (u_short)(vdslot->iNum))))
    {
	vfDbg(("swap_in: %d not in use or iNum mismatch %d:%d\n", vdescriptor,
		inode, vdslot->iNum));
	return NO_FDESC;
    }

/* Is the cache entry already valid? */
    if (!(vdslot->flags & VF_INACTV)) {
	vfDbg(("swap_in: %d active; uDesc; %d\n", vdescriptor, vdslot->uDesc));
	return vdslot->uDesc;
    }

/* Are there UNIX descriptors available */
    if (desc_used >= maxUDesc)
	if (swap_out(oldest_swappable(vdescriptor)) < 0) {
	    vfDbg(("swap_in: none swappable!\n"));
	    return NO_FDESC;
	}

    vfDbg(("swap_in: open(%s, %d)\n", vdslot->pathName,
	vdslot->flags & VF_OMODE));

#if	defined(RLOCK) && defined(LOCUS)
    if ((adescriptor = dosOpen(vdslot->pathName, vdslot->flags & VF_OMODE,
			0, vdslot->shareMode)) >= 0)
#else
    do
	adescriptor = open(vdslot->pathName, vdslot->flags & VF_OMODE);
    while (adescriptor == -1 && errno == EINTR);
    if (adescriptor >= 0)
#endif
    {
	fstat(adescriptor, &filstat);

	/*
	 * If the inode of the file does not equal the inode in the table
	 * someone unlinked the file!  Decide how to clean-up later.
	 */
	if (filstat.st_ino != vdslot->iNum) {
	    close(adescriptor);
	    vfDbg(("swap_in: iNum mismatch: cache: %d; file %d\n",
		vdslot->iNum, filstat.st_ino));
	    return NO_FDESC;
	}

	desc_used++;
	lseek(adescriptor, vdslot->rwPtr, 0);
	vdslot->uDesc = adescriptor;
	vdslot->lastUse = time((long *)0);
	vdslot->flags &= ~VF_INACTV;
	vfDbg(("swap_in: uDesc: %d\n", adescriptor));
	return adescriptor;
    }

    /* It is possible that the swap_in failed on the open because the file
	mode was changed to READ ONLY after being opened for WRITE access.
	This is the case with AUTOCAD.  What we'll do here is check to see
	if we are supposed to have write access to the file.  If so, and the
	file is READ ONLY (444) then we'll change the mode to WRITE, open the
	file (swap it in) and then restore the file mode.  Messy I know,
	but life's often like that!
    */

    /* check for write permission */
    if (vdslot->flags & VF_OMODE) { 

    	if (stat(vdslot->pathName, &filstat) == 0) {
	vfDbg(("swap_in: file %s mode %x\n",vdslot->pathName,filstat.st_mode));

		/* Is this the file we expect it to be? */
		if (filstat.st_ino != vdslot->iNum) {
	    		close(adescriptor);
	    		vfDbg(("swap_in: iNum mismatch: cache: %d; file %d\n",
				vdslot->iNum, filstat.st_ino));
	    		return NO_FDESC;
		}
		/* Do we have write access to the actual file? */
		if (!(filstat.st_mode & O_WRITE)) {
			/* No WRITE access; it must have changed since opening;
			   chmod to write access */

			newMode = filstat.st_mode | O_WRITE;
			if (chmod(vdslot->pathName, newMode) < 0) 
				goto swap_in_err;

			/* Open the file . . . */
#if	defined(RLOCK) && defined(LOCUS)
    			if ((adescriptor = dosOpen(vdslot->pathName,
						vdslot->flags & VF_OMODE, 0666,
						vdslot->shareMode)) < 0)
#else
			do
    				adescriptor = open(vdslot->pathName,
						vdslot->flags & VF_OMODE);
			while (adescriptor == -1 && errno == EINTR);
    			if (adescriptor < 0)
#endif
			{
				/* Restore original file mode */
				newMode = filstat.st_mode & ~O_WRITE;
				chmod(vdslot->pathName, newMode);
				goto swap_in_err;
			}

			desc_used++;
			lseek(adescriptor, vdslot->rwPtr, 0);
			vdslot->uDesc = adescriptor;
			vdslot->lastUse = time((long *)0);
			vdslot->flags &= ~VF_INACTV;
			vfDbg(("swap_in: uDesc: %d\n", adescriptor));

			/* then change back to read-only */
			newMode = filstat.st_mode & ~O_WRITE;
			if (chmod(vdslot->pathName, newMode) < 0) 
			   log("Swap_in: Can't restore original file mode.\n"); 

			return adescriptor;
		}
	}
    }

swap_in_err:
    vfDbg(("swap_in: open failed %d\n", errno));
    return -1;
}

/*
 * open_file() -	opens a file and returns a virtual MS-DOS descriptor.
 */

int
open_file(name, mode,
#ifdef RLOCK  /* record locking */
			share,
#endif  /* RLOCK */
				pid, request)
char *name;
int mode;
#ifdef RLOCK  /* record locking */
int share;
#endif  /* RLOCK */
int pid;
int request;
{
register int
	adescriptor,		/* Actual file descriptor */
	vdescriptor,		/* Virtual file descriptor */
	i;			/* For loop counter */
struct stat
	filstat;
char
	*fqName;		/* Fully qualified version of name */

/*
 * In MS-DOS, an old style I/O OPEN simply "re-initializes"
 * the contents of an FCB.  Programs often make no explicit closes.
 * Since UNIX restricts the number of files that can be opened
 * simultaneously, the bridge in an old style open allows a file to be
 * opened only once by a specific process.  Since the PC side of the bridge
 * maps old style sequential i/o into random i/o MS-DOS programs will
 * behave correctly.
 *
 * When we find a cache entry with the proper filename, we check the inode 
 * number to verify that the entry really refers to the proper file.  If
 * it does, we return that vdescriptor, otherwise we skip it.
 *
 */

    vfDbg(("open_file: \"%s\"; mode: %d; pid: %d; req: %d\n",
	name, mode, pid, request));
    log("open_file: desc_used = %d\n", desc_used);

    if (request == OLD_OPEN) {

#ifdef MULT_DRIVE
	/* when mult-drive, then name is already been run thru fnQualify */
	fqName = name;
#else  /* ~MULT_DRIVE */
	fqName = fnQualify(name, cwd);
#endif /* ~MULT_DRIVE */

	for (i = 0; i < vfCacheSize; i++)
#if defined(XENIX) || defined(DGUX)
	    if(vfCache[i].pathName != (char *)0) /* avoid xenix barf (strcmp) */
#endif  /* XENIX || DGUX */
	    if (strcmp(vfCache[i].pathName, fqName) == 0) {
		if ((stat(fqName,&filstat) == 0) && (filstat.st_ino == vfCache[i].iNum))
		{
		    vfCache[i].lastUse = time((long *)0);  /* time stamp open */
		    vfDbg(("open_file found in cache %d\n", i));
      		    log("open_file: desc_used = %d\n", desc_used);
		    return i;
		}
		else
		    log("Invalid entry in file cache: idx: %d  name: %s\n",
			i, vfCache[i].pathName);
	    }
    }

/* If there are no available UNIX descriptors swap one out */
    if (desc_used >= maxUDesc)
	if (swap_out(oldest_swappable(NON_ENTRY)) < 0) {
	    vfDbg(("open_file: Nothing swappable\n"));
	    log("open_file: desc_used = %d\n", desc_used);
	    return -TOO_MANY_FILES;
	}

/*
 * If DOS opens file with "not certain" (mode 4), open file for read/write
 * if possible, otherwise open for read only
 */

#ifdef GARDEN_PATH
    if (mode == O_RDWR)
	mode = NOT_CERTAIN;
#endif

    if (mode == NOT_CERTAIN) {
	if (access(fqName,WRITE_ACCESS) == 0)
		mode = O_RDWR;
    	else
		mode = O_RDONLY;
    }

    vfDbg(("open_file: open(%s, %d)\n", name, mode));

#if	defined(RLOCK) && defined(LOCUS)
    if ((adescriptor = dosOpen(name, mode, 0, share)) >= 0)
#else
    do
	adescriptor = open(name, mode);
    while (adescriptor == -1 && errno == EINTR);
    if (adescriptor == -1)
	adescriptor = -ACCESS_DENIED;
    if (adescriptor >= 0)
#endif
    {

#ifdef	LOCUS			/* handle default fstore requirement */
	if (mode & O_CREAT) 	/* need to do this only if file is created */
		dfl_fstore(name);
#endif /* LOCUS */

	desc_used++;
	fstat(adescriptor, &filstat);
	vdescriptor = add_file(adescriptor, name, mode, 
#ifdef RLOCK  /* record locking */
			share,
#endif  /* RLOCK */
		filstat.st_ino, pid, FALSE);

#ifdef  RLOCK   /* record locking */
	if(vdescriptor < 0) {
		desc_used--;
	        log("open_file: desc_used = %d\n", desc_used);
		errno = rlockErr; 
		return vdescriptor;	/* error code */
	}
#endif   /* RLOCK */

	if (request == OLD_OPEN)
		vfCache[vdescriptor].flags |= VF_EPHEM
				| ((nFCBOpens++ < rsrvFCBs) ? VF_STKYFCB : 0);

	vfDbg(("open_file: vDesc: %d; uDesc: %d\n", vdescriptor, adescriptor));
	log("open_file: desc_used = %d\n", desc_used);
	return vdescriptor;
    }

    vfDbg(("open_file: can't open (%d)\n", errno));
    log("open_file: desc_used = %d\n", desc_used);
    return adescriptor;		/* error code */
}

/*
 * create_file() -	creates a file and returns a virtual file descriptor.
 */

int
create_file(name, openMode, pid, unlink_on_sig, dosAttr, reqMode)
char
	*name;
int
	openMode,
	pid,
	unlink_on_sig,
	dosAttr;
{
register int
	vdi;
int
	adescriptor,			/* Actual file descriptor */
	vdescriptor;			/* Virtual file descriptor */
struct stat
	filstat;
struct vFile
	*vdslot;
char
	*fqName;			/* Fully qualified version of name */
long
	create_time;			/* time stamp for the create */
unsigned char
	trans_errno;			/* errno translated to DOS error */
#ifdef HIDDEN_FILES
char hid_name[MAX_PATH];		/* file after conversion to hidden */
char *beg_name;				/* beginning of the filename (w/o dirs) */
char *strcpy(), *strcat();
#endif /* HIDDEN_FILES */

    vfDbg(("create_file: \"%s\"; openMode: %d; pid: %d; reqMode: %d\n",
	name, openMode, pid, reqMode));

/* If there are no available UNIX descriptors, swap one out */
    if (desc_used >= maxUDesc)
	if (swap_out(oldest_swappable(NON_ENTRY)) < 0) {
	    vfDbg(("create_file: nothing swappable\n"));
	    return -TOO_MANY_FILES;
	}

	/* create (open) the file */
	/* DON'T TRUNCATE YET!  We won't know if it's OK to do that */
	/* until add_file() returns. */
	switch (reqMode) {
#if	defined(RLOCK) && defined(LOCUS)
	case PRINT:
		do
			adescriptor = open(name, O_RDWR | O_CREAT,
					(dosAttr & READ_ONLY) ? 0444 : 0666);
		while (adescriptor == -1 && errno == EINTR);
		break;

	case TEMPFILE:
		adescriptor = dosOpen(name, O_RDWR | O_CREAT,
			(dosAttr & READ_ONLY) ? 0444 : 0666);
		chmod(name, S_IREAD | S_IWRITE);
		break;

	default:
		adescriptor = dosOpen(name, O_RDWR | O_CREAT,
			(dosAttr & READ_ONLY) ? 0444 : 0666, SHR_DOS);
		break;

#else	/* +(RLOCK && LOCUS)- */
	case TEMPFILE:
		do
			adescriptor = open(name, O_RDWR | O_CREAT,
					(dosAttr & READ_ONLY) ? 0444 : 0666);
		while (adescriptor == -1 && errno == EINTR);
		chmod(name, S_IREAD | S_IWRITE);
		break;

	default:
#ifdef HIDDEN_FILES
		if (dosAttr & HIDDEN)
		{
			strcpy(hid_name,name);
			/* chop the name out of the hid_name string */
			if ((beg_name = strrchr(hid_name,'/')) == NULL)
				hid_name[0] = '\0';
			else
				*(beg_name+1) = '\0';	
			strcat(hid_name,".");
			if ((beg_name = strrchr(name,'/')) == NULL)
				beg_name = name;
			strcat(hid_name,beg_name);
			name = hid_name;
		}
#endif /* HIDDEN_FILES */
		do
			adescriptor = open(name, O_RDWR | O_CREAT,
					(dosAttr & READ_ONLY) ? 0444 : 0666);
		while (adescriptor == -1 && errno == EINTR);
		break;
#endif
	}	/* switch (reqMode) */

    if (adescriptor >= 0) {
#ifdef	LOCUS			/* handle default fstore requirement */
		dfl_fstore(name);
#endif /* LOCUS */
	desc_used++;
	fstat(adescriptor, &filstat);
	vdescriptor = add_file(adescriptor, name, openMode,
#ifdef RLOCK  /* record locking */
		/* As per the MS Core File Sharing Protocol manual */
		(int)SHR_DOS,
#endif  /* RLOCK */
	    filstat.st_ino, pid, unlink_on_sig);

#ifdef  RLOCK   /* record locking */
	if (vdescriptor < 0)
		return vdescriptor;
#endif   /* RLOCK */

	/* OK, now we can trash it! */
	/* The mode (2nd argument) is ingored since the file exists. */
	close(creat(name, 0666));

	/* Set dos time stamp to UNIX create time */
	vfCache[vdescriptor].vdostime = filstat.st_mtime;

	/* FCB style creates are ephemeral */
	if (reqMode == FCBCREATE)
		vfCache[vdescriptor].flags |= VF_EPHEM
				| ((nFCBOpens++ < rsrvFCBs) ? VF_STKYFCB : 0);

	return vdescriptor;
    }

    if (errno == ENOENT)
	errno = ENOTDIR;

    vfDbg(("create_file: can't create %s (%d)\n", name, errno));
    err_handler(&trans_errno, PCI_CREATE, name);
    return (-(int)trans_errno);
}


/*
 * close_file() -		closes the actual UNIX file descriptor if
 *				there is one and deletes an entry from the
 *				table.
 */

int
close_file(vdescriptor, inode)
register unsigned int
	vdescriptor;
register u_short
	inode;
{
register struct vFile
	*vdslot;
struct stat
	filstat;

    if (vdescriptor >= vfCacheSize) {
	vfDbg(("close-file: vDesc %d is out of range\n", vdescriptor));
	return NO_FDESC;
    }

    vdslot = &vfCache[vdescriptor];

/* If there is an actual descriptor, close it */
    if (vdslot->flags & VF_PRESET) { 
	vfDbg(("close_file: vDesc %d is preset.\n", vdescriptor));
	return NO_FDESC;
    }
    if (!(vdslot->flags & VF_INUSE)) { 
	/* vfDbg(("close_file: vDesc %d is not in use.\n", vdescriptor)); */
	return NO_FDESC;
    }
    if ((inode != 0) && (inode != (u_short)(vdslot->iNum))) {
	vfDbg(("close_file: vDesc %d iNum mis-match %d:%d\n", vdescriptor,
		inode, vdslot->iNum));
	return NO_FDESC;
    }

    if (!(vdslot->flags & VF_INACTV)) {
	fstat(vdslot->uDesc, &filstat);			/* update dos time */
	if (filstat.st_mtime != vdslot->vdostime)	/* only if write since*/
	    vdslot->vdostime = filstat.st_mtime;	/* last time stamp */

	desc_used--;
	close(vdslot->uDesc);
	vfDbg(("close_file: vDesc %d closed %d\n", vdescriptor, vdslot->uDesc));

#ifdef RLOCK  /* record locking */
	/* Free corresponding locks - no entry for tmp, spool, FCB, 
	   and (maybe) dos compat files */
	if ((vdslot->flags & FF_NOLOCK) == 0) {
		/*
		   Free locks on the file being closed.	 This call will
		   (erroneously) release locks placed by the same process
		   ID via other file handles open to this file.	 However,
		   if this isn't done, locks hang around until the process
		   exits, which is worse.  With a bit of work this could
		   be fixed correctly.  (A Starlan comment. - gth) 
		*/

		/* Remove the locks on the file placed by this process */
        	log("close_file: restoring locks: dosPid %u\n", vdslot->dosPid);
		rstLocks((int) vdslot->shareIndex, (long) vdslot->dosPid);
		vdslot->lockCount = 0;
	}
#endif  /* RLOCK */
    }

#ifdef RLOCK   /* record locking */
    /* Free corresponding share table entry 
       		- no entry for tmp, spool or preset files */
    if ((vdslot->flags & (FF_SPOOL | VF_PRESET)) == 0) {
	/* Remove process file header on global open file */
        log("close_file: shareIndex %d\n", vdslot->shareIndex);
        log("close_file: removing open from global open file table\n");
	rmvOpen((int) vdslot->shareIndex);
    }
#endif  /* RLOCK */

    delete_file(vdescriptor);
    return vdescriptor;
}



/*
 * unlink_on_termsig() - 	Deletes all file contexts with delete flag set.
 */

void
unlink_on_termsig()
{
    register int
	i;

    register struct vFile
	*vdslot;

#ifdef JANUS
    merge_cleanup();	/* allow merge stuff to be cleaned up */
#endif	/* JANUS */
/* Spool all open print files.	Note that we don't know where the user
   really wanted them printed, so we will do it at the default place.
*/
    for (i = 0; i < NPRINT; i++) {
	if (print_desc[i] != -1) {
	    s_print(i,0,0,NULL);
	    unlink(print_name[i]);
	    print_desc[i] = -1;
	    print_name[i] = NULL;
	}
    }

/* Deletes all PCI termporary files on termination */
    for (i = 0, vdslot = vfCache; i < vfCacheSize; i++, vdslot++) {
	if ((vdslot->flags & (VF_INUSE | VF_UNLINK
#ifdef RLOCK
	| FF_SPOOL
#endif  /* RLOCK */
	)) == (VF_INUSE | VF_UNLINK
#ifdef RLOCK
	| FF_SPOOL
#endif  /* RLOCK */
	))
	{
	    if (!(vdslot->flags & VF_INACTV)) {
		desc_used--;
		close(vdslot->uDesc);
        	log("unlink exiting: desc_used = %d\n", desc_used);
	    }
	    unlink(vdslot->pathName);
	    delete_file(i);
	}
    }
}



/*
 * delfile_pid() -	Deletes all file contexts with a common process id.
 */

void
delfile_pid(pid)
register int
	pid;
{
register int
	i;			/* For loop counter */
register struct vFile
	*vdslot;

/* Delete all file contexts with specified process id */
    for (i = 0, vdslot = vfCache; i < vfCacheSize; i++, vdslot++)
	if ((vdslot->flags & VF_INUSE) && vdslot->dosPid == pid)
	    close_file(i, ANY_INUM);
}

/*
 *    changename() -			Updates the open file cache when rename
 *					is called so that the table contains the
 *					new filename.
 */

void
changename(oldname, newname)
char	*oldname;
char	*newname;
{
    register int
	i;			/* Loop counter for page table searches */
    int
	newpathlen;
    register struct vFile
	*vdslot;
    char
	oldpathname[MAX_PATH],
	newpathname[MAX_PATH];

    vfDbg(("changename: old: %s; new: %s\n", oldname, newname));

    /* Construct absolute pathname */
#ifdef MULT_DRIVE
	/* when mult-drive, then already fully qualified */
    strcpy(oldpathname, oldname);
    strcpy(newpathname, newname);
#else  /* ~MULT_DRIVE */
    strcpy(oldpathname, fnQualify(oldname, cwd));
    strcpy(newpathname, fnQualify(newname, cwd));
#endif /* ~MULT_DRIVE */
    newpathlen = strlen(newpathname) + 1;

/* Scan entire cache updating all entries tha match the old path name */
    for (i = 0, vdslot = vfCache; i < vfCacheSize; i++, vdslot++) 
	{
#if defined(XENIX) || defined(DGUX)
		if (vdslot->pathName != (char *)0)   /* prevent xenix barf (strcmp) */
#endif  /* XENIX || DGUX */
			if (strcmp(vdslot->pathName, oldpathname) == 0) 
			{
	    		vdslot->pathName = morememory(vdslot->pathName, newpathlen);
	    		strcpy(vdslot->pathName, newpathname);
	    		log("changename: changed entry %d\n",i);
			}
    }
}


/*
 * write_done() -		Returns TRUE if file has been written,
 *				otherwise FALSE.
 */

int
write_done(vdescriptor)
register unsigned int
	vdescriptor;
{
    vfDbg(("write_done: slot %d; %d\n", vdescriptor,
	vdescriptor < vfCacheSize && (vfCache[vdescriptor].flags & VF_DIRTY)));
    return vdescriptor < vfCacheSize && (vfCache[vdescriptor].flags & VF_DIRTY);
}


/*
 * file_written() -		Indicates a file has been written (is dirty!)
 */

void
file_written(vdescriptor)
register unsigned int
	vdescriptor;
{
    vfDbg(("file_written: slot %d\n", vdescriptor));
    if (vdescriptor < vfCacheSize)
	return;

    vfCache[vdescriptor].flags |= VF_DIRTY;
}




/*
 *	get_dos_time		Returns the virtual dos time stamp maintained
 *				for each file in the cache.  (Used in place of
 *				actual UNIX time stamp since DOS time stamps
 *				files at different times than does UNIX.)
 */

long
get_dos_time(vdescriptor)
int vdescriptor;
{
	return (vfCache[vdescriptor].vdostime);
}


/* Under System V, C programs are provided with an external long called */
/* timezone, which contains the number of seconds that the local time is */
/* behind GMT. We have to fake this up for Ultrix, but it's pretty easy */

#ifdef SYS5
extern long timezone;
#else
long Timezone;
#endif

time_t
UnixTime(year,month,day,hour,min,sec)
int year, month, day;
int hour, min, sec;
{
    static short mdays[] = {0, 31, 59, 90, 120, 151, 181, 
                           212, 243, 273, 304, 334};
    
    int days;
    time_t utime;
    struct tm *ts;
#ifdef BERKELEY42
    struct timeval tv;
    struct timezone tz;

#define timezone Timezone

    gettimeofday(&tv, &tz);

    timezone = tz.tz_minuteswest * 60;

    /* account for daylight savings time */
    if (localtime((time_t *)&tv.tv_sec)->tm_isdst)
	timezone -= 3600;
#endif

    days = (year * 365) + ((year + 3) / 4);
    if (((year & 3) == 0) && (month > 2)) days++;
    days += mdays[month-1] + day - 1;
    utime = ((days + 3652L) * 86400L) + (hour * 3600L) + (min *60) + sec
	+ timezone ;

    ts = localtime(&utime);
    if (days = (ts->tm_hour - hour)) {
	if (abs(days) > 1) 
	    utime += (days < 0) ? -3600 : 3600;
	else
	    utime += (days < 0) ? 3600 : -3600;
    }
    return (utime);
}

time_t
DosToUnixTime(dosdate, dostime)
int dosdate, dostime;
{
    int year, month, day;
    int hour, min, sec;
    time_t unixtime;
    
    year = (dosdate >> 9) & 0x7f;
    month = (dosdate >> 5) & 0x0f;
    day = dosdate & 0x1f;
    hour = (dostime >> 11) & 0x1f;
    min = (dostime >> 5) & 0x3f;
    sec = (dostime << 1) & 0x3e;
    unixtime = UnixTime(year,month,day,hour,min,sec);
    log("DosToUnixTime: %d/%d/%d %d:%d:%d %ld %ld\n",year,month,day,hour,
	min,sec,unixtime,timezone);
    return(unixtime);
}


/*
   pci_timedate: Return or set date and time of open file
*/

pci_timedate(vdesc, flag, time, date, resp)
int vdesc;
int flag;
int time;
int date;
struct output
	*resp;
{
struct tm
	*timeRec;			/* Broken down time record */
struct stat
	fileStat;			/* Unix file status */
struct vFile
	*cacheSlot;			/* Virtual descriptor cache slot */
long
	dos_time_stamp;			/* The DOS style stamp for this vdesc */
struct {
	time_t actime, modtime;
} utimbuf;

	cacheSlot = &vfCache[vdesc];

	resp->hdr.res = SUCCESS;
	resp->hdr.stat = NULL;
	if (!(cacheSlot->flags & VF_INUSE)
	||  stat(cacheSlot->pathName, &fileStat) < 0)
		resp->hdr.res = FAILURE;
	else {
		if (flag) {		/* set time and date */
			utimbuf.actime = fileStat.st_atime;
			utimbuf.modtime = DosToUnixTime(date,time);
			if (utime(cacheSlot->pathName,&utimbuf)) {
				resp->hdr.res = ACCESS_DENIED;
			}
		}
		else {			/* get time and date */
			dos_time_stamp = get_dos_time (vdesc);
			timeRec = localtime(&dos_time_stamp);
			resp->hdr.date = bdate(timeRec);
			resp->hdr.time = btime(timeRec);
		}
	}
}


/*--------------------------------------------------------------------------*/
/* Called when a file is deleted, to fixup the file context cache.
/* When the file that was deleted, is listed in the cache, then
/* zap the filename field, so it cannot be searched for by name,
/* but is still useful by programs that have it open
/*--------------------------------------------------------------------------*/
void
del_fname(name)
char
	*name;
{
register struct vFile
	*vdslot;

#ifndef MULT_DRIVE
    name = fnQualify(name, cwd);
#endif /* ~MULT_DRIVE */

    vfDbg(("del_fname %s\n", name));

    /* Scan file desc cache */
    for (vdslot = vfCache; vdslot < &vfCache[vfCacheSize]; vdslot++)
	/* Change name of matching entries */
#if defined(XENIX) || defined(DGUX)	/* prevent xenix barf on strcmp call */
	if (vdslot->pathName != (char *)0)
#endif	/* XENIX || DGUX */
	if (strcmp(vdslot->pathName, name) == 0)
	    *vdslot->pathName = (char) -1;

    return;
}



/* close all files in cache, no matter what */
close_all()
{
register int
	ii;

    vfDbg(("close_all\n"));

    for (ii = 0; ii < vfCacheSize; ii++)
	    close_file(ii, ANY_INUM);
}



/*
 *	get_vdescriptor		Returns the virtual descriptor associated with
 *				the given actual UNIX i-node number -- or
 *				-1 if node is not found to be open and in table.
 */

int
get_vdescriptor (inode)
ino_t inode;
{
	register int vdesc;		/* virtual desc, index into vfCache */
	struct vFile *vdslot;		/* place marker (ptr) in vfCache[] */

	for (vdesc = 0, vdslot = vfCache; vdesc < vfCacheSize; vdesc++, vdslot++)
	    if (vdslot->iNum == inode)
		return (vdesc);
	
	/* this inode is not to be found in vfCache */
	return (-1);
}


#ifdef RLOCK  /* record locking */

/*
 * lock_file() 
 *
 * lock or unlock the file
 */

int
lock_file(vdescriptor, dospid, mode, offset, length) 
int		vdescriptor;	/* PCI virtual file descriptor */
short		dospid;		/* Dos pid */
int		mode;		/* Lock or unlock */
long	offset, length;		/* Position and size of lock */
{

    log("lock_file:");
    log("vdesc = %d  pid = %u  mode = %d  offset = %ld  length = %ld\n",
	    vdescriptor, dospid, mode, offset, length);

    switch(mode) {

	/* lock */
	case 0: if (coLockRec(vdescriptor, dospid, offset, length)
		   == -1) { 
			log("lock_file: lock attempt failed.\n");
			return -1;
		}
	        break;

	/* unlock */
	case 1: if(coUnlockRec(vdescriptor, dospid, offset, length)
		   == -1) { 
			log("lock_file: unlock attempt failed.\n");
			return -1;
		}
	        break;

	 /* invalid data */ 
	default:
		errno = EINVAL;
		return -1;
    }

	return SUCCESS;
}
#endif  /* RLOCK */
#ifdef	LOCUS

#include <dstat.h>
/*
 * dfl_fstore			This routine is only for LOCUS systems.
 *				When a file is created it needs to have a
 *				default fstore.  Here we take the file 
 *				give it the fstore of its' parent directory.
 *
 *	Entry:
 *		pname		full qualified path name
 *
 *	Returns:
 *		0		Success
 *		1		Failure
 */

dfl_fstore(pname)
register char	*pname;
{
	struct	dstat ds_buf;		/* dstat buffer */
	char	*tmp,
		tmpstr[MAXPATHLEN];	/* will contain path to dstat */
	register char *cp = &tmpstr[0];

	strcpy(cp, pname);		/* copy path name */
	if ((tmp = strrchr(cp, '/')) == NULL) {        /* get final component */
		log("dfl_fstore: couldn't reduce path.\n");
		return(1);
	}
	if (tmp == cp)		/* we are in the root */
		tmp++;		/* special case */
	*tmp = NULL;			/* chop it */

	log("dfl_fstore: path to dstat: %s\n", cp);

	if (dstat(cp, &ds_buf, sizeof(ds_buf), 0) < 0) {
		log("dfl_fstore: dstat failed, errno: %d\n", errno);
		return(1);
	}

	if (chfstore(pname, ds_buf.dst_fstore) < 0) {
		log("dfl_fstore: chfstore failed, errno: %d\n", errno);
		return(1);
	}

	return(0);
}
	
#if	defined(RLOCK) && defined(LOCUS)

int
dosOpen(pathName, openMode, fileMode, excludeMode)
char		*pathName;		/* Name of file to open */
int		openMode;		/* UNIX file open mode */
int		fileMode;		/* UNIX file creation mode */
int		excludeMode;		/* DOS open exclusion mode */
{
int		uDesc;			/* Resulting UNIX descriptor */
int		saveErrno;		/* Saved errno value */
int		openAccess;		/* Bitwise access modes */
struct vFile	*olVFile;		/* Already open-locked file */
struct stat	statInfo;		/* Information on newly opened file */

	/* Open file, but don't truncate it till exclusion check succeeds */
	do
		uDesc = open(pathName, openMode & ~O_TRUNC, fileMode);
	while (uDesc == -1 && errno == EINTR);
	if (uDesc < 0)
		return -ACCESS_DENIED;

	/* Discover what kind of file this is */
	(void) fstat(uDesc, &statInfo);

	/* Prevent record lock conflicts from making read/write calls block */
	if ((statInfo.st_mode & S_IFMT) == S_IFREG)
		while (fcntl(uDesc, F_SETFL, openMode | O_NDELAY) == -1
							&& errno == EINTR)
			;

	/* Establish DOS open-time exclusion information in Locus kernel */
	do
		status = fcntl(uDesc, F_OPENLOCK, dos2LocusExcl(excludeMode));
	while (status == -1 && errno == EINTR);
	if (status >= 0) {
		/* Truncate the file if requested; return on success */
		if (!(openMode & O_TRUNC) || (ftruncate(uDesc, 0L) >= 0))
			return uDesc;
	}

	/*
	   Close the file but preserve errno from the F_OPENLOCK
	   fcntl() or the ftruncate() call, whichever was most
	   recent.  (close can fail in Locus!)
	*/
	saveErrno = errno;
	close(uDesc);
	errno = saveErrno;
	return -LOCK_VIOLATION;
}


int
dos2LocusExcl(dosExclude)
{
	switch (dosExclude) {
	default:
		vfDbg(("dos2LocusExcl: Bad DOS exclusion code\n"));
		return EXCLUDE_COMPAT;
		break;

	case SHR_RDWRT:	return EXCLUDE_BOTH;
	case SHR_WRT:	return EXCLUDE_WRITE;
	case SHR_RD:	return EXCLUDE_READ;
	case SHR_NONE:	return EXCLUDE_NONE;
	case SHR_DOS:
	case SHR_FCB:	return EXCLUDE_COMPAT;
	}
}

#endif	/* RLOCK && LOCUS */

#endif /* LOCUS */


/*
   uMaxDescriptors: Return count of available UNIX descriptors
*/

int
uMaxDescriptors()
{
static int	maxDesc = 0;

	/* Only compute maxDesc once */
	if (maxDesc == 0) {

#if	defined(BERKELEY42) || defined(LOCUS)
		maxDesc = getdtablesize();
#elif	defined(NOFILE)
		maxDesc = NOFILE;
#else
		maxDesc = (int)ulimit(4, 0L);
		if (maxDesc < 0)
			maxDesc = 20;
#endif	/* -NOFILE */

	}

	return maxDesc;
}
#ifdef RLOCK
/*
	rlockState()   Called by record locking library

	Called whenever the state (open, locked, etc) of a file
	changes.  We want to know when a file is locked or unlocked
	so that we do not swap out a descriptor on which there are
	locks.
*/
void
rlockState(index, state)
int index;	/* file index returned by addOpen() */
int state;	/* new state of this file */
{
	int vfSlot;		/* index into vfCache[] */
	struct vFile *vfp;	/* pointer into vfCache[] */

	/* we only care about lock status */
	if (state == RLSTATE_OPENED || state == RLSTATE_CLOSED)
		return;

	for (vfSlot = 0; vfSlot < vfCacheSize; vfSlot++)
	{
		vfp = &vfCache[vfSlot];
		if (vfp->shareIndex == index)
		{
			/* if it's locked, don't swap it */
			if (state == RLSTATE_LOCKED)
				vfp->flags &= ~VF_EPHEM;

			/* if it's not locked, let it be swapped */
			else if (state == RLSTATE_ALL_UNLOCKED)
				vfp->flags |= VF_EPHEM;
			break;
		}
	}
}
#endif /* RLOCK */
