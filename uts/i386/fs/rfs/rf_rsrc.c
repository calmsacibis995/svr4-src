/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_rsrc.c	1.3.1.1"
/*
 * utility routines dealing with sr_mount and rf_resource lists
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/vnode.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/nserve.h"
#include "sys/debug.h"
#include "sys/list.h"
#include "sys/rf_adv.h"
#include "sys/kmem.h"
#include "sys/rf_sys.h"

/* imports */
extern int	strcmp();

/*
 * rf_resource_head is the head of the resource list.  This list is a doubly
 * linked ring with rf_resource_head itself cast to be a member.  It is
 * initially the only member of the list.
 */
rf_resource_head_t rf_resource_head = 
  {(rf_resource_t *)&rf_resource_head,(rf_resource_t *)&rf_resource_head};

/*
 * srm_count records the number of sr_mount_t's in circulation.  srm_alloc()
 * and srm_free() keep it current and restrict it to be no greater than
 * the configurable parameter nsrmount.
 */
uint	srm_count;

/* 
 * Resource list lookup using a vnode pointer, vp, as the key.
 * Returns a pointer to the matching resource structure or NULL if
 * none was found.
 */
rf_resource_t *
vp_to_rsc(vp)
	vnode_t *vp;
{
	register rf_resource_t *rp;
	register rf_resource_t *toofar = (rf_resource_t *)&rf_resource_head;

	for (rp = rf_resource_head.rh_nextp; rp != toofar; rp = rp->r_nextp)
		if (rp->r_rootvp == vp)
			return rp;
	return NULL;
}


/* 
 * Resource list lookup using the given name as the key.
 * Returns a pointer to the matching rf_resource structure or NULL if
 * none was found.
 */
rf_resource_t *
name_to_rsc(namep)
	char	*namep;
{
	register rf_resource_t *rp;
	register rf_resource_t *toofar = (rf_resource_t *)&rf_resource_head;

	for (rp = rf_resource_head.rh_nextp; rp != toofar; rp = rp->r_nextp)
		if (!strcmp(namep, rp->r_name))
			return rp;
	return NULL;
}

/*
 * Resource list lookup using an "index" value, rid, as the key.
 * Returns a pointer to the matching rf_resource structure or NULL if
 * none was found.
 */
rf_resource_t *
ind_to_rsc(rid)
	long rid;
{
	register rf_resource_t *rp;
	register rf_resource_t *toofar = (rf_resource_t *)&rf_resource_head;

	for (rp = rf_resource_head.rh_nextp; rp != toofar; rp = rp->r_nextp)
		if (rp->r_mntid == rid)
			return rp;
	return NULL;
}

/*
 * rsc_nm returns a pointer to the last element of a given resource
 * name.  E.g., if name == a.b.c, rsc_nm returns a ptr to c.  If
 * name == a, it would return a ptr to a.  The given name is assumed
 * to be NULL terminated.
 */
char    *
rsc_nm(name)
	register char   *name;
{
	register char   *ptr = name;

	ASSERT(name != NULL);

	while (*name)
		if (*name++ == SEPARATOR)
			ptr = name;
	return ptr;
}

/* Returns 1 if name is that of a locally advertised resource,
 * 0 otherwise.
 * No side-effects
 */
int
localrsrc(name)
	register char *name;
{
	register char *rsrc;
	int found = 0;

	/* Separate the resource and the domain name (if there
	 * is one).  If the domain is the current domain, check
	 * to see if the resource is in the local resource list. 
	 *
	 * THIS ALGORITHM WILL CHANGE WITH MULTI-LEVEL DOMAINS
	 */
	register rf_resource_t *rp;

	for (rsrc = name; *rsrc && *rsrc != SEPARATOR; rsrc++)
		;
	if (*rsrc == SEPARATOR) {
		/* 
		 * there is a domain name, return 'found' only if
		 * both the domain and resource names match.  
		 * temporarily replace the SEPARATOR with a NULL
		 * in order to compare domain names.  Restore
		 * that before returning.
		 */
		*rsrc++ = '\0'; 
		if (strcmp(name, rfs_domain) == 0
		    && (rp = name_to_rsc(rsrc)) != NULL
		    && !(rp->r_flags & R_UNADV)) {
			found = 1;
		}
		*--rsrc = SEPARATOR;    /* replace the SEPARATOR in the name */
	} else if ((rp = name_to_rsc(name)) != NULL
	    && !(rp->r_flags & R_UNADV)) {
		found = 1;
	}
	return found;
}


/*
 * Scans the list of resources for the first gap in index values.
 * 
 * Sets newrp->r_mntid to the next value within that gap and links
 * newrp into the list of resources so that it remains sorted in
 * increasing order of index values.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
int
insertrsc(newrp)
	register rf_resource_t *newrp;
{
	register rf_resource_t *rp;	/* search cursor */
	register rf_resource_t *toofar = (rf_resource_t *)&rf_resource_head; 
	/*
	 * Simulate static table index for mntindx.
	 */
	register long mntindx = 0;

	for (mntindx = 0, rp = rf_resource_head.rh_nextp; rp != toofar;
	   mntindx++, rp = rp->r_nextp) {
		if (rp->r_mntid != mntindx) {	/* here is 1st gap */
			newrp->r_mntid = mntindx;
			LS_INS_BEFORE(rp, newrp);
			return 0;
		}
	}
	newrp->r_mntid = mntindx;
	LS_INS_BEFORE(rp, newrp);
	return 0;
}

/*
 * this routine unlinks the specified element from the resource list and
 * frees its memory, NULLing the pointer to the resource, the caller is
 * expected to release its root vnode reference, if any.
 */
void
freersc(rpp)
	rf_resource_t **rpp;
{
	LS_REMOVE(*rpp);
	kmem_free((caddr_t)*rpp, sizeof(rf_resource_t));
	*rpp = NULL;
}

/*
 * Allocate a rf_resource_t, initialize its ls_elt_t, and return a reference,
 * if possible, else return NULL.
 */
rf_resource_t *
allocrsc()
{
	register rf_resource_t *rp;

	if ((rp = (rf_resource_t *)kmem_zalloc(sizeof(rf_resource_t), KM_SLEEP))
	  != NULL) {
		LS_INIT(rp);
	}
	return rp;
}

/*
 * id_to_srm scans the mount list associated with the resource referenced
 * by rp for a mount identified by sysid, returning a pointer to that
 * mount structure, or NULL if none is found.
 */
sr_mount_t *
id_to_srm(rp, sysid) 
	rf_resource_t *rp;	/* which resource */
	sysid_t sysid;		/* which circuit */
{
	sr_mount_t *srp;	/* to scan list */

	for (srp = rp->r_mountp; srp != NULL; srp = srp->srm_nextp)
		if (srp->srm_sysid == sysid)
			return srp;		/* found it */
	return NULL; 				/* no such mount */
}

/*
 * srm_alloc is used to allocate a sr_mount_t.  It won't allow srm_count to
 * exceed nsrmount.  It also can fail if the memory allocation fails.
 * Otherwise, *srmpp refers to the new element and 0 is returned.
 * For historical reasons, it returns ENOSPC for failure.
 *
 * NOTE:  The caller must link the new element into the rp's r_mountp list.
 * NOTE:  Because kmem_zalloc might sleep, check and increment srm_count
 * 	  before that call.
 */
int
srm_alloc(srmpp)
	sr_mount_t **srmpp;	/* out parameter, the new element */
{
	if (srm_count == nsrmount) {
		return ENOSPC;
	}
	srm_count++;
	if ((*srmpp = (sr_mount_t *)kmem_zalloc(sizeof(sr_mount_t), KM_SLEEP))
	    == NULL) {
		srm_count--;
		return ENOSPC;
	}
	return 0;
}

/*
 * Free the sr_mount_t referred to by *srpp, update srm_count,
 * and NULL *srpp;
 */
void
srm_free(srpp)
	sr_mount_t	**srpp;
{
	kmem_free((caddr_t)*srpp, sizeof(sr_mount_t));
	srm_count--;
	*srpp = NULL;
}

/*
 * srm_remove removes the sr_mount structure referenced by *srpp from the
 * r_mountp list of the rf_resource structure referenced by *rpp and frees
 * its memory.  If doing so removes the last mount of an unadvertised 
 * resource, that resource structure is also freed.
 * Returns 0 for success or a non-zero errno for failure.
 */
int
srm_remove(rpp, srpp)
	rf_resource_t	**rpp;	/* which resource */
	sr_mount_t	**srpp;	/* which mount */
{
	register struct rf_resource *rp = *rpp;
	sr_mount_t	*srp = *srpp;

	if ((rp == NULL) || (srp == NULL))
		return ENODEV;	
	if (srp->srm_nextp != NULL)
		srp->srm_nextp->srm_prevp = srp->srm_prevp;
	if (srp->srm_prevp != NULL) {
		srp->srm_prevp->srm_nextp = srp->srm_nextp;
	} else {
		rp->r_mountp = srp->srm_nextp;
		/*
		 * If this was the last mount of an unadvertised resource,
		 * free the resource
		 */
		if (rp->r_mountp == NULL) {
			if (rp->r_flags & R_UNADV) {
			    vnode_t *vp = (*rpp)->r_rootvp;

			    freersc(rpp);
			    VN_RELE(vp);	/* NOTE:  keep this ordering */
			}
		} else {
			/* EMPTY */
			ASSERT(rp->r_queuep != NULL);
		}
	}
	srm_free(srpp);
	return 0;
}
