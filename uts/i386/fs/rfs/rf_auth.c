/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_auth.c	1.3.1.1"

/*
 * Routines for id mapping
 */
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/nserve.h"
#include "sys/stream.h"
#include "sys/rf_cirmgr.h"
#include "sys/vnode.h"
#include "sys/idtab.h"
#include "sys/debug.h"
#include "sys/rf_debug.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/systm.h"
#include "rf_auth.h"
#include "sys/kmem.h"
#include "sys/rf_sys.h"
#include "sys/list.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "rf_canon.h"

#define CACHESIZE       5
#define MAX_MLINKS      2       /* max number of gdp links per machine */
#define UF 		(-1)

/*
 * CHECK(tabp, val, ret) compares remote id referred to by tabp
 * with val and if there is a match sets ret to the mapped local id.
 * It returns 	0 if there is a match
 * 		1 if remote id from tabptr > val
 *        and  -1 if remote id from tabptr < val.
 */
#define CHECK(tabp,val,ret)     ((tabp->i_rem == val)? \
	  ((tabp->i_loc == UF)?(ret=(tabp+1)->i_loc,0):(ret=tabp->i_loc,0)):\
	  ((tabp->i_rem < val)?\
	    ((tabp->i_loc == UF && (tabp+1)->i_rem >= val)?\
	      (ret=(tabp+1)->i_loc,0):-1):\
	    (((tabp-1)->i_loc == UF && (tabp-1)->i_rem <= val)?\
	      (ret=tabp->i_loc,0):1)))

typedef long 	align_t;
typedef long	*maddr_t;		/* memory address type */

#define ASIZE (sizeof(align_t))
#define ALIGN(s) ((s) + (((s)%ASIZE)?ASIZE-((s)%ASIZE):0))

#define MAXNAMES 100            /* maximum # of names per client list */
#ifndef BUFSIZ
#define BUFSIZ 1024             /* for client list allocation */
#endif

/* imports */
extern int	copyinstr();
extern int	suser();
extern int	strcmp();

STATIC char     *Global[2] = {0, 0};    /* global default table ref */

STATIC int      Tabwlock = 0;           /* Table write lock */
STATIC int      Tabrlock = 0;           /* Table read lock */
STATIC int      Want_wr = 0;            /* I want to write */
STATIC int      Want_rd = 0;            /* I want to read */

STATIC void	wr_lock();              /* Lock for writers */
STATIC void	wr_unlock();            /* Unlock for writers */
STATIC void	rd_lock();
STATIC void	rd_unlock();
STATIC void	rf_remidmap();
STATIC int	rf_heapalloc();
STATIC uid_t	rglid();

void
auth_init()
{
	Global[UID_DEV] = Global[GID_DEV] = NULL;
}

/*
 * Allocates storage for a kernel copy of the user supplied client list
 * 'list', copying the contents of 'list' to this storage and returning
 * a reference to it in the out parameter allocbufpp.  'list' is expected
 * to be NULL terminated.
 *
 * Returns 0 for success, a non-zero errno for failure.
 */
int
rf_addalist(allocbufpp, list)
	char		**allocbufpp;
	char		**list;
{
	register int	px;
	register int	nx;
	register char	*lptr;
	char		*ref = 0;	/* allocation reference to return */
	unsigned	size = 0;	/* size of block to be allocated */
	char		buffer[BUFSIZ];
	char		*alist[MAXNAMES];
	int		sizelist[MAXNAMES];
	int		error = 0;
	uint		c_count;	/* copyinstr out parameter */

	DUPRINT2(DB_MNT_ADV, "rf_addalist: list=%x\n", list);
	/*
	 * Copy in pointers.
	 */
	for (px = 0; px < MAXNAMES; px++) {
		DUPRINT3(DB_MNT_ADV, "rf_addalist: list[%d]=%x\n", px, list);
		if (copyin((char *)list++, (char *)&alist[px],
		  sizeof(char *))) {
			return EFAULT;
		}
		if (!alist[px]) {
			break;
		}
	}
	if (px == MAXNAMES) {
		return EINVAL;
	}
	/*
	 * Scan list names and remember sizes for space allocation.
	 */
	for (nx = 0; nx < px; nx++) {
		if (error = copyinstr(alist[nx], buffer, BUFSIZ, &c_count)) {
			return error;
		}
		DUPRINT3(DB_MNT_ADV,
		  "rf_addalist: name[%d]=%s\n", nx, buffer);
		sizelist[nx] = c_count;
		size += c_count;
	}
	/*
	 * Allocate space for names.
	 */
	size++;
	size = ALIGN(size);
	if (error = rf_heapalloc((char **)&ref, size)){
		return error;
	}
	lptr = ref;
	/*
	 * Copy in list names.
	 */
	for (nx = 0; nx < px; nx++) {
		*lptr++ = (char)sizelist[nx] - 1;
		if (error = copyinstr(alist[nx], lptr, BUFSIZ, &c_count)) {
			rf_heapfree(ref);
			return error;
		}
		lptr = lptr + c_count - 1;
	}
	/* now fill up rest with nulls  */
	for (nx = (lptr - ref); nx < size; nx++) {
		*lptr++ = '\0';
	}
	*allocbufpp = ref;
	return error;
}

/*
 * Compares name, which is assumed to be a fully specified machine name, with
 * the names in the list that ref refers to.  Returns 1 if there is a match
 * within that list, 0 otherwise.  The following conditions constitute a match:
 *
 * 1. complete match of full names.
 * 2. complete match of domain part of name if list item ends
 *    with a SEPARATOR.  e.g., if name = "a.b.c" and the list
 *    item = "a.b." there would be a match because a.b. implies
 *    a match with any name in the domain a.b.
 * 3. the name has the same dompart as this machine, and the
 *    namepart of name exactly matches an item in the list.
 *
 * NOTE: this code parses the names from left to right, i.e.,
 *       domain.name.
 */
int
rf_checkalist(ref, name)
	char    *ref;
	char    *name;
{
	register char   *lp;
	register char   *np;
	register char   *nlp = NULL;
	char    *sp;
	char    *npart = NULL;  /* if != NULL, pointer to namepart of name */

	DUPRINT3(DB_MNT_ADV, "rf_checkalist: ref=%x, name=%s\n", ref, name);

	/* first check to see if name is in same domain as machine      */
	for (np = name, lp = rfs_domain; *lp != '\0'; lp++, np++) {
		if (*np != *lp) {
			break;
		}
	}

	/* if we match domain totally, and name is at a separator,      */
	/* we call it a match and call whatever is left the namepart    */
	if (*np == SEPARATOR && *lp == '\0') {
		npart = np + 1;
		DUPRINT3(DB_MNT_ADV,
		  "rf_checkalist: name (%s) is local, npart =%s\n",
		  name, npart);
	}

	for (lp = ref; *lp != '\0'; lp = nlp) {
		nlp = lp + (int) *lp + 1;
		sp = ++lp;      /* go past the count    */
		/* first check for full name match or domain match      */
		for (np = name; *np != '\0'; np++, lp++) {
			if (*np != *lp) {
				break;
			}
		}
		if ((*np == '\0' && nlp == lp) || (*(np - 1) == SEPARATOR &&
		  lp == nlp && *(lp - 1) == SEPARATOR)) {
			DUPRINT1(DB_MNT_ADV,
			  "rf_checkalist returns 1, full match\n");
			return 1;
		}
		/* now check for current domain match   */
		if (!npart) {
			continue;
		}
		for (np = npart, lp = sp; *np != '\0'; np++, lp++) {
			if (*np != *lp) {
				break;
			}
		}

		if (*np == '\0' && lp == nlp) {
			DUPRINT1(DB_MNT_ADV,
			  "rf_checkalist returns 1, local domain\n");
			return 1;
		}
	}
	DUPRINT1(DB_MNT_ADV, "rf_checkalist returns 0\n");
	return 0;
}


/*
 * rf_heapalloc() allocates a block of size bytes from
 * kernel memory pool. The size of the memory chunk is saved
 * at the beginning of the allocated memory for later use in rf_heapfree().
 * A pointer to the block is placed in allocbufpp, and
 * an error condition is returned.
 */
STATIC int
rf_heapalloc(allocbufpp, size)
	char **allocbufpp;
	size_t size;
{
	maddr_t memp;
	size_t  memsize;

	wr_lock();
	/* align the size on word boundary */
	memsize = ALIGN(size) + sizeof(maddr_t);

	if (rf_maxkmem && rf_availkmem < memsize) {
		wr_unlock();
		*allocbufpp = NULL;
		return ENOMEM;
	}
	rf_availkmem -= memsize;
	memp = (maddr_t)kmem_zalloc(memsize, KM_SLEEP);
	*memp = (long)memsize;
	*allocbufpp = (char *)(memp + 1);
	wr_unlock();
	return 0;
}


/* rf_heapfree() takes a pointer to a buffer
 * allocated by rf_heapalloc() and frees the buffer.
 */
void
rf_heapfree(offset)
	caddr_t  offset;
{
	maddr_t memp = (maddr_t)offset;
	size_t	memsize;

	wr_lock();
	--memp;
	memsize = (size_t)*memp;	/* retrieve memory size */
	ASSERT(memsize);
	rf_availkmem += memsize;
	kmem_free((caddr_t)memp, memsize);
	wr_unlock();
	return;
}

/*
 * Translation table system call (actually called through rfsys).
 * The uid, gid translation tables in the kernel are locked
 * whenever they are updated (e.g., from a user level) or
 * accessed from the kernel (e.g., through the kernel routines
 * interface). This is to prevent updating the table during an
 * access operation.
 *
 * rf_setidmap:
 *  finds the gdp structure for the named machine,
 *  and adds uid or gid maps to the idmap locations
 *  associated with the machine.
 *  Returns 0 or an error code if failed.
 */
int
rf_setidmap(name, flag, map, cr)
	char *name;
	int flag;
	struct idtab *map;
	cred_t *cr;
{
	struct idtab header;
	struct idtab idtab_init;
	struct idtab *idp;
	struct idhead *hdp;
	register gdp_t *gp;
	register gdp_t *endgdp = gdp + maxgdp;
	char 	sname[MAXDNAME+1];
	char    *ref = NULL;
	char    **mapaddr[MAX_MLINKS];
	int 	cnt;
	int 	gcnt;
	uint 	c_count;
	int	error;

	if (!suser(cr)) {
		return EPERM;
	}

	/*
	 * NULL name says to clear all tables
	 * which include all the idmap tables (UID and GID) associated
	 * with the active GDPs and Global idmap tables.
	 */
	if (!name) {
		for (gp = gdp; gp < endgdp; gp++) {
			if (gp->constate == GDPFREE) {
				rf_remidmap(gp);
			}
		}
		if (Global[UID_DEV]) {
			rf_heapfree(Global[UID_DEV]);
			Global[UID_DEV] = NULL;
		}
		if (Global[GID_DEV]) {
			rf_heapfree(Global[GID_DEV]);
			Global[GID_DEV] = NULL;
		}
		return 0;
	}
	/* copy in the machine name and initial information     */
	if ((error = copyinstr(name, sname, MAXDNAME, &c_count)) != 0) {
		return error;
	}

	/*
	 * if name is GLOBAL_CH, this is global name, else
	 * look for name in gdp structure.
	 */
	if (sname[0] == GLOBAL_CH) {
		mapaddr[0] =
		    (flag == UID_MAP) ? &(Global[UID_DEV]) : &(Global[GID_DEV]);
		gcnt = 1;
	} else {
		for (gp = gdp, gcnt = 0; gp < endgdp &&
		  gcnt < MAX_MLINKS; gp++) {
			if (strcmp(sname, gp->token.t_uname) == 0) {
				mapaddr[gcnt++] = (flag == UID_MAP) ?
				  &(gp->idmap[UID_DEV]) : &(gp->idmap[GID_DEV]);
			}
		}
		if (!gcnt) {
			return ENONET;
		}
	}

	/* if there was a map, set it   */
	if (map && copyin((char *)map, (char *)&header, sizeof(struct idtab))) {
		return EFAULT;
	}

	/*
	 * free as late as possible, to save current map if something
	 * goes wrong
	 */
	for (cnt = 0; cnt < gcnt; cnt++) {
		if (*mapaddr[cnt]) {
			rf_freeidmap(*mapaddr[cnt]);
			*mapaddr[cnt] = NULL;
		}
	}

	if (map) {
		if (error = rf_heapalloc((char **)&ref,
		      (size_t)(sizeof(struct idtab) *
			(header.i_tblsiz + HEADSIZE + CACHESIZE)))) {
				return error;
		}
		rd_lock();

		/* set up header        */
		hdp = (struct idhead *) ref;
		hdp->i_default = header.i_defval;
		hdp->i_size = header.i_tblsiz;
		hdp->i_cend = HEADSIZE + CACHESIZE;
		hdp->i_next = HEADSIZE;
		hdp->i_tries = hdp->i_hits = 0;

		/* mark cache unused    */
		idp = (struct idtab *) ref;
		idtab_init.i_rem = idtab_init.i_loc = UF;

		for (cnt = HEADSIZE; cnt < HEADSIZE + CACHESIZE; cnt++) {
			idp[cnt] = idtab_init;
		}

		/* copy in the table    */
		if (copyin((char *) (map + 1), (char *) (idp + HEADSIZE +
		    CACHESIZE), sizeof(struct idtab ) * header.i_tblsiz)) {
			rd_unlock(); /* must unlock before free */
			rf_heapfree(ref);
			error = EFAULT;
			return error;
		}
		rd_unlock();

		for (cnt = 0; cnt < gcnt; cnt++) {
			*mapaddr[cnt] = ref;
		}
	}
	return 0;
}

/*
 * this is called with an idmap that is in the gdp table.
 * this function only frees the map if there are 1 or 0 references
 * to the map in the gdp structure.  This allows multiple pointers
 * to the same map from the gdp structure.
 */
void
rf_freeidmap(offset)
	char *offset;
{
	register gdp_t *gp;
	register gdp_t *endgdp = gdp + maxgdp;
	register int    gcnt = 0;

	for (gp = gdp; gp < endgdp; gp++) {
		if (gp->idmap[UID_DEV] == offset ||
		   gp->idmap[GID_DEV] == offset)
			gcnt++;
	}
	if (gcnt <= 1) {
		rf_heapfree(offset);
	}
}

STATIC void
rf_remidmap(gp)
	gdp_t *gp;
{
	if (gp->idmap[UID_DEV]) {
		rf_freeidmap(gp->idmap[UID_DEV]);
		gp->idmap[UID_DEV] = NULL;
	}
	if (gp->idmap[GID_DEV]) {
		rf_freeidmap(gp->idmap[GID_DEV]);
		gp->idmap[GID_DEV] = NULL;
	}
}


/*
 *  glid returns a local id given a gdp_t pointer
 *  and remote id.  The argument idtype chooses whether
 *  the uid or gid table is searched.
 *
 *  glid uses a binary search to improve the search time.
 *  This requires the table to be sorted.  The table is assumed
 *  to be sorted in ascending order with remote id as the
 *  primary sort key.
 *
 */

uid_t
glid(idtype, gp, rid)
	int			idtype;
	gdp_t			*gp;
	register uid_t		rid;
{
	register struct idtab	*low;	/* invariant: points to record <= rid */
	register struct idtab	*high;	/* invariant: points to record >= rid */
	register struct idtab	*middle;/* points to most recent record */
	ushort			size;	/* size of table */
	uid_t			defval = UID_NOBODY;
					/* default value for local id */
	uid_t			ret = UID_NOBODY;
					/* return value */
	struct idtab		*idt = NULL;
	struct idhead		*hp;

	rd_lock();
	if (gp && gp->idmap[idtype]) {
		idt = (struct idtab *)gp->idmap[idtype];
	} else if (Global[idtype]) {
		idt = (struct idtab *)Global[idtype];
	} else {
		goto glid_out;  /* ret already has default value */
	}

	/* at this point should have a good table */
	hp = (struct idhead *) idt;
	size = idt->i_tblsiz;
	defval = idt->i_defval;

	/* CHECK sets ret if it finds a match, if it doesn't, use default */
	ret = defval ? defval : rid;
	if (!size) {
		goto glid_out;
	}
	low = idt + hp->i_cend;
	high = idt + size + hp->i_cend - 1;
	if (rid < low->i_rem || rid > high->i_rem) {	/* out of range */
		goto glid_out;
	}

	/* check the extremes of range  */
	if (!CHECK(low, rid, ret) || !CHECK(high, rid, ret)) {
		goto glid_out;
	}

	while (high - low > 1) {
		middle = (high - low) / 2 + low;
		switch (CHECK(middle, rid, ret)) {
		case -1:        /* middle is < rid */
			low = middle;
			break;
		case 1:         /* middle is > rid */
			high = middle;
			break;
		case 0:         /* found it */
			goto glid_out;
		}
	}

glid_out:
	rd_unlock();
	return ret;
}

/* get local id, given sysid & rid */
STATIC uid_t
rglid(idtype, gp, rid)
	int			idtype;
	gdp_t			*gp;
	register uid_t		rid;
{
	register struct idtab	*low;	/* invariant: points to record <= rid */
	register struct idtab	*high;	/* invariant: points to record >= rid */
	ushort			size;	/* size of table */
	uid_t			defval = UID_NOBODY;
	uid_t			ret = UID_NOBODY;
	struct idtab		*idt = NULL;
	struct idhead		*hp;

	rd_lock();
	if (gp && gp->idmap[idtype]) {
		idt = (struct idtab *) gp->idmap[idtype];
	} else if (Global[idtype]) {
		idt = (struct idtab *) Global[idtype];
	} else {
		if (rid != defval) {
			ret = UID_NOACCESS;
		}
		goto rglid_out;         /* ret already has default value */
	}

	/* at this point should have a good table       */
	hp = (struct idhead *) idt;
	size = idt->i_tblsiz;
	defval = idt->i_defval;

	/* CHECK sets ret if it finds a match, if it doesn't, use default */
	ret = (defval) ? defval : rid;

	if (!size) {
		if (defval && rid != defval) {
			ret = UID_NOACCESS;
		}
		goto rglid_out;
	}

	hp->i_tries++;

	low = idt + HEADSIZE;
	for (high = low + CACHESIZE; low < high; low++) {
		if (low->i_loc == rid) {
			ret = low->i_rem;
			hp->i_hits++;
			goto rglid_out;
		}
	}

	low = high;
	for (high += size; low < high; low++) {
		if (low->i_loc == rid ||
		  (low->i_loc == UF && (low + 1)->i_loc == rid)) {
			ret = low->i_rem;
			(idt + (hp->i_next))->i_loc = rid;
			(idt + (hp->i_next))->i_rem = ret;
			hp->i_next = (hp->i_next < (ushort)(hp->i_cend - 1)) ?
			  hp->i_next + 1 : HEADSIZE;
			goto rglid_out;
		}
	}
	/*
	 * No match found.  Need to insure that inaccessible ids are
	 * appropriately flagged.  This is done by doing a forward
	 * mapping.  If the forward mapping changes the input, it
	 * means that this id is inaccessible from this machine, and
	 * UID_NOACCESS is returned.
	 */
	if (glid(idtype, gp, rid) != rid) {
		ret = UID_NOACCESS;
		(idt + (hp->i_next))->i_loc = rid;
		(idt + (hp->i_next))->i_rem = ret;
		hp->i_next = (hp->i_next < (ushort)(hp->i_cend - 1)) ?
		  hp->i_next + 1 : HEADSIZE;
	}

rglid_out:
	rd_unlock();
	return ret;
}

STATIC void
rd_lock()
{
	Want_rd++;
	while (Tabrlock) {
		sleep((caddr_t)&Tabrlock, PWAIT);
	}
	Tabwlock++; /* lock potential writers */
	Want_rd--;
}

STATIC void
rd_unlock()
{
	if (!--Tabwlock && Want_wr) {
		wakeprocs((caddr_t)&Tabwlock, PRMPT);
	} else if (Want_rd) {		 /* This may not be necessary */
		wakeprocs((caddr_t)&Tabrlock, PRMPT);
	}
}

STATIC void
wr_lock()
{
	Want_wr++;
	while (Tabwlock || Tabrlock) {
		sleep((caddr_t)&Tabwlock, PWAIT);
	}
	Tabrlock++;     /* lock potential readers & writers */
	Want_wr--;
}

STATIC void
wr_unlock()
{
	Tabrlock--;
	if (Want_wr) {
		wakeprocs((caddr_t)&Tabwlock, PRMPT);
	} else if (Want_rd) {
		wakeprocs((caddr_t)&Tabrlock, PRMPT);
	}
}

void
vattr_rmap(gdpp, vap)
	gdp_t *gdpp;
	vattr_t *vap;
{
	vap->va_uid = rglid(UID_DEV, gdpp, vap->va_uid);
	vap->va_gid = rglid(GID_DEV, gdpp, vap->va_gid);
}
