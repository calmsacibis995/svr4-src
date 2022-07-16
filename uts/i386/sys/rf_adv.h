/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/rf_adv.h	1.6.5.1"

#ifndef _SYS_RF_ADV_H
#define _SYS_RF_ADV_H

/*
 * Resource structure, supplants both advertise table and server mount
 * table.  Each advertised resource has an entry in this table holding
 * necessary info, including a list of info about each remote mount of
 * the resource.
 */

#define RFS_NMSZ 15		/* size of advertised name */

typedef struct sr_mount {
	struct sr_mount *srm_nextp;	/* ptr to next in list */
	struct sr_mount *srm_prevp;	/* ptr to prev in list */
	sysid_t 	srm_sysid;	/* which client */
	ushort  	srm_flags;	/* see sr_flag values below */
	long     	srm_mntid;	/* denotes mounted resource on client */
	int 		srm_kbcnt;	/* Kbytes read/written by this client */
	int 		srm_refcnt;	/* number of outstanding gifts */
	int		srm_slpcnt;	/* used by fumount, recover */
} sr_mount_t;

/* sr_flag values */
#define	SRM_RDONLY	1	/* This mount is read only */
#define SRM_LINKDOWN 	2	/* Link to client is down */ 
#define SRM_FUMOUNT	4	/* being fumounted */
#define SRM_CACHE	8	/* this client is cacheing this resource */

typedef struct rf_resource {
	struct rf_resource *r_nextp;	/* ptr to next in list */
	struct rf_resource *r_prevp;	/* ptr to prev in list */
	/*
	 * r_mountp heads the list of sr_mount structures for this resource.
	 */
	struct sr_mount *r_mountp;	
	int		r_flags;		/* see values below */
	struct vnode   *r_rootvp;		/* root vnodep */
	char		r_name[RFS_NMSZ];	/* sent to name server */
	struct	rcvd   *r_queuep;		/* receive queue */
	char	       *r_clistp;		/* authorization list */
	/*
	 * Pre-release 4 clients expect to be given an index into the server's
	 * srmount table when they mount a resource, then pass that back
	 * to the server with each request.  The srmount table is no longer
	 * used in RFS, so a replacement to that index is r_mntid which is a
	 * pseudo index used for the same purpose.
	 */
	long		r_mntid;	
} rf_resource_t;

/* r_flag values */
#define R_RDONLY	1	/* advertised read only */
#define R_CACHEOK    	2	/* files are cacheable */
#define R_UNADV		4	/* unadvertised, but not free yet */
#define R_FUMOUNT	8	/* being fumounted */

typedef struct rf_resource_head {
	rf_resource_t *rh_nextp;
	rf_resource_t *rh_prevp;
} rf_resource_head_t;

extern rf_resource_head_t rf_resource_head;

extern int	nsrmount;	/* configurable limit on srmounts */
extern uint	srm_count;	/* number of extant srmounts */

extern rf_resource_t *ind_to_rsc();
extern rf_resource_t *vp_to_rsc();
extern rf_resource_t *name_to_rsc();
extern char *rsc_nm();
extern int localrsrc();
extern rf_resource_t *allocrsc();
extern int insertrsc();
extern void freersc();
extern sr_mount_t *id_to_srm();
extern int srm_alloc();
extern int srm_remove();
extern void srm_free();

#endif	/* _SYS_RF_ADV_H */
