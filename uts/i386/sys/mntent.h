/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)head.sys:sys/mntent.h	1.4.5.1"
#define bcopy(f,t,n)    memcpy(t,f,n)
#define bzero(s,n)	memset(s,0,n)
#define bcmp(s,d,n)	memcmp(s,d,n)

#define index(s,r)	strchr(s,r)
#define rindex(s,r)	strrchr(s,r)

#define MNTTAB		"/etc/mnttab"
#define VFSTAB		"/etc/vfstab"

#define	MNTTYPE_UFS	"ufs"
#define MNTTYPE_SWAP    "swap"  /* swap file system */
#define MNTTYPE_IGNORE  "ignore"/* No type specified, ignore this entry */
#define MNTTYPE_LO      "lo"    /* Loop back File system */

#define      MNTOPT_RO       "ro"    /* read only */
#define   MNTMAXSTR       128
#define MNTOPT_RW       "rw"    /* read/write */
#define MNTOPT_RQ	"rq"	/* read/write with quotas */
#define MNTOPT_QUOTA    "quota" /* quotas */
#define MNTOPT_NOQUOTA  "noquota"/* no quotas */
#define MNTOPT_SOFT     "soft"  /* soft mount */
#define MNTOPT_HARD     "hard"  /* hard mount */
#define MNTOPT_NOSUID   "nosuid"/* no set uid allowed */
#define MNTOPT_NOAUTO   "noauto"/* hide entry from mount -a */
#define MNTOPT_GRPID    "grpid" /* SysV-compatible group-id on create */
#define MNTOPT_REMOUNT  "remount"/* change options on previous mount */
#define MNTOPT_NOSUB    "nosub"  /* disallow mounts beneath this one */
#define MNTOPT_MULTI    "multi"  /* Do multi-component lookup */

#define L_SET		0	/* for lseek */

char * hasmntopt();
